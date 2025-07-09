//
// Created by zhou on 25-6-30.
//

#include "STLSurfaceGroup.h"

#include "SystemStateHub.h"
#include <SunPosition.h>

#include "Radiation.h"

namespace comp {
    void STLSurfaceGroup::awake() {
        const std::string jsonStr1 = R"({"params": {
                                            "weather": {"name": "weather", "type": "string", "value": ""}
                                },
                              "inputs": {

                              },
                              "outputs": {}})";
        init_vars(jsonStr1);

        //初始化表面组
        surface_group = std::make_shared<SurfaceGroup>(name, path, surface_names);

        auto site = core::SystemStateHub::getInstance().getSite();

        longitude = site->longitude;
        latitude = site->latitude;
        timeZone = static_cast<int>(site->timeZone);


    }

    void STLSurfaceGroup::before(const core::SimTime &time) {

        if (!flag) {
            //获取经纬度，计算太阳方位角和高度角，更新数据
            auto [year, month, day, hour, min, sec] = time.getCurrentDateTime();
            auto [altitude, azimuth] = util::SunPosition::calculate_sun_position(longitude, latitude, timeZone, year, month, day, hour, min, sec);

            // 输出示例（北京地区正午）
            std::cout << std::fixed << std::setprecision(2);
            std::cout<<longitude<<" "<<latitude<<" "<<timeZone<<" "<< year<<" "<<month<<" "<< day<<" "<<hour<<" "<<min<<" "<<sec<<" "<<std::endl;
            std::cout << "太阳高度角: " << altitude << "°" << std::endl;  // 例如 60.5°
            std::cout << "太阳方位角: " << azimuth << "°" << std::endl;    // 例如 180°（正南）

            //获取辐射数据
            weather->before(time);


            const double rad1=weather->getOutputVal("rad1")["value"].get<double>();
            const double rad2=weather->getOutputVal("rad2")["value"].get<double>();
            //const double windspeed=weather->getOutputVal("wind_speed")["value"].get<double>();

            //std::cout << "直接辐射: " << rad1 << "W/m2" << std::endl;
            //std::cout << "天空散射辐射: " << rad2 << "W/m2" << std::endl;
            //std::cout << "风速: " << windspeed << "m/s" << std::endl;

            //std::cout<<"children"<<outputs <<std::endl;
            //std::cout<<"parents:"<<BaseComponent::outputs<<std::endl;

            for (const auto& surface_name : surface_names) {
                auto normal = surface_group->getAveNormal(surface_name);
                std::string shd = surface_name+"_shd";  //遮挡值
                std::string rad_direct  = surface_name+"_rad_direct";
                std::string rad_diffuse = surface_name+"_rad_diffuse";
                std::string rad_reflect = surface_name+"_rad_reflect";
                std::string rad_total   = surface_name+"_rad_total";
                double val = surface_group->get_shadow_value(altitude, azimuth, surface_name);
                //std::cout<<shd<<" "<<val<<" ";
                //std::cout<<"输出："<<outputs<<std::endl;
                this->outputs[shd]["value"] = val; //余弦值

                //辐射计算
                //地面反射
                std::cout<<"法向量"<<normal.x<<" "<<normal.y<<" "<<normal.z<<std::endl;
                std::vector n = {normal.x,normal.y,normal.z};
                auto [totRad, directRad, diffuseRad, reflectRad] = util::Radiation::calculateTotalRadiationOnSurface(
                    rad1, rad2,
                    n,
                    val,
                    altitude*M_PI/180.0,
                    0.12
                );

                this->outputs[rad_direct]["value"] = directRad;
                this->outputs[rad_diffuse]["value"] = diffuseRad;
                this->outputs[rad_reflect]["value"] = reflectRad;
                this->outputs[rad_total]["value"] = totRad;



            }
            //std::cout<<std::endl;



            flag = true;
        }




    }

    void STLSurfaceGroup::after(const core::SimTime &time) {
        flag = false;
    }

    void STLSurfaceGroup::parse(const json &in_params) {
        BaseComponent::parse(in_params);
        try {
            std::cout<<in_params<<std::endl;
            path = in_params[0].get<std::string>();

            // 天气数据
            std::string weather_name= in_params[1].get<std::string>();
            params["weather"]["value"] = weather_name;
            weather = core::SystemStateHub::getInstance().getComponent(weather_name);

            if (in_params.is_array() && in_params.size() > 1) {
                // 提取变量名列表（跳过前1个元素）
                for (size_t i = 2; i < in_params.size(); ++i) {
                    auto surf_name = in_params[i].get<std::string>();
                    std::cout<<surf_name<<std::endl;

                    surface_names.push_back(surf_name);


                    std::string shd = surf_name+"_shd";  //遮挡值
                    std::string rad_direct  = surf_name+"_rad_direct";
                    std::string rad_diffuse = surf_name+"_rad_diffuse";
                    std::string rad_reflect = surf_name+"_rad_reflect";
                    std::string rad_total   = surf_name+"_rad_total";

                    json var = {
                        {"name", shd},
                        {"type", "double"},
                        {"isInstValue", true},
                        {"value", 1.0}
                    };
                    addOutput(shd, var);

                    //辐射值
                    json rad_diffuse_var = {{"name", rad_diffuse},{"type", "double"},{"isInstValue", true},{"value", 0.0}};
                    json rad_direct_var = {{"name", rad_direct},{"type", "double"},{"isInstValue", true},{"value", 0.0}};
                    json rad_reflect_var = {{"name", rad_reflect},{"type", "double"},{"isInstValue", true},{"value", 0.0}};
                    json rad_total_var = {{"name", rad_total},{"type", "double"},{"isInstValue", true},{"value", 0.0}};

                    addOutput(rad_direct, rad_direct_var);
                    addOutput(rad_diffuse, rad_diffuse_var);
                    addOutput(rad_reflect, rad_reflect_var);
                    addOutput(rad_total, rad_total_var);





                }
            }

            std::cout<<"STL:"<<outputs<<std::endl;


        } catch (const json::parse_error& e) {
            std::cerr << "输入文件参数错误: " << e.what() << std::endl;
        }

    }
}

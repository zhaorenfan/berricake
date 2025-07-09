//
// Created by zhou on 25-6-13.
//

#include "EPWReader.h"

#include <iostream>


void comp::EPWReader::awake()  {
    const std::string jsonStr1 = R"({"params": {},
                              "inputs": {},
                              "outputs": {
                                            "dry_bulb_temp":  {"name": "dry_bulb_temp", "type": "double", "value": 0.0, "isInstValue": true},
                                            "wind_speed": {"name": "wind_speed", "type": "double", "value": 0.0, "isInstValue": true},
                                            "rad1": {"name": "rad1", "type": "double", "value": 0.0, "isInstValue": true},
                                            "rad2": {"name": "rad2", "type": "double", "value": 0.0, "isInstValue": true},
                                            "rad3": {"name": "rad3", "type": "double", "value": 0.0, "isInstValue": true}
                              }})";
    init_vars(jsonStr1);





}

void comp::EPWReader::before(const core::SimTime& time) {
    if (!flag) {
        // outputs["temp"]["value"] = getRealNumber();
        // outputs["wind_speed"]["value"] = getRandomNumber();
        double currentTime = time.currentTime / 3600.0;  // 转换为小时
        std::vector<float> data = getDataAtHour(currentTime);

        // 提取数据
        if (data.size() >= 5) {
            outputs["dry_bulb_temp"]["value"] = data[0];
            outputs["wind_speed"]["value"] = data[1];
            outputs["rad1"]["value"] = data[2];
            outputs["rad2"]["value"] = data[3];
            outputs["rad3"]["value"] = data[4];
        }

        flag = true;
    }
    //std::cout<<outputs<<std::endl;

}

void comp::EPWReader::sayHello() {
    std::cout<<"FileReader SayHello"<<std::endl;
}

void comp::EPWReader::parse(const json &in_params) {
    BaseComponent::parse(in_params);
    std::cout<<in_params<<std::endl;

    try {
        if (!in_params.empty()) {
            filename = in_params[0];
            std::cout << "天气文件路径: " << filename << std::endl;
            readFile();
        } else {
            std::cout << "处理天气模块的输入文件参数错误" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "读取天气文件时出错: " << e.what() << std::endl;
    }
}

void comp::EPWReader::after(const core::SimTime& time) {
    flag = false;
}

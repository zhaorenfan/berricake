//
// Created by zhou on 25-6-17.
//

#include "Output.h"

#include <iostream>

namespace comp {

    void Output::awake() {


        const std::string jsonStr1 = R"({"params": {},
                              "inputs": {},
                              "outputs": {}})";
        init_vars(jsonStr1);


    }

    void Output::after(const core::SimTime& time) {
        if (outFile.is_open()) {
            // 定义精度误差范围
            const double EPSILON = 1e-9; // 可根据需要调整这个值

            // 计算当前时间对60秒的余数
            double remainder = fmod(time.currentTime, this->interval*time.timeDelta);

            // 检查余数是否在误差范围内接近0
            if (fabs(remainder) < EPSILON) {
                if (outFile.is_open()) {
                    outFile << time.get_current_datetime_str() << ",";
                    for (const auto& var_name : var_names) {
                        if (inputs.contains(var_name)) {
                            outFile << inputs[var_name]["value"] << ",";
                        } else {
                            outFile << "nan" << ",";
                        }
                    }
                    outFile << "\n";
                }
            }
        }
    }

    Output::~Output() {
        if (outFile.is_open()) {
            outFile.close();
        }
    }

    void Output::parse(const json &in_params) {
        BaseComponent::parse(in_params);
        //std::cout<<in_params<<std::endl;

        try {
            //打开文件
            std::string file_name = in_params[0].get<std::string>();
            std::string interval_str = in_params[1].get<std::string>();
            this->interval = std::stod(interval_str);
            outFile.open(file_name);


            if (in_params.is_array() && in_params.size() > 1) {
                // 提取变量名列表（跳过前2个元素）
                for (size_t i = 2; i < in_params.size(); ++i) {
                    auto var_name = in_params[i].get<std::string>();


                    var_names.push_back(var_name);

                    // 初始化输入值为0.0
                    inputs[var_name] = json::object();
                    inputs[var_name]["value"] = 0.0;
                }
            }

            //std::cout<<"变量名："<<var_names<<std::endl;

            if (outFile.is_open()) {
                //std::cout<<"输出表头"<<std::endl;
                outFile << "Time"<<",";
                for (const auto& var_name : var_names) {
                    outFile << var_name<<",";
                }
                outFile <<"\n";
            }
        } catch (const json::parse_error& e) {
            std::cerr << "输入文件参数错误: " << e.what() << std::endl;
        }

    }
} // comp
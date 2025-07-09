//
// Created by zhou on 25-6-2.
//

#include "BaseComponent.h"
#include <iostream>

void core::BaseComponent::init_vars(const std::string &jsonStr) {
    try {
        json j = json::parse(jsonStr.begin(), jsonStr.end());
        // 判断是否存在"name"字段
        if (j.contains("params")) {
            //std::cout << "存在params字段" << std::endl;
            //params = j["params"];
            for (auto& element : j["params"].items()) {
                const auto& key = element.key();
                const auto& value = element.value();
                params[key] = value;
            }
        }
        if (j.contains("inputs")) {
            //std::cout << "存在inputs字段" << std::endl;
            for (auto& element : j["inputs"].items()) {
                const auto& key = element.key();
                const auto& value = element.value();
                inputs[key] = value;
            }
        }
        if (j.contains("outputs")) {
            //std::cout << "存在outputs字段" << std::endl;
            for (auto& element : j["outputs"].items()) {
                const auto& key = element.key();
                const auto& value = element.value();
                //std::cout << "Key: " << key << ", Value: " << value << std::endl;
                if (value.contains("isInstValue")) {
                    if (value["isInstValue"]==true) {
                        previous_outputs[key] = value["value"];
                    }
                    else {
                        previous_outputs_2[key] = value["value"];
                    }
                }
                outputs[key] = value;
            }


        }
    }catch (const json::parse_error& e) {
        std::cerr << "解析错误: " << e.what() << std::endl;
    }
}

void core::BaseComponent::update_previous_values() {
    // 遍历previous_outputs中的所有键
    //std::cout<<outputs<<std::endl;
    //std::cout<<previous_outputs<<std::endl;
    for (auto& item : previous_outputs.items()) {
        // 检查outputs中是否存在该键
        if (const std::string& k = item.key(); outputs.contains(k) && outputs[k].is_object()) {
            // 检查"value"字段是否存在
            if (outputs[k].contains("value")) {
                // 更新previous_outputs中的值
                //std::cout<<"更新的值："<<outputs[k]["value"]<<std::endl;
                previous_outputs[k] = outputs[k]["value"];

            }
        }
    }

}

void core::BaseComponent::update_previous_output_2() {
    // 遍历previous_outputs中的所有键
    for (auto& item : previous_outputs_2.items()) {
        // 检查outputs中是否存在该键
        if (const std::string& k = item.key(); outputs.contains(k) && outputs[k].is_object()) {
            // 检查"value"字段是否存在
            if (outputs[k].contains("value")) {
                // 更新previous_outputs中的值
                previous_outputs_2[k] = outputs[k]["value"];
            }
        }
    }
}

void core::BaseComponent::check_convergence(double threshold) {
    //std::cout<<"检查收敛性："<<name <<std::endl;
    //std::cout<<previous_outputs <<std::endl;
    //std::cout<<previous_outputs_2 <<std::endl;

    // 如果当前输出为空，认为收敛
    if (outputs.empty()) {
        //std::cout << "模块 "<<name<<" 无 outputs，认为收敛" << std::endl;
        is_converged = true;
        return;
        //return true;
    }


    // 如果没有历史值（首次迭代），认为未收敛
    if (previous_outputs.empty()) {
        //std::cout << "首次迭代，无历史值" << std::endl;
        is_converged = false;
        return;
        //return false;
    }

    // 遍历所有历史输出值
    for (auto& item : previous_outputs.items()) {
        const std::string& k = item.key();
        double previous_value = item.value().get<double>();


        // 检查当前输出中是否存在相同的键
        if (!outputs.contains(k) || !outputs[k].contains("value")) {
            // 若当前输出缺少历史键，视为未收敛
            //std::cout <<"模块 "<<name<< " 当前输出缺少键: " << k << "或缺失value"<<std::endl;
            is_converged = false;
            return;
            //return false;
        }

        // 获取当前值并计算残差
        double current_value = outputs[k]["value"].get<double>();
        double residual = std::abs(current_value - previous_value);

        std::cout<<name<<":" <<k <<":"<< previous_value << ", " << current_value << ", " << residual << std::endl;

        // 只要有一个残差超过阈值，即未收敛
        if (residual > threshold) {
            std::cout<< name << "不收敛" << std::endl;
            is_converged =false;
            return;
            //return false;
        }

    }

    // 所有变量均收敛
    //std::cout<<name << "收敛" << std::endl;
    is_converged = true;
    //return true;
}

void core::BaseComponent::set_previous_outputs_2_to_current() {
    // 遍历 previous_outputs_2 中的所有键值对
    //std::cout<<"set_previous_outputs_2_to_current"<<std::endl;
    //std::cout<<outputs<<std::endl;
    for (auto& item : previous_outputs_2.items()) {
        const std::string& key = item.key();

        // 检查 outputs 中是否存在该键
        if (outputs.contains(key) && outputs[key].is_object()) {
            // 检查 "value" 字段是否存在
            if (outputs[key].contains("value")) {
                // 将 previous_outputs_2 的值赋给 outputs[key]["value"]
                outputs[key]["value"] = item.value();
            }
        }
    }
}

void core::BaseComponent::parse(const json &in_params) {
    try {
        if (in_params.size() <= params.size() && in_params.size()<=(params.size()+inputs.size())) {
            std::cerr << "组件参数数量错误，应不小于参数数量 且 总数不大于参数和输入的总数 "<< std::endl;
            exit(1);
        }

    }catch (const json::parse_error& e) {
        std::cerr << "输入文件参数错误: " << e.what() << std::endl;
    }
}

void core::BaseComponent::addOutput(const std::string& name, const json& output_var) {
    /*{
        {"name", shd},
        {"type", "double"},
        {"isInstValue", true},
        {"value", 1}
    }*/
    //std::cout<<"添加输出"<<output_var<<std::endl;
    outputs[name] = output_var;
    if (output_var.contains("isInstValue")) {
        if (output_var["isInstValue"]==true) {
            previous_outputs[name] = output_var["value"];
        }
        else {
            previous_outputs_2[name] = output_var["value"];
        }
    }
    //std::cout<<"最新输出"<<outputs<<std::endl;
    //std::cout<<"最新输出"<<BaseComponent::outputs<<std::endl;


}

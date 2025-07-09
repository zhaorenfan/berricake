//
// Created by zhou on 25-6-12.
//

#include "WindModule.h"
#include <SystemStateHub.h>
#include <iostream>

void comp::WindModule::awake() {

    const std::string jsonStr1 = R"({"params": {
                                             "e": {"name": "e", "type": "double", "value": 0.2}
                              },
                              "inputs": {
                                             "wind_speed": {"name": "wind_speed", "type": "double", "value": 20.0}
                              },
                              "outputs": {
                                             "power":  {"name": "power", "type": "double", "value": 0.0, "isInstValue": true},
                                             "energy": {"name": "energy", "type": "double", "value": 0.0, "isInstValue": false}
                              }})";
    init_vars(jsonStr1);

}

void comp::WindModule::update(const core::SimTime& time) {
    //LOG_DEBUG("windModule");

    // std::cout<<params<<std::endl;
    // std::cout<<inputs<<std::endl;
    // std::cout<<outputs<<std::endl;
    //std::cout<<previous_outputs<<std::endl;
    //std::cout<<previous_outputs_2<<std::endl;
    const double e = params["e"]["value"];
    double wind_speed = inputs["wind_speed"]["value"];
    double power = e * wind_speed;
    outputs["power"]["value"] = power;
    double energy = outputs["energy"]["value"];
    outputs["energy"]["value"] = energy+power;

}

void comp::WindModule::parse(const json &in_params) {
    BaseComponent::parse(in_params);
    //in_params是一个数组
    try {
        std::cout<<in_params<<std::endl;
        params["e"]["value"] = in_params[0];
    }catch (const json::parse_error& e) {
        std::cerr << "输入文件参数错误: " << e.what() << std::endl;
    }
}

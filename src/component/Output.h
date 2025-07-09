//
// Created by zhou on 25-6-17.
//

#ifndef OUTPUT_H
#define OUTPUT_H
#include <BaseComponent.h>
#include <fstream>
#include <json.hpp>

namespace comp {

class Output :public core::BaseComponent{
private:
    std::ofstream outFile;

    json var_names;

    double interval = 1;


public:
    void awake() override;
    void after(const core::SimTime& time) override;
    ~Output() override;
    void parse(const json &in_params) override;
};

} // comp

#endif //OUTPUT_H

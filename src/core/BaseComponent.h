//
// Created by zhou on 25-6-2.
//

#ifndef BASECOMPONENT_H
#define BASECOMPONENT_H

#include <json.hpp>

#include "SimTime.h"


using json =  nlohmann::json;

namespace core {


    class BaseComponent {
    protected:
        std::string name; //组件名
        json params= json::object();
        json inputs = json::object();
        json outputs = json::object();
        json previous_outputs = json::object();   //瞬时值
        json previous_outputs_2 = json::object(); //非瞬时值

        bool is_converged = false;


    public:

        virtual void awake(){};
        virtual void before(const SimTime& time){};
        virtual void update(const SimTime& time) {};
        virtual void after(const SimTime& time){};
        virtual void sayHello(){}
        virtual ~BaseComponent()= default;

        void init_vars(const std::string& jsonStr);

        void update_previous_values();
        void update_previous_output_2();

        void check_convergence(double threshold);
        void set_previous_outputs_2_to_current();

        bool get_converged() const {
            return is_converged;
        }
        void set_converged(bool val) {
            this->is_converged = val;
        }

        json getOutputVal(const std::string &varname) {
            return this->outputs[varname];
        }

        void setInputVal(const std::string& varname, const json& val) {
            this->inputs[varname] = val;
        }

        virtual void parse(const json& in_params);

        void setName(std::string name) {
            this->name = name;
        }
        [[nodiscard]] std::string getName() const {
            return this->name;
        }


        void addOutput(const std::string& name, const json& output_var);
    };
}




#endif //BASECOMPONENT_H

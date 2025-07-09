//
// Created by zhou on 25-6-12.
//

#ifndef WINDMODULE_H
#define WINDMODULE_H


#include <BaseComponent.h>


namespace comp {
class WindModule: public core::BaseComponent{
private:

public:
    void awake() override;
    void update(const core::SimTime& time) override;
    void parse(const json &in_params) override;

};
}




#endif //WINDMODULE_H

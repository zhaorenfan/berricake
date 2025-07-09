//
// Created by zhou on 25-6-18.
//

#ifndef LINK_H
#define LINK_H
#include <memory>
#include "BaseComponent.h"


namespace core {

class Link {
private:
    std::string source;
    std::string source_variable;
    std::string target;
    std::string target_variable;

    std::shared_ptr<BaseComponent> source_component;
    std::shared_ptr<BaseComponent> target_component;
public:
    // 构造函数
    Link(const std::string& sourceComp, const std::string& sourceVar,
         const std::string& targetComp, const std::string& targetVar);

    // 更新链接
    void update();


};

} // core

#endif //LINK_H

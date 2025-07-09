//
// Created by zhou on 25-6-18.
//

#include "Link.h"

#include <iostream>

#include "SystemStateHub.h"

namespace core {

    Link::Link(const std::string &sourceComp, const std::string &sourceVar, const std::string &targetComp,
        const std::string &targetVar) :
        source(sourceComp),
        source_variable(sourceVar),
        target(targetComp),
        target_variable(targetVar){

        // 参数验证
        if (sourceComp.empty() || targetComp.empty()) {
            throw std::invalid_argument("源模块和目标模块不能为空");
        }

        if (sourceVar.empty() || targetVar.empty()) {
            throw std::invalid_argument("源变量和目标变量不能为空");
        }

        source_component = SystemStateHub::getInstance().getComponent(source);
        target_component = SystemStateHub::getInstance().getComponent(target);

    }

    void Link::update() {

        try {
            // 获取源值
            json source_val = source_component->getOutputVal(source_variable);

            target_component->setInputVal(target_variable, source_val);


        } catch (const std::exception& e) {
            std::cerr << "更新链接错误: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "更新链接时发生未知错误" << std::endl;
        }

    }
} // core
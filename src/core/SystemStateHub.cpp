#include "SystemStateHub.h"
#include <iostream>

namespace core {


    // 方法实现
    void SystemStateHub::sayHello(const std::string& name) {
        std::cout << "Hello, " << name << " from SystemStateHub!" << std::endl;
    }

    bool SystemStateHub::registerComponent(const std::string& componentName,
                                          std::shared_ptr<BaseComponent> component) {
        if (components.find(componentName) != components.end()) {
            return false; // 组件已存在
        }
        components[componentName] = component;
        return true;
    }

    void SystemStateHub::unregisterComponent(const std::string& componentName) {
        components.erase(componentName);
    }

    std::shared_ptr<BaseComponent> SystemStateHub::getComponent(const std::string& componentName) const {
        auto it = components.find(componentName);
        if (it != components.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::vector<std::string> SystemStateHub::getAllComponentNames() const {
        std::vector<std::string> names;
        names.reserve(components.size());
        for (const auto& pair : components) {
            names.push_back(pair.first);
        }
        return names;
    }

    void SystemStateHub::forEachComponent(const std::function<void(const std::string&,
                                                                  std::shared_ptr<BaseComponent>)>& func) {
        for (const auto& pair : components) {
            func(pair.first, pair.second);
        }
    }

    bool SystemStateHub::createLink(const std::string &source_component, const std::string &source_variable,
        const std::string &target_component, const std::string &target_variable) {
        //std::cout<<"创建Link"<<std::endl;
        if (!components.contains(source_component)) {
            return false;
        }
        if (!components.contains(target_component)) {
            return false;
        }
        std::shared_ptr<Link> link = std::make_shared<Link>(source_component, source_variable, target_component, target_variable);
        links.push_back(link);

        return true;
    }
} // namespace core

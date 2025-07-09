#ifndef COMPONENTFACTORY_H
#define COMPONENTFACTORY_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <iostream>

#include "BaseComponent.h"

namespace core {


class ComponentFactory {
private:
    using CreateFn = std::function<std::shared_ptr<BaseComponent>()>;
    static std::unordered_map<std::string, CreateFn>& getRegistry() {
        static std::unordered_map<std::string, CreateFn> registry;
        return registry;
    }
    
    static std::mutex& getMutex() {
        static std::mutex mutex;
        return mutex;
    }

public:
    static void registerClass(const std::string& className, CreateFn creator) {
        std::lock_guard<std::mutex> lock(getMutex());
        auto& registry = getRegistry();
        if (registry.find(className) != registry.end()) {
            std::cerr << "Warning: Component " << className << " already registered!" << std::endl;
            return;
        }
        registry[className] = creator;
    }

    static std::shared_ptr<BaseComponent> create(const std::string& className) {
        std::lock_guard<std::mutex> lock(getMutex());
        auto& registry = getRegistry();
        auto it = registry.find(className);
        if (it != registry.end()) {
            return it->second();
        }
        std::cerr << "Error: Component " << className << " not registered!" << std::endl;
        return nullptr;
    }
};

// 改进的注册宏 - 使用局部静态变量确保初始化
#define REGISTER_COMPONENT(className) \
    namespace { \
        struct className##Registrar { \
            className##Registrar() { \
                ::core::ComponentFactory::registerClass(#className, []() { \
                    return std::make_shared<className>(); \
                }); \
            } \
            static void init() { \
                static className##Registrar instance; \
            } \
        }; \
        __attribute__((constructor)) \
        static void className##RegistrarInit() { \
            className##Registrar::init(); \
        } \
    }

} // namespace core

#endif //COMPONENTFACTORY_H
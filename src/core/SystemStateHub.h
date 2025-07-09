#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "BaseComponent.h"
#include "Link.h"
#include "Site.h"

namespace core {
    class SystemStateHub {
    private:
        // 存储组件的映射表
        std::unordered_map<std::string, std::shared_ptr<BaseComponent>> components;
        // Link
        std::vector<std::shared_ptr<Link>> links;
        // Site
        std::shared_ptr<Site> site;

        // 私有构造函数和拷贝控制
        SystemStateHub() {
            components = {};
            site=std::make_shared<Site>();
            site->name = "default_shenyang";
            site->timeZone=8;
            site->latitude=41.8;
            site->longitude=123.43;
            site->elevation = 45;
        };
        SystemStateHub(const SystemStateHub&) = delete;
        SystemStateHub& operator=(const SystemStateHub&) = delete;

    public:
        // 获取单例实例的静态方法，返回引用
        static SystemStateHub& getInstance() {
            static SystemStateHub instance;  // C++11后线程安全的局部静态变量

            return instance;
        }

        static void sayHello(const std::string& name);

        // 注册组件
        bool registerComponent(const std::string& componentName, std::shared_ptr<BaseComponent> component);

        // 移除组件
        void unregisterComponent(const std::string& componentName);

        // 获取组件
        std::shared_ptr<BaseComponent> getComponent(const std::string& componentName) const;

        // 获取所有组件名称
        std::vector<std::string> getAllComponentNames() const;

        // 对所有组件执行操作
        void forEachComponent(const std::function<void(const std::string&, std::shared_ptr<BaseComponent>)>& func);

        //创建Link
        bool createLink(const std::string& source_component, const std::string& source_variable,
             const std::string& target_component, const std::string& target_variable);

        void update_links() const {

            for (const auto& link : links) {
                link->update();
            }
        }



        //地理位置
        void setSite(const json& site_info) const {
            site->name = site_info["name"].get<std::string>();
            site->timeZone = site_info["timezone"].get<double>();
            site->latitude = site_info["latitude"].get<double>();
            site->longitude = site_info["longitude"].get<double>();
            site->elevation = site_info["elevation"].get<double>();
        }

        std::shared_ptr<Site> getSite() const {
            return site;
        }

    };
} // namespace core

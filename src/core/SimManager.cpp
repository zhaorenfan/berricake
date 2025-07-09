//
// Created by zhou on 25-6-12.
//
#include "SimManager.h"

#include "SystemStateHub.h"
#include <Parser.h>
#include "ComponentFactory.h"

namespace core {
    SimManager::SimManager(): time(0,10,1) {
        //time = SimTime(0, 10, 1);
        run_periods = json::array();
    }

    void SimManager::parse_file(const std::string& in_file) {
        const auto res = util::Parser::file_to_json(in_file);
        std::cout<<"in.idf"<<std::endl;
        std::cout<<res<<std::endl;
        const json& modules = res["modules"];
        const json& links = res["links"];
        const json& site = res["site"];
        this->run_periods = res["run_periods"];
        this->timestep = res["timestep"];
        this->max_iterations = std::stoi(res["control"][2].get<std::string>());

        // std::cout<<modules<<std::endl;
        // std::cout<<links<<std::endl;
        for (auto module : modules) {
            std::string class_name = module["class_name"];
            std::string object_name = module["object_name"];
            json params = module["params"];
            std::cout<<"创建实例"<<class_name<<" "<<"实例名"<<object_name<<std::endl;
            auto comp_instance = ComponentFactory::create(class_name);
            comp_instance->setName(object_name);

            //组件初始化
            comp_instance->parse(params);

            //全局注册
            SystemStateHub::getInstance().registerComponent(object_name, comp_instance);

        }

        for (auto link : links) {
            if (link.size()!=4) {
                std::cout<<"连接参数数目不匹配，应为4"<<std::endl;
                exit(1);
            }
            std::string src = link[0];
            std::string src_var = link[1];
            std::string trg = link[2];
            std::string trg_var = link[3];

            std::cout<<"创建连接 从 "<<src<<" 变量 "<<src_var<<" 连接到 "<<trg<<" 的 "<<trg_var<<std::endl;

            bool isOk = SystemStateHub::getInstance().createLink(src, src_var, trg, trg_var);

            if (!isOk) {
                exit(1);
            }
        }

        SystemStateHub::getInstance().setSite(site);

    }

    bool SimManager::run_a_step(const SimTime& time) {
        //LOG_DEBUG("运行一步");

        int iteration = 0; //当前时间步
        bool converged = false; //是否收敛

        // 每个时间步开始时，重置模块的收敛状态
        SystemStateHub::getInstance().forEachComponent([](const std::string& name, const std::shared_ptr<BaseComponent> &component) {
            component->set_converged(false);
        });

        SystemStateHub::getInstance().forEachComponent([time](const std::string& name, const std::shared_ptr<BaseComponent> &component) {
            component->before(time);
        });

        while (iteration<max_iterations && !converged) {
            // 1. 更新所有模块的前一次输出值

            SystemStateHub::getInstance().forEachComponent([](const std::string& name, const std::shared_ptr<BaseComponent>& component) {
                component->update_previous_values();
            });

            // 2. 执行模块计算（假设模块的 update 方法会更新输出）

            SystemStateHub::getInstance().forEachComponent([time](const std::string& name, const std::shared_ptr<BaseComponent> &component) {
                component->update(time);
            });

            // # 3. 通过 Link 同步变量
            SystemStateHub::getInstance().update_links();

            // 4. 检查每个模块的收敛状态
            SystemStateHub::getInstance().forEachComponent([this](const std::string& name, const std::shared_ptr<BaseComponent> &component) {
                component->check_convergence(convergence_threshold);
            });
            // 5. 检查全局收敛
            converged = check_global_convergence();

            if (converged==false) {
                SystemStateHub::getInstance().forEachComponent([this](const std::string& name, const std::shared_ptr<BaseComponent>& component) {
                    component->set_previous_outputs_2_to_current();
                });
            }
            iteration += 1;
        }
        if (converged==true) {
            SystemStateHub::getInstance().forEachComponent([this, time](const std::string& name, const std::shared_ptr<BaseComponent> &component) {
                component->update_previous_values();
                component->update_previous_output_2();
                component->after(time);

            });
        }
        if (iteration>max_iterations) {
            const std::string msg = "超出最大迭代次数"+ std::to_string(iteration);
            std::cout<<msg<<std::endl<<"请检查输入文件，或修改最大迭代次数"<<std::endl;
            exit(1);
        }

        //LOG_DEBUG("运行一步结束");
        return converged;
    }

    bool SimManager::check_global_convergence() {
        bool all_converged = true;

        SystemStateHub::getInstance().forEachComponent(
            [&all_converged](const std::string& name, const std::shared_ptr<BaseComponent>& component) {
                std::cout<<component->getName()<<"收敛情况："<<component->get_converged()<<std::endl;
                if (!component->get_converged()) {
                    all_converged = false;  // 标记为未收敛
                    // 注意：不能提前 return，需继续遍历所有组件
                }
            }
        );
        std::cout<<"全局收敛："<<all_converged<<std::endl;

        return all_converged;  // 返回最终结果
    }

    void SimManager::run() {
        SystemStateHub::getInstance().forEachComponent([](const std::string& name, const std::shared_ptr<core::BaseComponent>& component) {
            component->awake();
        });

        time.timeDelta = this->timestep;

        for (const auto& run_period : this->run_periods) {
            //std::cout<<run_period<<std::endl;

            time.currentTime = 0;

            // 解析时间参数
            try {
                // 使用get<std::string>()先获取字符串，再转为整数
                time.startYear = std::stoi(run_period["params"][0].get<std::string>());
                time.startMonth = std::stoi(run_period["params"][1].get<std::string>());
                time.startDay = std::stoi(run_period["params"][2].get<std::string>());
                time.endYear = std::stoi(run_period["params"][3].get<std::string>());
                time.endMonth = std::stoi(run_period["params"][4].get<std::string>());
                time.endDay = std::stoi(run_period["params"][5].get<std::string>());

                time.calcEndTime();

                // 使用解析后的时间数据
                std::cout<< run_period["object_name"].get<std::string>() << " Start date: " << time.startYear << "-"
                          << time.startMonth << "-" << time.startDay << " ";
                std::cout << "End date: " << time.endYear << "-"
                          << time.endMonth << "-" << time.endDay << std::endl;

                while (time.currentTime<=time.endTime) {
                    if (!run_a_step(time)) {
                        std::cout<<"收敛失败"<<std::endl;
                        std::cout<<"时间："<<time.currentTime<<std::endl;
                        exit(1);
                    }
                    time.advanceTime();
                }

            } catch (const std::exception& e) {
                std::cerr << "Error parsing time parameters: " << e.what() << std::endl;
            }




        }


    }

} // core
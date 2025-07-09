//
// Created by zhou on 25-6-19.
//

#include "Parser.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>



namespace util {
    json Parser::file_to_json(const std::string &file_path) {
        std::string content;
        try {
            std::ifstream file(file_path);
            if (!file.is_open()) {
                std::cerr << "错误：文件 " << file_path << " 未找到" << std::endl;
                return json();
            }

            content.assign((std::istreambuf_iterator<char>(file)),
                          (std::istreambuf_iterator<char>()));
            file.close();
        } catch (const std::exception& e) {
            std::cerr << "错误：读取文件时发生异常: " << e.what() << std::endl;
            return {};
        }

        json result = {
            {"site", json::object()},
            {"control", json::array()},
            {"timestep", 3600.0},
            {"run_periods", json::array()},
            {"modules", json::array()},
            {"links", json::array()}
        };

        std::vector<std::string> lines;
        std::string line;
        for (char c : content) {
            if (c == '\n') {
                lines.push_back(line);
                line.clear();
            } else {
                line += c;
            }
        }
        if (!line.empty()) lines.push_back(line);

        for (size_t index = 0; index < lines.size();) {
            std::string element_str;
            while (true) {
                std::string line = lines[index];
                // 去除制表符和换行符
                line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
                line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());

                // 去除注释
                size_t comment_pos = line.find('!');
                if (comment_pos != std::string::npos) {
                    line = line.substr(0, comment_pos);
                }

                // 去除首尾空格
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);

                element_str += line;
                if (element_str.back() == ';') {
                    break;
                }

                index++;
                if (index >= lines.size()) {
                    break;
                }
            }

            // 移除末尾的分号
            if (!element_str.empty() && element_str.back() == ';') {
                element_str.pop_back();
            }

            // 分割元素
            std::vector<std::string> parts;
            std::string part;
            bool in_quote = false;
            for (char c : element_str) {
                if (c == '"') {
                    in_quote = !in_quote;
                }
                if (c == ',' && !in_quote) {
                    // 去除首尾空格
                    part.erase(0, part.find_first_not_of(" \t"));
                    part.erase(part.find_last_not_of(" \t") + 1);
                    if (!part.empty()) {
                        parts.push_back(part);
                    }
                    part.clear();
                } else {
                    part += c;
                }
            }
            // 添加最后一个部分
            part.erase(0, part.find_first_not_of(" \t"));
            part.erase(part.find_last_not_of(" \t") + 1);
            if (!part.empty()) {
                parts.push_back(part);
            }

            if (parts.empty()) {  // 空行
                index++;
                continue;
            }

            if (parts[0] == "Site") {
                if (parts.size() == 6) {
                    try {
                        result["site"] = {
                            {"name", parts[1]},
                            {"timezone", std::stof(parts[2])},
                            {"latitude", std::stof(parts[3])},
                            {"longitude", std::stof(parts[4])},
                            {"elevation", std::stof(parts[5])}
                        };
                    } catch (const std::exception& e) {
                        std::cerr << "警告：解析 Site 时发生错误: " << e.what() << std::endl;
                    }
                }
            } else if (parts[0] == "SimulationControl") {
                result["control"] = json::array();
                for (size_t i = 1; i < parts.size(); ++i) {
                    result["control"].push_back(parts[i]);
                }
            } else if (parts[0] == "Timestep") {
                if (parts.size() == 2) {
                    try {
                        result["timestep"] = std::stof(parts[1]);
                    } catch (const std::exception& e) {
                        std::cerr << "警告：解析 Timestep 时发生错误: " << e.what() << std::endl;
                    }
                }
            } else if (parts[0] == "Link") {
                json link = json::array();
                for (size_t i = 1; i < parts.size(); ++i) {
                    link.push_back(parts[i]);
                }
                result["links"].push_back(link);
            } else if (parts[0] == "RunPeriod") {
                json run_period = {
                    {"class_name", "RunPeriod"},
                    {"object_name", parts.size() > 1 ? parts[1] : ""},
                    {"params", json::array()}
                };
                for (size_t i = 2; i < parts.size(); ++i) {
                    run_period["params"].push_back(parts[i]);
                }
                result["run_periods"].push_back(run_period);
            } else {
                json module = {
                    {"class_name", parts[0]},
                    {"object_name", parts.size() > 1 ? parts[1] : ""},
                    {"params", json::array()}
                };
                for (size_t i = 2; i < parts.size(); ++i) {
                    module["params"].push_back(parts[i]);
                }
                result["modules"].push_back(module);
            }

            index++;
        }

        return result;
    }
} // util
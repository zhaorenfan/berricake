//
// Created by zhou on 25-6-28.
//

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "Point3D.h"

namespace geom {
    struct Triangle {
        Point3D normal;
        Point3D vertices[3];
        int id=-1;
    };

    using SolidData = std::vector<Triangle>;
    using StlModel = std::map<std::string, SolidData>;


inline StlModel parseStlFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + filePath);
    }

    StlModel model;
    std::string currentSolidName;
    Triangle currentTriangle;  // 移到循环外，确保一个facet内复用
    int vertexCount = 0;
    bool inSolid = false;
    bool inFacet = false;
    bool inLoop = false;  // 新增：标记是否在outer loop内
    int triangleId = 0;   // 三角形ID（按顺序递增）

    std::string line;

    // 正则表达式模式（新增outer loop和endloop）
    std::regex solidPattern(R"(^\s*solid\s+(\w+)\s*$)");
    std::regex endsolidPattern(R"(^\s*endsolid\s*(\w+)?\s*$)");
    std::regex facetPattern(R"(^\s*facet\s+normal\s+([+-]?\d+(?:\.\d+)?)\s+([+-]?\d+(?:\.\d+)?)\s+([+-]?\d+(?:\.\d+)?)\s*$)");
    std::regex loopPattern(R"(^\s*outer\s+loop\s*$)");          // 新增：匹配outer loop
    std::regex endloopPattern(R"(^\s*endloop\s*$)");            // 新增：匹配endloop
    std::regex vertexPattern(R"(^\s*vertex\s+([+-]?\d+(?:\.\d+)?)\s+([+-]?\d+(?:\.\d+)?)\s+([+-]?\d+(?:\.\d+)?)\s*$)");
    std::regex endfacetPattern(R"(^\s*endfacet\s*$)");          // 新增：匹配endfacet

    while (std::getline(file, line)) {
        // 去除行首行尾空白
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        if (line.empty()) continue;

        std::smatch match;

        // 1. 匹配solid开始
        if (std::regex_match(line, match, solidPattern)) {
            currentSolidName = match[1].str();
            inSolid = true;
            continue;
        }

        // 2. 匹配solid结束
        if (std::regex_match(line, match, endsolidPattern)) {
            inSolid = false;
            inFacet = false;
            inLoop = false;
            continue;
        }

        if (!inSolid) continue;  // 不在solid内则跳过

        // 3. 匹配facet开始（此时初始化三角形）
        if (std::regex_match(line, match, facetPattern)) {
            currentTriangle = Triangle();  // 仅在facet开始时创建新三角形
            currentTriangle.normal.x = std::stod(match[1].str());
            currentTriangle.normal.y = std::stod(match[2].str());
            currentTriangle.normal.z = std::stod(match[3].str());
            inFacet = true;
            inLoop = false;
            vertexCount = 0;  // 重置顶点计数
            continue;
        }

        // 4. 匹配outer loop（仅在facet内有效）
        if (inFacet && std::regex_match(line, loopPattern)) {
            inLoop = true;  // 进入loop后才开始解析顶点
            continue;
        }

        // 5. 匹配endloop（结束顶点解析）
        if (inFacet && std::regex_match(line, endloopPattern)) {
            inLoop = false;
            continue;
        }

        // 6. 匹配endfacet（结束当前facet）
        if (inFacet && std::regex_match(line, endfacetPattern)) {
            inFacet = false;
            continue;
        }

        // 7. 解析顶点（仅在loop内有效）
        if (inLoop && std::regex_match(line, match, vertexPattern)) {
            if (vertexCount < 3) {
                // 正确存储顶点数据（currentTriangle在整个facet内复用）
                currentTriangle.vertices[vertexCount].x = std::stod(match[1].str());
                currentTriangle.vertices[vertexCount].y = std::stod(match[2].str());
                currentTriangle.vertices[vertexCount].z = std::stod(match[3].str());
                vertexCount++;

                // 收集到3个顶点后，添加到模型
                if (vertexCount == 3) {
                    currentTriangle.id = triangleId++;  // 按顺序分配ID
                    model[currentSolidName].push_back(currentTriangle);
                    // 调试输出（可选）
                    // std::cout << "解析三角面 " << currentTriangle.id << "：\n"
                    //           << "  顶点0：" << currentTriangle.vertices[0].x << ","
                    //           << currentTriangle.vertices[0].y << "," << currentTriangle.vertices[0].z << "\n"
                    //           << "  顶点1：" << currentTriangle.vertices[1].x << ","
                    //           << currentTriangle.vertices[1].y << "," << currentTriangle.vertices[1].z << "\n"
                    //           << "  顶点2：" << currentTriangle.vertices[2].x << ","
                    //           << currentTriangle.vertices[2].y << "," << currentTriangle.vertices[2].z << "\n";
                }
            }
        }
    }

    return model;
}

    // 打印解析结果
    inline void printStlModel(const StlModel& model) {
        for (const auto& pair : model) {
            const std::string& solidName = pair.first;
            const SolidData& triangles = pair.second;

            std::cout << "===== 三角面: " << solidName << " =====" << std::endl;
            std::cout << "包含 " << triangles.size() << " 个三角面" << std::endl << std::endl;

            for (size_t i = 0; i < triangles.size(); ++i) {
                const Triangle& tri = triangles[i];
                std::cout << "三角面 " << i + 1 << ":" << std::endl;
                std::cout << "  法向量: ("
                          << tri.normal.x << ", "
                          << tri.normal.y << ", "
                          << tri.normal.z << ")" << std::endl;
                for (int j = 0; j < 3; ++j) {
                    const Point3D& vertex = tri.vertices[j];
                    std::cout << "  顶点 " << j + 1 << ": ("
                              << vertex.x << ", "
                              << vertex.y << ", "
                              << vertex.z << ")" << std::endl;
                }
                std::cout << std::endl;  // 每个三角面之间空一行
            }
            std::cout << "=====================================" << std::endl << std::endl;
        }
    }


    // 将细分后的模型写回STL文件
    inline void writeStlFile(const StlModel& model, const std::string& filePath) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开文件进行写入: " + filePath);
        }

        for (const auto& pair : model) {
            const std::string& solidName = pair.first;
            const SolidData& triangles = pair.second;

            file << "solid " << solidName << "\n";

            for (const auto& tri : triangles) {
                file << "  facet normal "
                     << tri.normal.x << " "
                     << tri.normal.y << " "
                     << tri.normal.z << "\n";
                file << "    outer loop\n";
                for (const auto & vertice : tri.vertices) {
                    file << "      vertex "
                         << vertice.x << " "
                         << vertice.y << " "
                         << vertice.z << "\n";
                }
                file << "    endloop\n";
                file << "  endfacet\n";
            }

            file << "endsolid " << solidName << "\n";
        }
    }

}




#endif //TRIANGLE_H

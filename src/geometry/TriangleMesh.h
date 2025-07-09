//
// Created by zhou on 25-6-27.
//

#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include <algorithm>
#include <array>
#include <vector>
#include <cmath>
#include <iostream>
#include <map>

#include "Point3D.h"
#include "Triangle.h"

namespace geom {
    // 计算三角形面积
inline double triangle_area(const Point3D& a, const Point3D& b, const Point3D& c) {
    const Point3D ab = b - a;
    const Point3D ac = c - a;
    const Point3D cross_product = cross(ab, ac);
    return 0.5 * sqrt(cross_product.x * cross_product.x +
                      cross_product.y * cross_product.y +
                      cross_product.z * cross_product.z);
}

struct TriangleMesh {
    std::vector<Point3D> vertices;
    std::vector<std::vector<size_t>> faces;
    std::vector<Point3D> centers;  //中心点
    std::vector<Point3D> normals;  //法向量
    std::vector<double> areas; //面积
    std::vector<int> levels;
    int parent_id=-1;  //父表面的id
    double original_area;
    double avg_area;
};

inline double distance(const Point3D& a, const Point3D& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) +
                    std::pow(a.y - b.y, 2) +
                    std::pow(a.z - b.z, 2));
}

inline TriangleMesh adaptive_subdivide_with_centers(const Point3D& v1,
                                                    const Point3D& v2,
                                                    const Point3D& v3,
                                                    const int face_id,
                                                    const Point3D& normal,  // 新增参数：原始法向量
                                                    const double min_edge_length = 0.5,
                                                    const int max_level = 5) {
    TriangleMesh mesh;
    mesh.parent_id = face_id;
    std::map<std::pair<size_t, size_t>, size_t> edge_midpoints;

    // 计算原始三角形面积
    mesh.original_area = triangle_area(v1, v2, v3);
    //std::cout<<"PT"<<v1.x<<v1.y<<v1.z<<v2.x<<v2.y<<v2.z<<v3.x<<v3.y<<v3.z<<"Area"<<mesh.original_area<<std::endl;

    auto get_midpoint = [&](size_t a, size_t b) {
        auto key = std::make_pair(std::min(a, b), std::max(a, b));
        if (!edge_midpoints.contains(key)) {
            edge_midpoints[key] = mesh.vertices.size();
            mesh.vertices.emplace_back(
                (mesh.vertices[a].x + mesh.vertices[b].x) / 2,
                (mesh.vertices[a].y + mesh.vertices[b].y) / 2,
                (mesh.vertices[a].z + mesh.vertices[b].z) / 2
            );
        }
        return edge_midpoints[key];
    };

    // 初始化
    mesh.vertices = {v1, v2, v3};
    mesh.faces = {{0, 1, 2}};
    mesh.centers.emplace_back(
        (v1.x + v2.x + v3.x) / 3,
        (v1.y + v2.y + v3.y) / 3,
        (v1.z + v2.z + v3.z) / 3
    );

    // 使用传入的法向量，而不是计算
    mesh.normals.push_back(normal);
    mesh.levels.push_back(0);
    mesh.areas.push_back(mesh.original_area);  // 初始化第一个面的面积

    for (int level = 1; level <= max_level; ++level) {
        std::vector<std::vector<size_t>> new_faces;
        std::vector<Point3D> new_centers;
        std::vector<Point3D> new_normals; // 新增：存储新面的法向量
        std::vector<double> new_areas;  // 存储新面的面积
        bool needs_subdivision = false;

        for (size_t idx = 0; idx < mesh.faces.size(); ++idx) {
            const auto& face = mesh.faces[idx];

            // 计算边长
            std::array<double, 3> edge_lengths = {
                distance(mesh.vertices[face[1]], mesh.vertices[face[0]]),
                distance(mesh.vertices[face[2]], mesh.vertices[face[1]]),
                distance(mesh.vertices[face[0]], mesh.vertices[face[2]])
            };

            // 检查是否需要细分
            if (std::ranges::any_of(edge_lengths,
                                    [&](double len) { return len > 1.5 * min_edge_length; })) {
                needs_subdivision = true;

                // 获取中点
                size_t mid0 = get_midpoint(face[0], face[1]);
                size_t mid1 = get_midpoint(face[1], face[2]);
                size_t mid2 = get_midpoint(face[2], face[0]);

                // 创建子三角形
                std::vector<std::vector<size_t>> sub_tris = {
                    {face[0], mid0, mid2},
                    {face[1], mid1, mid0},
                    {face[2], mid2, mid1},
                    {mid0, mid1, mid2}
                };

                // 计算每个子三角形的中心点，并保留原始法向量
                for (const auto& tri : sub_tris) {
                    const Point3D& a = mesh.vertices[tri[0]];
                    const Point3D& b = mesh.vertices[tri[1]];
                    const Point3D& c = mesh.vertices[tri[2]];
                    // 计算子三角形面积（核心新增逻辑）
                    double sub_area = triangle_area(a, b, c);

                    // 计算中心点
                    new_centers.emplace_back(
                        (a.x + b.x + c.x) / 3,
                        (a.y + b.y + c.y) / 3,
                        (a.z + b.z + c.z) / 3
                    );

                    // 所有子三角形都使用原始法向量
                    new_normals.push_back(mesh.normals[idx]);

                    new_faces.push_back(tri);
                    new_areas.push_back(sub_area);
                }
            } else {
                // 如果不需要细分，保留原面及其法向量
                new_faces.push_back(face);
                new_centers.push_back(mesh.centers[idx]);
                new_normals.push_back(mesh.normals[idx]);
                new_areas.push_back(mesh.areas[idx]);  // 保留原面积
            }
        }

        // 更新网格数据
        mesh.faces = new_faces;
        mesh.centers = new_centers;
        mesh.normals = new_normals;
        mesh.areas = new_areas;  // 更新面积列表
        mesh.levels = std::vector<int>(mesh.faces.size(), level);

        if (!needs_subdivision) break;
    }

    // 计算平均每个三角形的面积
    mesh.avg_area = mesh.original_area / static_cast<double>(mesh.faces.size());

    return mesh;
}

inline void print_mesh(const TriangleMesh& mesh) {
    std::cout << "顶点列表 (" << mesh.vertices.size() << "个):\n";
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        const auto& v = mesh.vertices[i];
        std::cout << "  " << i << ": (" << v.x << ", " << v.y << ", " << v.z << ")\n";
    }

    std::cout << "\n三角形面 (" << mesh.faces.size() << "个):\n";
    for (size_t i = 0; i < mesh.faces.size(); ++i) {
        std::cout << "  三角形" << i << ": [";
        for (auto idx : mesh.faces[i]) {
            std::cout << idx << " ";
        }
        std::cout << "] 中心点(" << mesh.centers[i].x << ", "
                  << mesh.centers[i].y << ", " << mesh.centers[i].z
                  << ") "<<"法向向量"<< mesh.normals[i].x << ", " << mesh.normals[i].y << ", " << mesh.normals[i].z
                      <<"层级:" << mesh.levels[i] << "\n";
    }

    std::cout << "\n面积信息:\n";
    std::cout << "- 原始三角形面积: " << mesh.original_area << "\n";
    std::cout << "- 细分后三角形数量: " << mesh.faces.size() << "\n";
    std::cout << "- 平均每个三角形面积: " << mesh.avg_area << "\n";
    std::cout << "- 最大细分层级: " << *std::ranges::max_element(mesh.levels) << "\n";
}

    // 将STL模型中的每个三角形细分并转换为TriangleMesh
    // 只要特定的名称
inline std::map<std::string, std::vector<TriangleMesh>> subdivideModel(const StlModel& model,
                                                                       const std::vector<std::string>& surfNames,
                                                                       double minEdgeLength = 0.5,
                                                                       int maxLevel = 5
                                                                               ) {
    std::map<std::string, std::vector<TriangleMesh>> result;



    std::cout << "正在将STL模型中的每个三角形细分为TriangleMesh..." << std::endl;
    std::cout << "最小边长度阈值: " << minEdgeLength << ", 最大细分层级: " << maxLevel << std::endl;

    size_t totalOriginalTriangles = 0;
    size_t totalSubdividedTriangles = 0;

    // 遍历每个solid
    for (const auto& pair : model) {
        const std::string& solidName = pair.first;

        // 检查 solidName 是否在 surf_names 中
        if (std::ranges::find(surfNames, solidName) == surfNames.end()) {
            continue; // 不存在，跳过
        }

        const SolidData& triangles = pair.second;

        std::vector<TriangleMesh> solidMeshes;
        totalOriginalTriangles += triangles.size();

        std::cout << "处理 三角面: " << solidName << " (" << triangles.size() << " 个三角形)" << std::endl;

        // 遍历每个三角形并进行细分
        int sum=0;
        for (const auto& tri : triangles) {
            // 使用现有的自适应细分方法处理每个三角形
            TriangleMesh mesh = adaptive_subdivide_with_centers(
                tri.vertices[0], tri.vertices[1], tri.vertices[2],
                tri.id,
                tri.normal,
                minEdgeLength, maxLevel
            );

            solidMeshes.push_back(mesh);
            totalSubdividedTriangles += mesh.faces.size();

            sum = mesh.faces.size();

        }

        result[solidName] = solidMeshes;
        std::cout << "  三角面 '" << solidName << "' 细分完成，生成 "
                  << sum << " 个网格" << std::endl;
    }

    std::cout << "细分完成！" << std::endl;
    std::cout << "- 原始三角形总数: " << totalOriginalTriangles << std::endl;
    std::cout << "- 细分后三角形总数: " << totalSubdividedTriangles << std::endl;

    return result;
}

inline void writeTriangleMesh2Stl(
        const std::map<std::string, std::vector<TriangleMesh>>& subdividedMesh,
        const std::string& filePath
    ) {
    // 打开输出文件
    std::ofstream stlFile(filePath);
    if (!stlFile.is_open()) {
        throw std::runtime_error("无法打开输出文件: " + filePath);
    }

    // 遍历每个solid
    for (const auto& solidEntry : subdividedMesh) {
        const std::string& solidName = solidEntry.first;
        const std::vector<TriangleMesh>& meshes = solidEntry.second;

        // 写入solid起始标记
        stlFile << "solid " << solidName << "\n";

        // 遍历当前solid下的所有细分网格
        for (const TriangleMesh& mesh : meshes) {
            // 遍历网格中的每个三角面
            for (size_t i = 0; i < mesh.faces.size(); ++i) {
                const auto& face = mesh.faces[i];    // 顶点索引
                const Point3D& normal = mesh.normals[i];  // 法向量（来自原始输入）

                // 写入法向量
                stlFile << "  facet normal "
                        << normal.x << " "
                        << normal.y << " "
                        << normal.z << "\n";

                // 写入顶点环
                stlFile << "    outer loop\n";
                for (size_t vertIdx : face) {
                    const Point3D& vert = mesh.vertices[vertIdx];
                    stlFile << "      vertex "
                            << vert.x << " "
                            << vert.y << " "
                            << vert.z << "\n";
                }
                stlFile << "    endloop\n";

                // 结束当前三角面
                stlFile << "  endfacet\n";
            }
        }

        // 写入solid结束标记
        stlFile << "endsolid " << solidName << "\n";
    }

    std::cout << "已成功导出细分后的STL文件: " << filePath << std::endl;
}



}




#endif //TRIANGLEMESH_H

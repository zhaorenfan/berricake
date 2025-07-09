//
// Created by zhou on 25-6-29.
//

#ifndef SURFACEGROUP_H
#define SURFACEGROUP_H
#include <string>
#include <vector>

#include "TriangleMesh.h"
#include "Triangle.h"
#include "ShadowAnalyzer.h"

namespace geom {
    class SurfaceGroup {
    private:
        std::string group_name;
        StlModel model;  //stl模型
        std::map<std::string, std::vector<TriangleMesh>> meshMap; //划分网格
        std::vector<std::string> surf_names; //需要分析的表面名
        std::map<std::string, double> areas; //面积
        std::map<std::string, Point3D> normals; //平均法向量

        ShadowAnalyzer analyzer;
    public:
        SurfaceGroup(const std::string &name, const std::string &path, const std::vector<std::string>& surf_names)
            :group_name(name), surf_names(surf_names) {

            // 1. 解析STL模型（作为遮挡物）
            std::cout << "正在解析STL文件: "<<path<<std::endl;
            model = parseStlFile(path);
            std::cout << "解析完成，模型包含 " << model.size() << " 个三角面\n";

            // 2. 细分模型（需要分析的网格）
            std::cout << "正在细分模型...\n";
            meshMap = subdivideModel(model, surf_names, 0.1, 5);

            analyzer.batchCalculate(
                model,
                meshMap,
                0.0, 90.0, 15.0,    // 高度角：起始、结束、步长
                0.0, 360.0, 30.0,   // 方位角：起始、结束、步长
                group_name+"_shd.csv"  // 输出文件名
            );

            //面积统计
            std::cout << "表面面积统计...\n";
            for (const auto& [key, value] : model) {
                if (std::ranges::find(surf_names, key)==surf_names.end()) {
                    continue;
                }
                double area = 0.0;
                Point3D normal;

                for (const auto &tri : value) {
                    const double area_tri = triangle_area(tri.vertices[0], tri.vertices[1], tri.vertices[2]);
                    area += area_tri;
                    normal=normal+tri.normal*area_tri;
                }
                std::cout<<"Area["<<key<< "]= "<<area<<std::endl;
                areas[key] = area;
                normals[key] = normal.normalize();
            }


        };

    public:
        double get_shadow_value(double altitude, double azimuth, const std::string& surface) {
            return analyzer.get_shadow_value(altitude, azimuth, surface);
        }
        double get_area(const std::string& surface) {
            return areas[surface];
        }

        std::vector<TriangleMesh> getSurface(const std::string& surf_name) {
            return meshMap[surf_name];
        }

        Point3D getAveNormal(const std::string& surf_name) {
            return normals[surf_name];
        }



    };
}





#endif //SURFACEGROUP_H

//
// Created by zhou on 25-6-29.
//

#ifndef SHADOWANALYZER_H
#define SHADOWANALYZER_H
#include <iomanip>
#include <ranges>
#include <vector>

#include "Point3D.h"
#include "Triangle.h"
#include "TriangleMesh.h"


namespace geom {


// 工具函数：太阳方向向量计算（弧度转换+归一化）
inline Point3D sun_angle_to_direction(double altitude_deg, double azimuth_deg) {
    const double alt_rad = altitude_deg * M_PI / 180.0;
    const double azi_rad = azimuth_deg * M_PI / 180.0;

    const double cos_alt = std::cos(alt_rad);
    const double sin_alt = std::sin(alt_rad);
    const double cos_azi = std::cos(azi_rad);
    const double sin_azi = std::sin(azi_rad);

    // 计算太阳方向向量（单位向量）
    Point3D dir(
        sin_azi * cos_alt,
        cos_azi * cos_alt,
        sin_alt
    );
    return dir.normalize();
}

// 射线-三角形相交检测（Möller-Trumbore算法）
inline bool ray_triangle_intersection(
    const Point3D& ray_origin,
    const Point3D& ray_dir,  // 单位向量
    const Triangle& triangle,
    double& t,               // 射线起点到交点的距离
    const double epsilon = 1e-8  // 精度阈值
) {
    const Point3D& v0 = triangle.vertices[0];
    const Point3D& v1 = triangle.vertices[1];
    const Point3D& v2 = triangle.vertices[2];

    const Point3D edge1 = v1 - v0;
    const Point3D edge2 = v2 - v0;
    const Point3D h = ray_dir.cross(edge2);
    const double a = edge1.dot(h);

    // 射线与三角形平行或共面
    if (std::fabs(a) < epsilon) return false;

    const double f = 1.0 / a;
    const Point3D s = ray_origin - v0;
    const double u = f * s.dot(h);

    // u不在[0,1]范围内，无交点
    if (u < -epsilon || u > 1.0 + epsilon) return false;

    const Point3D q = s.cross(edge1);
    const double v = f * ray_dir.dot(q);

    // v不在[0,1]或u+v>1，无交点
    if (v < -epsilon || u + v > 1.0 + epsilon) return false;

    // 计算交点距离
    t = f * edge2.dot(q);
    return t > epsilon;  // 交点在射线前方
}

// 收集遮挡物并设置ID（确保每个三角形的id正确对应其所属表面）
inline std::vector<std::shared_ptr<Triangle>> collect_occluders(const StlModel& model) {
    std::vector<std::shared_ptr<Triangle>> occluders;

    for (const auto& [solidName, triangles] : model) {
        for (const auto& tri : triangles) {
            // 复制三角形并设置其所属表面ID
            auto occluder = std::make_shared<Triangle>(tri);

            occluders.push_back(occluder);
        }

    }
    return occluders;
}

// 判断单个三角面是否被遮挡（核心逻辑）
inline bool is_face_occluded(
    const std::vector<std::shared_ptr<Triangle>>& occluders,
    const TriangleMesh& mesh,
    size_t face_idx,
    const Point3D& sun_dir
) {

    // 检查面索引有效性
    if (face_idx >= mesh.faces.size() || face_idx >= mesh.centers.size() || face_idx >= mesh.normals.size()) {
        throw std::out_of_range("无效的面索引: " + std::to_string(face_idx));
    }

    // 当前网格所属表面ID（用于自遮挡判断）
    const int current_parent_id = mesh.parent_id;
    const Point3D center = mesh.centers[face_idx];
    const Point3D face_normal = mesh.normals[face_idx].normalize();

    // 背向太阳的面直接判定为遮挡（法向量与太阳方向夹角≥90度）
    if (face_normal.dot(sun_dir) <= 1e-8) {
        return true;
    }

    // 射线方向：与太阳方向相同
    const Point3D ray_dir = sun_dir;
    // 射线起点偏移：沿法向量方向微小偏移，避免与自身面相交
    const double bias = 1e-5;
    const Point3D ray_origin = center + face_normal * bias;


    // 遍历所有遮挡物，检测是否有有效遮挡
    for (const auto& occluder_ptr : occluders) {
        if (!occluder_ptr) continue;  // 跳过空指针

        const auto& occluder = *occluder_ptr;
        //std::cout<<occluder_ptr->id<<" "<<current_parent_id<<std::endl;
        // 自遮挡判断：遮挡物与当前面属于同一表面，直接跳过
        if (occluder.id == current_parent_id) {
            //std::cout<<"检查到自身表面"<<occluder_ptr->id<<std::endl;
            continue;
        }

        // 检测射线是否与遮挡物相交
        double t;
        if (ray_triangle_intersection(ray_origin, ray_dir, occluder, t)) {
            return true;  // 存在有效遮挡
        }
    }

    return false;  // 无遮挡
}

// 阴影分析器类
class ShadowAnalyzer {
private:
    //Point3D sun_dir_;  // 太阳方向向量（单位向量）
    std::vector<std::shared_ptr<Triangle>> occluders_;  // 所有遮挡物
    std::map<std::string, std::vector<std::vector<bool>>> results_;  // 遮挡结果：solid名→网格→面是否被遮挡
    std::map<std::string, std::vector<std::vector<double>>> results_cos;  // 遮挡结果：余弦值
    std::shared_ptr<const std::map<std::string, std::vector<TriangleMesh>>> meshMap_ptr_;  // 分析的网格数据

    // 定义查询键：(高度角, 方位角, 表面名)
    using Key = std::tuple<double, double, std::string>;
    // 全局映射表，存储从CSV读取的数据
    std::map<Key, double> shadow_table;

    // 计算单个solid的面积加权遮挡率（内部使用）
    double calculateSolidOcclusionRate(
        const std::vector<std::vector<bool>>& solid_results,
        const std::vector<TriangleMesh>& meshes
    ) const {
        double total_area = 0.0;
        double occluded_area = 0.0;

        for (size_t mesh_idx = 0; mesh_idx < solid_results.size(); ++mesh_idx) {
            if (mesh_idx >= meshes.size()) break;  // 避免越界

            const auto& mesh_results = solid_results[mesh_idx];
            const auto& mesh = meshes[mesh_idx];

            for (size_t face_idx = 0; face_idx < mesh_results.size(); ++face_idx) {
                if (face_idx >= mesh.areas.size()) break;  // 避免越界

                const double face_area = mesh.areas[face_idx];
                total_area += face_area;
                if (mesh_results[face_idx]) {
                    occluded_area += face_area;
                }
            }
        }

        return (total_area > 1e-9) ? (occluded_area / total_area) : 0.0;
    }

    double calculateSolidOcclusionCosAve(
        const std::vector<std::vector<bool>>& solid_results,
        const std::vector<std::vector<double>>& solid_results_cos,
        const std::vector<TriangleMesh>& meshes
    ) const {
        double total_area = 0.0;
        double occluded_area_cos = 0.0;

        for (size_t mesh_idx = 0; mesh_idx < solid_results_cos.size(); ++mesh_idx) {
            if (mesh_idx >= meshes.size()) break;  // 避免越界

            const auto& mesh_results = solid_results[mesh_idx];
            const auto& mesh_results_cos = solid_results_cos[mesh_idx];
            const auto& mesh = meshes[mesh_idx];

            for (size_t face_idx = 0; face_idx < mesh_results.size(); ++face_idx) {
                if (face_idx >= mesh.areas.size()) break;  // 避免越界

                const double face_area = mesh.areas[face_idx];

                total_area += face_area;
                if (!mesh_results[face_idx]) {
                    occluded_area_cos += face_area*mesh_results_cos[face_idx];
                    //std::cout<<"face_area "<<face_area<<" cos="<<mesh_results_cos[face_idx]<<std::endl;
                }
            }
        }

        return (total_area > 1e-9) ? (occluded_area_cos / total_area) : 0.0;
    }

public:
    // 构造函数：通过太阳高度角和方位角初始化
    ShadowAnalyzer()= default;


    // 加载遮挡物（从STL模型）
    void loadOccluders(const StlModel& model) {
        occluders_ = collect_occluders(model);
    }

    // 分析网格的遮挡情况
    void analyze(const std::map<std::string, std::vector<TriangleMesh>>& meshMap, double sun_altitude, double sun_azimuth) {
        meshMap_ptr_ = std::make_shared<const std::map<std::string, std::vector<TriangleMesh>>>(meshMap);
        results_.clear();

        Point3D sun_dir_=sun_angle_to_direction(sun_altitude, sun_azimuth);


        for (const auto& [solidName, meshes] : meshMap) {
            std::vector<std::vector<bool>> solid_results;
            solid_results.reserve(meshes.size());

            std::vector<std::vector<double>> solid_results_cos;
            solid_results_cos.reserve(meshes.size());

            for (const auto& mesh : meshes) {
                std::vector<bool> mesh_results(mesh.faces.size(), false);

                std::vector<double> mesh_results_cos(mesh.faces.size(), 0.0);

                for (size_t face_idx = 0; face_idx < mesh.faces.size(); ++face_idx) {
                    try {
                        mesh_results[face_idx] = is_face_occluded(occluders_, mesh, face_idx, sun_dir_);
                        auto normal = mesh.normals[face_idx].normalize();
                        if (!mesh_results[face_idx]) {
                            double cos_val = normal.dot(sun_dir_);
                            mesh_results_cos[face_idx]=cos_val;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "警告：处理面 " << face_idx << " 时出错: " << e.what() << "，默认标记为未遮挡\n";
                        mesh_results[face_idx] = false;
                    }

                    // 进度提示（每100个面刷新一次，避免输出过于频繁）
                    // if (face_idx % 100 == 0) {
                    //     std::cout << "处理 " << solidName << " 网格，面 " << face_idx
                    //               << "/" << mesh.faces.size() << "\r" << std::flush;
                    // }
                }
                // std::cout << "处理 " << solidName << " 网格，面 " << mesh.faces.size()
                //           << "/" << mesh.faces.size() << " 完成          \n";

                solid_results.push_back(mesh_results);
                solid_results_cos.push_back(mesh_results_cos);
            }

            results_[solidName] = solid_results;
            results_cos[solidName] = solid_results_cos;
            //std::cout << solidName << " 处理完成，共 " << solid_results.size() << " 个网格\n";
        }
    }

    // 打印面积加权遮挡率结果
    void printResults() const {
        if (!meshMap_ptr_) {
            throw std::runtime_error("未加载网格数据，无法打印结果");
        }

        const auto& meshMap = *meshMap_ptr_;
        std::cout << "\n===== 面积加权遮挡分析结果 =====" << std::endl;

        for (const auto& [solidName, solid_results] : results_) {
            auto mesh_it = meshMap.find(solidName);
            if (mesh_it == meshMap.end()) {
                std::cerr << "警告：未找到 " << solidName << " 对应的网格数据\n";
                continue;
            }
            const auto& meshes = mesh_it->second;

            // 计算总面积和被遮挡面积
            double total_area = 0.0;
            double occluded_area = 0.0;
            for (size_t mesh_idx = 0; mesh_idx < solid_results.size(); ++mesh_idx) {
                if (mesh_idx >= meshes.size()) break;

                const auto& mesh_results = solid_results[mesh_idx];
                const auto& mesh = meshes[mesh_idx];
                for (size_t face_idx = 0; face_idx < mesh_results.size(); ++face_idx) {
                    if (face_idx >= mesh.areas.size()) break;

                    const double face_area = mesh.areas[face_idx];
                    total_area += face_area;
                    if (mesh_results[face_idx]) {
                        occluded_area += face_area;
                    }
                }
            }

            // 计算并打印遮挡率
            const double occlusion_rate = (total_area > 1e-9) ? (occluded_area / total_area) * 100 : 0.0;
            std::cout << std::fixed << std::setprecision(2);
            std::cout << solidName << ":\n";
            std::cout << "  总表面积: " << std::setprecision(4) << total_area << "\n";
            std::cout << "  被遮挡表面积: " << std::setprecision(4) << occluded_area << "\n";
            std::cout << "  面积加权遮挡率: " << std::setprecision(2) << occlusion_rate << "%\n\n";
        }
    }

    // 导出结果到CSV文件
    void exportResults(const std::string& filename) const {
        if (!meshMap_ptr_) {
            throw std::runtime_error("未加载网格数据，无法导出结果");
        }

        const auto& meshMap = *meshMap_ptr_;
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开文件: " + filename);
        }

        // 写入CSV表头
        file << "solid名称,总表面积,被遮挡表面积,面积加权遮挡率(%)\n";
        std::cout << std::fixed << std::setprecision(4);

        for (const auto& [solidName, solid_results] : results_) {
            auto mesh_it = meshMap.find(solidName);
            if (mesh_it == meshMap.end()) continue;
            const auto& meshes = mesh_it->second;

            // 计算面积数据
            double total_area = 0.0, occluded_area = 0.0;
            for (size_t mesh_idx = 0; mesh_idx < solid_results.size(); ++mesh_idx) {
                if (mesh_idx >= meshes.size()) break;

                const auto& mesh_results = solid_results[mesh_idx];
                const auto& mesh = meshes[mesh_idx];
                for (size_t face_idx = 0; face_idx < mesh_results.size(); ++face_idx) {
                    if (face_idx >= mesh.areas.size()) break;
                    const double face_area = mesh.areas[face_idx];
                    total_area += face_area;
                    if (mesh_results[face_idx]) occluded_area += face_area;
                }
            }

            // 写入CSV行
            const double rate = (total_area > 1e-9) ? (occluded_area / total_area) * 100 : 0.0;
            file << solidName << ","
                 << std::setprecision(4) << total_area << ","
                 << std::setprecision(4) << occluded_area << ","
                 << std::setprecision(2) << rate << "\n";


        }

        std::cout << "结果已导出至: " << filename << std::endl;
    }

    // 批量计算不同太阳角度的遮挡率并导出CSV
    void batchCalculate(
        const StlModel& model,
        const std::map<std::string, std::vector<TriangleMesh>>& meshMap,
        double start_altitude, double end_altitude, double step_altitude,
        double start_azimuth, double end_azimuth, double step_azimuth,
        const std::string& output_file
    ) {
        // 验证输入参数有效性
        if (step_altitude <= 0 || step_azimuth <= 0 || start_altitude > end_altitude || start_azimuth > end_azimuth) {
            throw std::invalid_argument("无效的角度范围或步长");
        }

        // 收集所有表面名称（用于CSV表头）
        std::vector<std::string> solid_names;
        for (const auto& [name, _] : meshMap) {
            solid_names.push_back(name);
        }

        // 打开输出文件
        std::ofstream csv_file(output_file);
        if (!csv_file.is_open()) {
            throw std::runtime_error("无法打开输出文件: " + output_file);
        }

        // 写入CSV表头
        csv_file << "高度角,方位角";
        for (const auto& name : solid_names) {
            csv_file << "," << name;
        }
        csv_file << "\n";

        // 预收集遮挡物（避免重复计算）
        const auto occluders = collect_occluders(model);

        // 遍历所有角度组合
        for (double altitude = start_altitude; altitude <= end_altitude + 1e-6; altitude += step_altitude) {
            for (double azimuth = start_azimuth; azimuth <= end_azimuth + 1e-6; azimuth += step_azimuth) {
                // 创建分析器并计算当前角度的遮挡率

                occluders_ = occluders;  // 复用预收集的遮挡物
                analyze(meshMap, altitude, azimuth);

                // 获取各表面的遮挡率（比例，非百分比）
                // const auto rates = getSolidOcclusionRates();

                const auto rates = getSolidOcclusionCos();


                // 写入CSV行
                csv_file << std::fixed << std::setprecision(1) << altitude << "," << azimuth;
                for (const auto& name : solid_names) {
                    const auto it = rates.find(name);
                    const double rate = (it != rates.end()) ? it->second : 0.0;
                    csv_file << "," << std::setprecision(4) << rate;

                    //记录表格
                    Key key = std::make_tuple(altitude, azimuth, name);
                    shadow_table[key] = rate;
                }
                csv_file << "\n";




                // 打印进度
                std::cout << "已计算: 高度角=" << std::setprecision(1) << altitude
                          << "°, 方位角=" << std::setprecision(1) << azimuth << "°\r" << std::flush;
            }
        }

        std::cout << "\n批量计算完成，结果已导出至: " << output_file << std::endl;
    }

    static double angle_distance(double alt1, double azi1, double alt2, double azi2) {
        // 高度角差值（非周期性）
        double d_alt = alt1 - alt2;

        // 方位角差值（考虑周期性，取最小差值）
        double d_azi = std::fabs(azi1 - azi2);
        d_azi = std::min(d_azi, 360.0 - d_azi);  // 如350°与10°的差值为20°

        // 欧氏距离（高度角和方位角权重相同）
        return std::sqrt(d_alt * d_alt + d_azi * d_azi);
    }

    //双线性差值
    // 计算两个角度对之间的球面距离（考虑方位角的周期性）
    double angular_distance(double alt1, double azi1, double alt2, double azi2) {
        // 处理方位角的周期性（确保差值在-180°到180°之间）
        double d_azi = std::fmod(azi2 - azi1 + 540.0, 360.0) - 180.0;
        double d_alt = alt2 - alt1;

        // 使用球面余弦定律简化版（适用于小角度）
        // 对于大角度，建议使用完整的球面距离公式
        return std::sqrt(d_alt * d_alt + d_azi * d_azi * std::cos((alt1 + alt2) * M_PI / 360.0));
    }

    // 查找最近的4个点并进行双线性插值
    double find_quad_interpolation(double target_alt, double target_azi, const std::string& surface) {
        // 收集该表面名下的所有角度对和对应值
        std::vector<std::tuple<double, double, double, double>> candidates;  // (alt, azi, value, distance)
        for (const auto& [key, value] : shadow_table) {
            auto [alt, azi, s] = key;
            if (s == surface) {
                double dist = angular_distance(target_alt, target_azi, alt, azi);
                candidates.emplace_back(alt, azi, value, dist);
            }
        }

        // 若数据不足，抛出异常
        if (candidates.size() < 4) {
            throw std::out_of_range("表面 " + surface + " 的数据点不足(需要至少4个)");
        }

        // 按距离升序排序
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) { return std::get<3>(a) < std::get<3>(b); });

        // 取最近的4个点
        std::vector<std::tuple<double, double, double>> nearest;  // (alt, azi, value)
        for (int i = 0; i < 4; ++i) {
            nearest.emplace_back(
                std::get<0>(candidates[i]),
                std::get<1>(candidates[i]),
                std::get<2>(candidates[i])
            );
        }

        // 检查这4个点是否能形成一个合理的四边形（简化版）
        // 这里仅检查是否有两个不同的高度角和方位角
        std::vector<double> alts = {std::get<0>(nearest[0]), std::get<0>(nearest[1]),
                                   std::get<0>(nearest[2]), std::get<0>(nearest[3])};
        std::vector<double> azis = {std::get<1>(nearest[0]), std::get<1>(nearest[1]),
                                   std::get<1>(nearest[2]), std::get<1>(nearest[3])};

        // 排序后检查唯一值数量
        std::sort(alts.begin(), alts.end());
        std::sort(azis.begin(), azis.end());
        size_t unique_alts = std::unique(alts.begin(), alts.end()) - alts.begin();
        size_t unique_azis = std::unique(azis.begin(), azis.end()) - azis.begin();

        if (unique_alts < 2 || unique_azis < 2) {
            // 无法形成四边形，退化为反距离权重插值
            double sum_weights = 0.0;
            double weighted_value = 0.0;

            for (const auto& [alt, azi, value] : nearest) {
                double dist = angular_distance(target_alt, target_azi, alt, azi);
                double weight = 1.0 / (dist * dist + 1e-9);  // 反距离平方权重

                sum_weights += weight;
                weighted_value += weight * value;
            }

            return weighted_value / sum_weights;
        }

        // 双线性插值（假设4个点能形成矩形网格）
        // 1. 找到最接近的两个高度角和方位角
        double min_alt = std::min(alts[0], alts[1]);
        double max_alt = std::max(alts[2], alts[3]);
        double min_azi = std::min(azis[0], azis[1]);
        double max_azi = std::max(azis[2], azis[3]);

        // 2. 将目标点归一化到[0,1]范围
        double u = (target_alt - min_alt) / (max_alt - min_alt);
        double v = (target_azi - min_azi) / (max_azi - min_azi);

        // 3. 确保归一化值在有效范围内
        u = std::clamp(u, 0.0, 1.0);
        v = std::clamp(v, 0.0, 1.0);

        // 4. 查找4个角点的值（简化版，实际应按网格位置匹配）
        double q11 = 0.0, q12 = 0.0, q21 = 0.0, q22 = 0.0;

        // 这里简化处理，假设已按高度角和方位角排序
        // 实际应用中可能需要更复杂的匹配逻辑
        q11 = std::get<2>(nearest[0]);  // 左下角
        q12 = std::get<2>(nearest[1]);  // 左上角
        q21 = std::get<2>(nearest[2]);  // 右下角
        q22 = std::get<2>(nearest[3]);  // 右上角

        // 5. 执行双线性插值
        double result = (1 - u) * (1 - v) * q11 +
                        u * (1 - v) * q21 +
                        (1 - u) * v * q12 +
                        u * v * q22;

        return result;
    }

    double find_closest_value(double target_alt, double target_azi, const std::string& surface) {
        // 收集该表面名下的所有角度对和对应值
        std::vector<std::tuple<double, double, double>> candidates;  // (alt, azi, value)
        for (const auto& [key, value] : shadow_table) {
            auto [alt, azi, s] = key;
            if (s == surface) {
                candidates.emplace_back(alt, azi, value);  // 假设shadow_table的值现在是double
            }
        }

        // 若该表面无任何数据，抛出异常
        if (candidates.empty()) {
            throw std::out_of_range("无表面 " + surface + " 的任何数据");
        }

        // 查找距离最小的候选者
        double min_distance = std::numeric_limits<double>::max();
        double best_value = -1.0;  // 改为double类型

        for (const auto& [alt, azi, value] : candidates) {
            double dist = angle_distance(target_alt, target_azi, alt, azi);
            if (dist < min_distance) {
                min_distance = dist;
                best_value = value;
            }
        }

        // 输出查找结果（便于调试）
        // std::cout << "警告：未找到精确匹配，使用最相近数据（距离=" << min_distance << "°）\n";
        return best_value;
    }

    double get_shadow_value(double altitude, double azimuth, const std::string& surface) {
        //std::cout<<altitude<< azimuth<<surface<<std::endl;
        // 处理方位角360°等效于0°
        if (std::abs(azimuth - 360.0) < 1e-6) {
            azimuth = 0.0;
        }

        if (altitude<=0) {
            return 0.0;
        }

        Key key = std::make_tuple(altitude, azimuth, surface);
        auto it = shadow_table.find(key);

        if (it != shadow_table.end()) {
            return it->second;
        }
        // 4. 精确匹配不存在，查找最相近的值
        //return find_closest_value(altitude, azimuth, surface);
        return find_quad_interpolation(altitude, azimuth, surface);
    }

private:
    // 获取各表面的遮挡率（返回比例，用于批量计算）
    std::map<std::string, double> getSolidOcclusionRates() const {
        std::map<std::string, double> rates;
        if (!meshMap_ptr_) return rates;

        const auto& meshMap = *meshMap_ptr_;
        for (const auto& [solidName, solid_results] : results_) {
            auto mesh_it = meshMap.find(solidName);
            if (mesh_it == meshMap.end()) {
                rates[solidName] = 0.0;
                continue;
            }
            rates[solidName] = calculateSolidOcclusionRate(solid_results, mesh_it->second);
        }
        return rates;
    }

    std::map<std::string, double> getSolidOcclusionCos() const {
        std::map<std::string, double> cos_values;
        if (!meshMap_ptr_) return cos_values;

        const auto& meshMap = *meshMap_ptr_;
        for (const auto& [solidName, solid_results] : results_cos) {
            auto mesh_it = meshMap.find(solidName);
            if (mesh_it == meshMap.end()) {
                cos_values[solidName] = 0.0;
                continue;
            }

            cos_values[solidName] = calculateSolidOcclusionCosAve(results_.at(solidName),solid_results,  mesh_it->second);
        }
        return cos_values;
    }
};

} // namespace geom



#endif //SHADOWANALYZER_H

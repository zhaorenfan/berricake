//
// Created by zhou on 25-7-1.
//

#ifndef RADIATION_H
#define RADIATION_H
#include <cmath>
#include <vector>

namespace util {

class Radiation {
public:
    // 计算建筑表面接收的地面反射辐射量
    static std::tuple<double, double, double, double> calculateTotalRadiationOnSurface(
        double directRadiation,             // 太阳直接辐射 (W/m²)
        double diffuseRadiation,            // 水平面天空散射辐射 (W/m²)
        const std::vector<double>& normal,  // 表面法向量 [nx, ny, nz]
        double cosIncidence,            // 余弦角
        double solarElevationAngle,        // 高度角
        double groundReflectance = 0.2,     // 地面反射率 (0-1)
        bool useAnisotropy = true)          // 是否考虑散射辐射各向异性
    {
        // 确保法向量是归一化的
        double norm = std::sqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
        std::vector<double> unitNormal = {normal[0]/norm, normal[1]/norm, normal[2]/norm};

        double solarZenithAngle = M_PI_2 - solarElevationAngle;  // 天顶角 = π/2 - 高度角

        // === 1. 计算直接辐射分量 ===
        double directComponent = directRadiation * cosIncidence;

        // === 2. 计算天空散射辐射分量 ===
        double diffuseComponent = 0.0;

        // 计算法向量与天顶方向的夹角余弦
        double cosZenithAngle = std::max(0.0, unitNormal[2]);

        // 各向同性模型
        double isotropicDiffuse = diffuseRadiation * cosZenithAngle;

        if (useAnisotropy) {
            // 考虑各向异性的天空散射辐射模型 (简化的Hay模型)
            double anisotropyFactor = 0.15 * std::cos(solarZenithAngle);
            double anisotropicDiffuse = anisotropyFactor * diffuseRadiation * cosIncidence;

            // 总散射辐射 = 各向同性分量 + 各向异性分量
            diffuseComponent = isotropicDiffuse + anisotropicDiffuse;
        } else {
            // 仅使用各向同性模型
            diffuseComponent = isotropicDiffuse;
        }

        // === 3. 计算地面反射辐射分量 ===
        double groundReflectedComponent = 0.5 * groundReflectance *
                                         (directRadiation + diffuseRadiation) *
                                         (1.0 - cosZenithAngle);

        // === 4. 计算总辐射 ===
        double totalRad = directComponent + diffuseComponent + groundReflectedComponent;

        return std::make_tuple(totalRad, directComponent, diffuseComponent, groundReflectedComponent);
    }
};

} // util

#endif //RADIATION_H

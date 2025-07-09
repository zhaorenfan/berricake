//
// Created by zhou on 25-6-27.
//

#ifndef POINT3D_H
#define POINT3D_H
#include <cmath>

namespace geom {
    struct Point3D {
        double x, y, z;
        explicit Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

        Point3D operator+(const Point3D& other) const {
            return Point3D(x + other.x, y + other.y, z + other.z);
        }

        Point3D operator-(const Point3D& other) const {
            return Point3D(x - other.x, y - other.y, z - other.z);
        }

        // 新增：一元负号运算符（用于单个点取反方向）
        Point3D operator-() const {
            return Point3D(-x, -y, -z);  // 每个分量取相反数
        }

        // 标量乘法
        Point3D operator*(double s) const {
            return Point3D(x * s, y * s, z * s);
        }

        Point3D operator/(double scalar) const {
            return Point3D(x / scalar, y / scalar, z / scalar);
        }

        // 向量长度
        double magnitude() const {
            return std::sqrt(x * x + y * y + z * z);
        }

        // 归一化
        Point3D normalize() const {
            double mag = magnitude();
            if (mag < 1e-9) return Point3D();
            return Point3D(x / mag, y / mag, z / mag);
        }

        // 点积
        double dot(const Point3D& p) const {
            return x * p.x + y * p.y + z * p.z;
        }

        // 叉积
        Point3D cross(const Point3D& p) const {
            return Point3D(
                y * p.z - z * p.y,
                z * p.x - x * p.z,
                x * p.y - y * p.x
            );
        }


    };

    // 计算叉积
    inline Point3D cross(const Point3D& a, const Point3D& b) {
        return Point3D(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }


}





#endif //POINT3D_H

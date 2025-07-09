//
// Created by zhou on 25-7-1.
//

#ifndef SUNPOSITION_H
#define SUNPOSITION_H

#include <cmath>
#include <tuple>

namespace util {

class SunPosition {
public:
// 计算儒略日
static double julian_day(int year, int month, int day, int hour, int minute, double second) {
    int a, b;
    if (month < 3) {
        month += 12;
        year -= 1;
    }
    a = year / 100;
    b = 2 - a + a / 4;
    double jdn = int(365.25 * (year + 4716)) + int(30.6001 * (month + 1)) + day + b - 1524.5;
    double jd = jdn + (hour + minute / 60.0 + second / 3600.0) / 24.0;
    return jd;
}

// 计算太阳位置(高度角和方位角)
static std::tuple<double, double> calculate_sun_position(double longitude, double latitude, int timezone,
                                                         int year, int month, int day, int hour, int minute, double second) {
    double jd = julian_day(year, month, day, hour, minute, second);
    double n = jd - 2451545.0;

    // 计算太阳的平黄经
    double L = 280.460 + 0.9856474 * n;
    L = std::fmod(L, 360);
    if (L < 0) {
        L = L + 360;
    }

    // 计算太阳的平近点角
    double g = 357.528 + 0.9856003 * n;
    g = std::fmod(g, 360);
    if (g < 0) {
        g = g + 360;
    }

    // 计算太阳的真黄经
    double lambda_sun = L + 1.915 * std::sin(g * M_PI / 180) + 0.020 * std::sin(2 * g * M_PI / 180);
    lambda_sun = std::fmod(lambda_sun, 360);
    if (lambda_sun < 0) {
        lambda_sun = lambda_sun + 360;
    }

    // 计算太阳的赤纬
    double epsilon = 23.439 - 0.0000004 * n;
    double delta = std::asin(std::sin(epsilon * M_PI / 180) * std::sin(lambda_sun * M_PI / 180)) * 180 / M_PI;

    // 计算时角
    double t = (hour + minute / 60.0 + second / 3600.0) + longitude / 15 - timezone;
    if (t > 24) {
        t = t - 24;
    } else if (t < 0) {
        t = t + 24;
    }
    double h = 15 * (t - 12);

    // 计算太阳高度角
    double solar_altitude = std::asin(std::sin(latitude * M_PI / 180) * std::sin(delta * M_PI / 180) +
                                     std::cos(latitude * M_PI / 180) * std::cos(delta * M_PI / 180) *
                                     std::cos(h * M_PI / 180)) * 180 / M_PI;

    // 计算太阳方位角
    double azimuth_numerator = std::sin(h * M_PI / 180);
    double azimuth_denominator = (std::cos(h * M_PI / 180) * std::sin(latitude * M_PI / 180) -
                                 std::tan(delta * M_PI / 180) * std::cos(latitude * M_PI / 180));
    double azimuth = std::atan2(azimuth_numerator, azimuth_denominator) * 180 / M_PI;
    if (azimuth < 0) {
        azimuth = azimuth + 360;
    }

    // 调整方位角以符合 0 为北，90 为东，180 为南，270 为西
    azimuth = std::fmod(azimuth + 180, 360);

    return std::make_tuple(solar_altitude, azimuth);
}
};

} // util

#endif //SUNPOSITION_H

//
// Created by zhou on 25-6-30.
//

#ifndef SITE_H
#define SITE_H
#include <string>

namespace core {

struct  Site {
    std::string name;
    double timeZone;
    double latitude;            // 纬度{deg}
    double longitude;           // 经度 {deg}
    double elevation;           // 海拔 {m}
};

} // core

#endif //SITE_H

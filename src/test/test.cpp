#include <iostream>
#include <SurfaceGroup.h>

using namespace geom;



int main() {
    try {
        //需要分析的表面名
        const std::vector<std::string> surf_names = {
            "top", "front"
        };
        //创建表面组实例
        SurfaceGroup group("surfs", "final_combined.stl", surf_names);

        //测试值
        double val = group.get_shadow_value(45, 100, "front");
        std::cout<<"计算值: " << val << std::endl;


    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
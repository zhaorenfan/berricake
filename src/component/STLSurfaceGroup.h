//
// Created by zhou on 25-6-30.
//

#ifndef STLSURFACEGROUP_H
#define STLSURFACEGROUP_H
#include <BaseComponent.h>
#include <SurfaceGroup.h>

using namespace geom;

namespace comp {
    class STLSurfaceGroup: public core::BaseComponent{
    private:
        std::shared_ptr<BaseComponent> weather;

        std::string path;
        std::shared_ptr<SurfaceGroup> surface_group;
        std::vector<std::string> surface_names;

        bool flag=false;

        double longitude; //经度
        double latitude; //纬度
        int timeZone;

    public:
        void awake() override;
        void before(const core::SimTime &time) override;
        void after(const core::SimTime& time) override;
        void parse(const json &in_params) override;
    };
}




#endif //STLSURFACEGROUP_H

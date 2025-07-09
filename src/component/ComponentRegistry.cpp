//
// Created by zhou on 25-6-21.
//

#include "ComponentRegistry.h"

#include "ComponentFactory.h"
#include "EPWReader.h"
#include "Output.h"
#include "WindModule.h"
#include "STLSurfaceGroup.h"

using namespace comp;

void ComponentRegistry::registerAllComponents() {
    core::ComponentFactory::registerClass("WindModule",[]() {return std::make_shared<WindModule>();});
    core::ComponentFactory::registerClass("EPWReader",[]() {return std::make_shared<EPWReader>();});
    core::ComponentFactory::registerClass("Output",[]() {return std::make_shared<Output>();});
    core::ComponentFactory::registerClass("STLSurfaceGroup",[]() {return std::make_shared<STLSurfaceGroup>();});
    // 其他组件...
}

add_library(core SHARED)


target_link_libraries(core
        util
        nlohmann_json
)

target_sources(core
PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Site.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SimTime.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/BaseComponent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ComponentFactory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Link.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SystemStateHub.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SimManager.cpp
)

target_include_directories(core
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

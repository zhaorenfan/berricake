add_library(geometry SHARED)


target_link_libraries(geometry

)

target_sources(geometry
PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Point3D.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TriangleMesh.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ShadowAnalyzer.cpp

)

target_include_directories(geometry
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

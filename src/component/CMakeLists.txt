
add_library(component SHARED)

target_link_libraries(component
        util
        core
        geometry
)

target_sources(component
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/EPWReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WindModule.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Output.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/STLSurfaceGroup.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/ComponentRegistry.cpp

)

target_include_directories(component
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)


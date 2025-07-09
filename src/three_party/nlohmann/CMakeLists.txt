
add_library(nlohmann_json SHARED)


target_sources(nlohmann_json
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/json.cpp

)

target_include_directories(nlohmann_json
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)


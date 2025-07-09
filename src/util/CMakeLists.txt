
add_library(util SHARED)

target_link_libraries(util
        nlohmann_json
)

target_sources(util
        PRIVATE
        # ${CMAKE_CURRENT_SOURCE_DIR}/Logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SunPosition.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Radiation.cpp
)

target_include_directories(util
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
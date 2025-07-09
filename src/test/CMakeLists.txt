add_executable(berricake)

target_sources(berricake
PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/berricake.cpp
)

target_link_libraries(berricake
PRIVATE
    core component
)

# test
add_executable(test)

target_sources(test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
)

target_link_libraries(test
        PRIVATE
        geometry
)



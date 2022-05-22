cmake_minimum_required(VERSION 3.11.0)
project(vulkan_basic VERSION 0.1.0)

include(CTest)
enable_testing()

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

file(GLOB_RECURSE SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file(GLOB_RECURSE IMGUI_FILE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/*)

add_executable(vulkan_basic 
    ${SRC_FILE} 
    ${IMGUI_FILE}
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)

target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include/imgui-1.87/ /usr/local/include/imgui-1.87/backends/)

target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

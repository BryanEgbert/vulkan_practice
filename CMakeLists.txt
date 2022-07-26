cmake_minimum_required(VERSION 3.11.0)
project(vulkan_basic VERSION 0.1.0)

include(CTest)
enable_testing()

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(Ktx REQUIRED)

file(GLOB_RECURSE SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file(GLOB_RECURSE IMGUI_FILE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/*)
file(GLOB_RECURSE TINY_GLTF ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinygltf/*)
file(GLOB_RECURSE FILE_LOADER ${CMAKE_CURRENT_SOURCE_DIR}/src/file_loader/*)

set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_executable(vulkan_basic 
    ${SRC_FILE} 
    ${IMGUI_FILE}
    ${TINY_GLTF}
    ${FILE_LOADER}
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARY})
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC ktx)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

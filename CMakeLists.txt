cmake_minimum_required(VERSION 3.16)

project(HVAlgo VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加cmake模块路径
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# 添加选项
option(ENABLE_TORCH "Enable PyTorch support for MLP filter" OFF)

# 查找依赖
find_package(MetavisionSDK REQUIRED COMPONENTS base core)
find_package(Eigen3 REQUIRED)

# 可选的PyTorch依赖
if(ENABLE_TORCH)
    find_package(Torch QUIET)
    if(NOT Torch_FOUND)
        message(WARNING "PyTorch not found. MLP filter will be disabled.")
        set(ENABLE_TORCH OFF)
    endif()
endif()

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 收集源文件
file(GLOB_RECURSE DENOISE_SOURCES "src/denoise/*.cpp")
file(GLOB_RECURSE CV_SOURCES "src/cv/*.cpp")
file(GLOB_RECURSE CV3D_SOURCES "src/cv3d/*.cpp")
file(GLOB_RECURSE RESTORATION_SOURCES "src/restoration/*.cpp")

# 如果没有启用torch，则排除multi_layer_perceptron_filter.cpp
if(NOT ENABLE_TORCH)
    list(REMOVE_ITEM DENOISE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/denoise/multi_layer_perceptron_filter.cpp")
endif()

# 创建库
add_library(hv_algo SHARED
    ${DENOISE_SOURCES}
    ${CV_SOURCES}
    ${CV3D_SOURCES}
    ${RESTORATION_SOURCES}
)

# 设置库属性
set(PUBLIC_HEADERS
    "include/denoise/double_window_filter.h"
    "include/denoise/event_flow_filter.h"
    "include/denoise/khodamoradi_denoiser.h"
    "include/denoise/reclusive_event_denoisor.h"
    "include/denoise/timesurface_denoisor.h"
    "include/denoise/yang_noise_filter.h"
)

# 如果启用了torch，则包含MLP filter头文件
if(ENABLE_TORCH)
    list(APPEND PUBLIC_HEADERS "include/denoise/multi_layer_perceptron_filter.h")
endif()

set_target_properties(hv_algo PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

# 链接依赖
target_link_libraries(hv_algo
    PUBLIC
        MetavisionSDK::base
        MetavisionSDK::core
        Eigen3::Eigen
)

# 如果启用了torch，则链接torch库
if(ENABLE_TORCH)
    target_link_libraries(hv_algo PUBLIC ${TORCH_LIBRARIES})
    target_compile_definitions(hv_algo PRIVATE ENABLE_TORCH)
endif()

# 设置包含目录
target_include_directories(hv_algo
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 编译选项
target_compile_options(hv_algo PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -O3>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -O3>
)

# 安装配置
include(GNUInstallDirs)

# 安装库文件
install(TARGETS hv_algo
    EXPORT HVAlgoTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hv_algo/denoise
)

# 安装头文件
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hv_algo
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# 生成并安装cmake配置文件
include(CMakePackageConfigHelpers)

# 生成版本文件
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/HVAlgoConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# 配置config文件
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/HVAlgoConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/HVAlgoConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HVAlgo
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

# 安装cmake配置文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/HVAlgoConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/HVAlgoConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HVAlgo
)

# 安装targets文件
install(EXPORT HVAlgoTargets
    FILE HVAlgoTargets.cmake
    NAMESPACE HVAlgo::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HVAlgo
)

# 生成并安装pkg-config文件
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/hv_algo.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/hv_algo.pc"
    @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/hv_algo.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# 可选：构建示例
option(BUILD_SAMPLES "Build sample applications" OFF)
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

# 可选：构建测试
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    # add_subdirectory(tests)
endif()
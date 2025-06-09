cmake_minimum_required(VERSION 3.10)
project(dwf_denoising VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找HV Toolkit 
find_package(HVToolkit REQUIRED)

# 查找HV Algo库
find_package(HVAlgo REQUIRED)

# 查找Metavision SDK
find_package(MetavisionSDK REQUIRED COMPONENTS core stream)

# 添加可执行文件
add_executable(dwf_denoising dwf_denoising.cpp)

# 包含头文件目录
target_include_directories(dwf_denoising
    PRIVATE
        ${HVAlgo_INCLUDE_DIRS}
        ${MetavisionSDK_INCLUDE_DIRS}
)

# 链接库 
target_link_libraries(dwf_denoising
    PRIVATE
        HVToolkit::hv_event_reader
        HVAlgo::hv_algo
        ${MetavisionSDK_LIBRARIES}
        pthread
)




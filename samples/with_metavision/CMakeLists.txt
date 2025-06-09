cmake_minimum_required(VERSION 3.16)
project(denoising_samples VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖库
find_package(Eigen3 REQUIRED)
find_package(MetavisionSDK COMPONENTS core stream ui REQUIRED)
find_package(HVAlgo REQUIRED)

# dwf_denoising 示例
set(sample dwf_denoising)
add_executable(${sample} ${sample}.cpp)
target_include_directories(${sample}
    PRIVATE
        ${HVAlgo_INCLUDE_DIRS}
        ${MetavisionSDK_INCLUDE_DIRS}
)
target_link_libraries(${sample}
    PRIVATE
        HVAlgo::hv_algo
        MetavisionSDK::core
        MetavisionSDK::stream
        MetavisionSDK::ui
)

# event_flow_denoising 示例
set(sample event_flow_denoising)
add_executable(${sample} ${sample}.cpp)
target_include_directories(${sample}
    PRIVATE
        ${HVAlgo_INCLUDE_DIRS}
        ${MetavisionSDK_INCLUDE_DIRS}
        ${EIGEN3_INCLUDE_DIR}
)
target_link_libraries(${sample}
    PRIVATE
        HVAlgo::hv_algo
        MetavisionSDK::core
        MetavisionSDK::stream
        MetavisionSDK::ui
)

# khodamoradi_denoising 示例
set(sample khodamoradi_denoising)
add_executable(${sample} ${sample}.cpp)
target_include_directories(${sample}
    PRIVATE
        ${HVAlgo_INCLUDE_DIRS}
        ${MetavisionSDK_INCLUDE_DIRS}
)
target_link_libraries(${sample}
    PRIVATE
        HVAlgo::hv_algo
        MetavisionSDK::core
        MetavisionSDK::stream
        MetavisionSDK::ui
)

# re_denoising 示例
set(sample re_denoising)
add_executable(${sample} ${sample}.cpp)
target_include_directories(${sample}
    PRIVATE
        ${HVAlgo_INCLUDE_DIRS}
        ${MetavisionSDK_INCLUDE_DIRS}
)
target_link_libraries(${sample}
    PRIVATE
        HVAlgo::hv_algo
        MetavisionSDK::core
        MetavisionSDK::stream
        MetavisionSDK::ui
)

# ts_denoising 示例
set(sample ts_denoising)
add_executable(${sample} ${sample}.cpp)
target_include_directories(${sample}
    PRIVATE
        ${HVAlgo_INCLUDE_DIRS}
        ${MetavisionSDK_INCLUDE_DIRS}
)
target_link_libraries(${sample}
    PRIVATE
        HVAlgo::hv_algo
        MetavisionSDK::core
        MetavisionSDK::stream
        MetavisionSDK::ui
)

# y_denoising 示例
set(sample y_denoising)
add_executable(${sample} ${sample}.cpp)
target_include_directories(${sample}
    PRIVATE
        ${HVAlgo_INCLUDE_DIRS}
        ${MetavisionSDK_INCLUDE_DIRS}
)
target_link_libraries(${sample}
    PRIVATE
        HVAlgo::hv_algo
        MetavisionSDK::core
        MetavisionSDK::stream
        MetavisionSDK::ui
)

# # mlpf_denoising 示例 (需要PyTorch)
# set(Torch_DIR "/home/taiyangshen/workspace/tools/libtorch/share/cmake/Torch/")
# find_package(Torch REQUIRED)
# set(sample mlpf_denoising)
# add_executable(${sample} ${sample}.cpp)
# target_include_directories(${sample}
#     PRIVATE
#         ${HVAlgo_INCLUDE_DIRS}
#         ${MetavisionSDK_INCLUDE_DIRS}
# )
# target_link_libraries(${sample}
#     PRIVATE
#         HVAlgo::hv_algo
#         MetavisionSDK::core
#         MetavisionSDK::stream
#         MetavisionSDK::ui
#         ${TORCH_LIBRARIES}
# )


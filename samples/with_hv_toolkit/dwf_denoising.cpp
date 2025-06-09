/*
 * Copyright 2025 ShiMetaPi
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "hv_event_reader.h"
#include <hv_algo/denoise/double_window_filter.h>
#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>

using namespace hv;
using namespace Shimeta::Algorithm::Denoise;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <事件文件.raw>" << std::endl;
        return -1;
    }
    std::string input_file = argv[1];
    HVEventReader reader;
    if (!reader.open(input_file)) {
        std::cerr << "无法打开事件文件: " << input_file << std::endl;
        return -1;
    }
    auto size = reader.getImageSize();
    int width = size.first;
    int height = size.second;
    
    // 获取并打印文件头信息
    const auto& header = reader.getHeader();
    auto start_time = header.start_timestamp;
    
    std::cout << "=== RAW文件信息 ===" << std::endl;
    std::cout << "文件路径: " << input_file << std::endl;
    std::cout << "图像尺寸: " << width << "x" << height << std::endl;
    std::cout << "起始时间戳: " << start_time << " μs" << std::endl;
    std::cout << "==================" << std::endl << std::endl;
    
    std::vector<Metavision::EventCD> all_events;
    if (reader.readAllEvents(all_events) == 0) {
        std::cerr << "事件文件为空或读取失败。" << std::endl;
        return -1;
    }
    
    std::cout << "实际读取事件数: " << all_events.size() << std::endl;
    
    // 去噪处理 - 计算处理时间
    std::cout << "\n开始DWF去噪处理..." << std::endl;
    auto dwf_start = std::chrono::high_resolution_clock::now();
    
    DoubleWindowFilter dwf;
    std::vector<Metavision::EventCD> denoised_events = dwf.process_events(all_events);
    
    auto dwf_end = std::chrono::high_resolution_clock::now();
    auto dwf_duration = std::chrono::duration_cast<std::chrono::milliseconds>(dwf_end - dwf_start);
    
    std::cout << "DWF处理完成!" << std::endl;
    std::cout << "处理前事件数: " << all_events.size() << std::endl;
    std::cout << "处理后事件数: " << denoised_events.size() << std::endl;
    std::cout << "去噪比例: " << std::fixed << std::setprecision(2) 
              << (1.0 - (double)denoised_events.size() / all_events.size()) * 100.0 << "%" << std::endl;
    std::cout << "DWF处理用时: " << dwf_duration.count() << " ms" << std::endl;
    std::cout << "处理速度: " << std::fixed << std::setprecision(1) 
              << (double)all_events.size() / dwf_duration.count() * 1000.0 << " events/s" << std::endl << std::endl;
    
    // 帧生成参数
    const std::uint32_t acc = 20000; // 20ms
    const std::uint32_t fps = 50;    // 50 FPS
    
    // 创建单个帧生成器，宽度加倍以容纳原始和去噪数据
    Metavision::PeriodicFrameGenerationAlgorithm frame_gen(width * 2, height, acc, fps);
    
    // 准备合并的事件数据
    std::vector<Metavision::EventCD> combined_events;
    
    // 将原始事件添加到左半部分（x坐标不变）
    for (const auto& event : all_events) {
        combined_events.push_back(event);
    }
    
    // 将去噪事件添加到右半部分（x坐标偏移width）
    for (const auto& event : denoised_events) {
        Metavision::EventCD shifted_event = event;
        shifted_event.x += width; // 偏移到右半部分
        combined_events.push_back(shifted_event);
    }
    
    // 按时间戳排序合并后的事件
    std::sort(combined_events.begin(), combined_events.end(), 
              [](const Metavision::EventCD& a, const Metavision::EventCD& b) {
                  return a.t < b.t;
              });
    
    // 设置回调函数
    cv::Mat display_frame;
    bool has_frame = false;
    
    frame_gen.set_output_callback([&](Metavision::timestamp ts, cv::Mat &output_frame) {
        display_frame = output_frame.clone();
        
        // 添加分割线
        cv::line(display_frame, cv::Point(width, 0), cv::Point(width, height), 
                cv::Scalar(128, 128, 128), 2);
        
        // 添加标题
        cv::putText(display_frame, "Original", cv::Point(10, 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
        cv::putText(display_frame, "Denoised", cv::Point(width + 10, 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
        
        has_frame = true;
    });
    
    // 创建显示窗口
    cv::namedWindow("Original vs Denoised", cv::WINDOW_NORMAL);
    
    std::cout << "开始连续播放... 按 'q' 退出" << std::endl;
    
    // 分批处理事件以实现连续播放
    const size_t batch_size = 20000; // 每批处理的事件数量
    size_t processed = 0;
    
    while (processed < combined_events.size()) {
        // 计算当前批次的结束位置
        size_t end_pos = std::min(processed + batch_size, combined_events.size());
        
        // 处理当前批次的合并事件
        auto begin_it = combined_events.begin() + processed;
        auto end_it = combined_events.begin() + end_pos;
        frame_gen.process_events(begin_it, end_it);
        
        // 显示帧
        if (has_frame) {
            cv::imshow("Original vs Denoised", display_frame);
            has_frame = false;
        }
        
        // 控制播放速度和退出
        int key = cv::waitKey(30); // 约33fps
        if (key == 'q' || key == 27) { // 'q' 或 ESC 退出
            break;
        }
        
        processed = end_pos;
    }
    
    cv::destroyAllWindows();
    std::cout << "播放完成！" << std::endl;
    return 0;
}
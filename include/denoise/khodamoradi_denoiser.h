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
#ifndef SHIMETA_SDK_ALGORITHM_DENOISE_KHODAMORADI_DENOISER_H
#define SHIMETA_SDK_ALGORITHM_DENOISE_KHODAMORADI_DENOISER_H

#include <cstdint>
#include <vector>

#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/base/events/event_cd_vector.h>
#include <metavision/sdk/base/events/event2d.h>

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

/// @brief Khodamoradi noise filter adapted for Metavision CD events.
class KhodamoradiDenoiser {
public:
    /// @brief 构造函数
    /// @param width 传感器宽度
    /// @param height 传感器高度
    /// @param duration 时窗长度（微秒）
    /// @param int_threshold 支持像素阈值
    explicit KhodamoradiDenoiser(
        uint16_t width,
        uint16_t height,
        Metavision::timestamp duration = 2000,
        size_t int_threshold = 2
    );

    /// @brief 初始化滤波器
    void initialize();

    /// @brief 过滤单个事件
    /// @param event 输入事件
    /// @return 是否为信号
    bool filter(const Metavision::EventCD &event);

    /// @brief 批量处理事件
    /// @param events 输入事件向量
    /// @return 保留的事件向量
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);

private:
    uint16_t width_;
    uint16_t height_;
    Metavision::timestamp duration_;
    size_t int_threshold_;
    std::vector<Metavision::EventCD> last_event_x_;
    std::vector<Metavision::EventCD> last_event_y_;

    /// @brief 搜索邻域相关事件
    /// @param event 当前事件
    /// @return 支持像素数
    size_t searchCorrelation(const Metavision::EventCD &event);
};

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta

#endif // SHIMETA_SDK_ALGORITHM_DENOISE_KHODAMORADI_DENOISER_H
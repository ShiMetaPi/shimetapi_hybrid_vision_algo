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
#ifndef SHIMETA_SDK_ALGORITHM_DENOISE_EVENT_FLOW_FILTER_H
#define SHIMETA_SDK_ALGORITHM_DENOISE_EVENT_FLOW_FILTER_H

#include <vector>
#include <deque>
#include <Eigen/Dense>
#include <cmath>
#include <metavision/sdk/base/events/event_cd.h>

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

/// @brief Event Flow Filter for CD events.
/// @details 该滤波器基于事件流的稠密性和流速特征进行噪声抑制。
class EventFlowFilter {
private:
    size_t mSearchRadius;
    double mFloatThreshold;
    size_t mBufferSize;
    int64_t mDuration; // us

    std::deque<Metavision::EventCD> mDeque;

public:
    /// @brief 构造函数
    /// @param bufferSize 缓冲区大小
    /// @param searchRadius 空间邻域半径
    /// @param floatThreshold 流速阈值
    /// @param duration 时间窗口(us)
    explicit EventFlowFilter(
        const size_t bufferSize = 100,
        const size_t searchRadius = 1,
        const double floatThreshold = 20.0,
        const int64_t duration = 2000
    );

    /// @brief 初始化滤波器
    void initialize();

    /// @brief 计算事件的流速
    /// @param event 输入事件
    /// @return 流速值
    double fitEventFlow(const Metavision::EventCD &event);

    /// @brief 判断事件是否为信号
    /// @param event 输入事件
    /// @return true为信号，false为噪声
    bool evaluate(const Metavision::EventCD &event);

    /// @brief 保留接口
    inline bool retain(const Metavision::EventCD &event) noexcept {
        return evaluate(event);
    }

    /// @brief 批量处理事件
    /// @param events 输入事件向量
    /// @return 保留的事件向量
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
};

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta

#endif // SHIMETA_SDK_ALGORITHM_DENOISE_EVENT_FLOW_FILTER_H
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
#ifndef SHIMETA_SDK_ALGORITHM_DENOISE_RECLUSIVE_EVENT_DENOISOR_H
#define SHIMETA_SDK_ALGORITHM_DENOISE_RECLUSIVE_EVENT_DENOISOR_H

#include <vector>
#include <metavision/sdk/base/events/event_cd.h>

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

/// @brief Recursive Event Denoisor (RED) for CD events.
/// @details Implements the RED算法，支持Metavision事件格式的批量处理。
class ReclusiveEventDenoisor {
private:
    int width_;
    int height_;
    int tau_;      // 时间常数，单位us
    int n_;        // 空间邻域半径
    std::vector<std::vector<int64_t>> last_event_time_on_;
    std::vector<std::vector<int64_t>> last_event_time_off_;

public:
    /// @brief 构造函数
    /// @param width 传感器宽度
    /// @param height 传感器高度
    /// @param tau 时间常数
    /// @param n 空间邻域半径
    ReclusiveEventDenoisor(int width, int height, int tau, int n);

    /// @brief 处理一批事件，返回去噪后的事件
    /// @param events 输入事件
    /// @return 去噪后的事件
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);

    /// @brief 重置内部状态
    void reset();
};

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta

#endif // SHIMETA_SDK_ALGORITHM_DENOISE_RECLUSIVE_EVENT_DENOISOR_H
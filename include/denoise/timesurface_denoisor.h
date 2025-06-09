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
#ifndef SHIMETA_SDK_ALGORITHM_DENOISE_TIMESURFACE_DENOISOR_H
#define SHIMETA_SDK_ALGORITHM_DENOISE_TIMESURFACE_DENOISOR_H

#include <vector>
#include <cmath>
#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/base/events/event_cd_vector.h>
#include <metavision/sdk/base/events/event2d.h>

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

/// @brief Time Surface Denoisor for CD events.
/// @details 该滤波器基于时空邻域的时间表面特征对事件进行去噪。
class TimeSurfaceDenoisor {
private:
    int mWidth;
    int mHeight;
    size_t mSearchRadius;
    double mDecay;
    double mFloatThreshold;

    // 记录正负极性事件的时间表面
    std::vector<std::vector<int64_t>> mPos;
    std::vector<std::vector<int64_t>> mNeg;

public:
    /// @brief 构造函数
    /// @param width 图像宽度
    /// @param height 图像高度
    /// @param decay 时间衰减常数（微秒）
    /// @param searchRadius 搜索半径
    /// @param floatThreshold 判定阈值
    TimeSurfaceDenoisor(int width, int height, double decay = 20000, size_t searchRadius = 1, double floatThreshold = 0.2);

    /// @brief 初始化表面
    void initialize();

    /// @brief 判断单个事件是否为信号
    /// @param event 输入事件
    /// @return true为信号，false为噪声
    bool evaluate(const Metavision::EventCD &event);

    /// @brief 处理单个事件
    /// @param event 输入事件
    /// @return true为保留，false为丢弃
    inline bool retain(const Metavision::EventCD &event) noexcept {
        return evaluate(event);
    }

    /// @brief 批量处理事件
    /// @param events 输入事件向量
    /// @return 去噪后的事件向量
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
};

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta

#endif // SHIMETA_SDK_ALGORITHM_DENOISE_TIMESURFACE_DENOISOR_H
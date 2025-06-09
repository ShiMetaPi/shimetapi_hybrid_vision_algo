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
#include "denoise/reclusive_event_denoisor.h"
#include <algorithm>
#include <limits>

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

ReclusiveEventDenoisor::ReclusiveEventDenoisor(int width, int height, int tau, int n)
    : width_(width), height_(height), tau_(tau), n_(n),
      last_event_time_on_(width, std::vector<int64_t>(height, std::numeric_limits<int64_t>::min())),
      last_event_time_off_(width, std::vector<int64_t>(height, std::numeric_limits<int64_t>::min())) {}

void ReclusiveEventDenoisor::reset() {
    std::fill(last_event_time_on_.begin(), last_event_time_on_.end(), std::vector<int64_t>(height_, std::numeric_limits<int64_t>::min()));
    std::fill(last_event_time_off_.begin(), last_event_time_off_.end(), std::vector<int64_t>(height_, std::numeric_limits<int64_t>::min()));
}

std::vector<Metavision::EventCD> ReclusiveEventDenoisor::process_events(const std::vector<Metavision::EventCD> &events) {
    std::vector<Metavision::EventCD> denoised_events;
    for (const auto &ev : events) {
        int x = ev.x;
        int y = ev.y;
        int p = ev.p;
        int64_t t = ev.t;
        bool is_signal = false;
        // 检查空间邻域内是否有同极性事件在tau时间内发生
        for (int dx = -n_; dx <= n_; ++dx) {
            for (int dy = -n_; dy <= n_; ++dy) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx < 0 || nx >= width_ || ny < 0 || ny >= height_)
                    continue;
                int64_t last_t = (p == 1) ? last_event_time_on_[nx][ny] : last_event_time_off_[nx][ny];
                if (t - last_t <= tau_) {
                    is_signal = true;
                    break;
                }
            }
            if (is_signal) break;
        }
        // 更新当前像素的最后事件时间
        if (p == 1) {
            last_event_time_on_[x][y] = t;
        } else {
            last_event_time_off_[x][y] = t;
        }
        if (is_signal) {
            denoised_events.push_back(ev);
        }
    }
    return denoised_events;
}

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta
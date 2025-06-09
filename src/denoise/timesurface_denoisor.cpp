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
#include "denoise/timesurface_denoisor.h"

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

TimeSurfaceDenoisor::TimeSurfaceDenoisor(int width, int height, double decay, size_t searchRadius, double floatThreshold)
    : mWidth(width), mHeight(height), mSearchRadius(searchRadius), mDecay(decay), mFloatThreshold(floatThreshold) {
    initialize();
}

void TimeSurfaceDenoisor::initialize() {
    mPos.assign(mWidth, std::vector<int64_t>(mHeight, 0));
    mNeg.assign(mWidth, std::vector<int64_t>(mHeight, 0));
}

bool TimeSurfaceDenoisor::evaluate(const Metavision::EventCD &event) {
    int16_t x = event.x;
    int16_t y = event.y;
    int polarity = event.p;
    int64_t ts = event.t;

    size_t support = 0;
    double diffTime = 0.0;
    auto &surface = (polarity == 1) ? mPos : mNeg;

    for (int dx = -static_cast<int>(mSearchRadius); dx <= static_cast<int>(mSearchRadius); ++dx) {
        for (int dy = -static_cast<int>(mSearchRadius); dy <= static_cast<int>(mSearchRadius); ++dy) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx < 0 || ny < 0 || nx >= mWidth || ny >= mHeight)
                continue;
            int64_t neighbor_ts = surface[nx][ny];
            if (neighbor_ts == 0)
                continue;
            diffTime += std::exp((neighbor_ts - ts) / mDecay);
            ++support;
        }
    }
    double surface_val = (support == 0) ? 0.0 : diffTime / support;
    // 更新时间表面
    surface[x][y] = ts;
    return surface_val >= mFloatThreshold;
}

std::vector<Metavision::EventCD> TimeSurfaceDenoisor::process_events(const std::vector<Metavision::EventCD> &events) {
    std::vector<Metavision::EventCD> output;
    output.reserve(events.size());
    for (const auto &event : events) {
        if (retain(event)) {
            output.push_back(event);
        }
    }
    return output;
}

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta
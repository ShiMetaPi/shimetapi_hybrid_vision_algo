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
#include "denoise/yang_noise_filter.h"

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

YangNoiseFilter::YangNoiseFilter(
    const int16_t width,
    const int16_t height,
    const int64_t duration,
    const size_t searchRadius,
    const size_t intThreshold
) :
    mWidth(width),
    mHeight(height),
    mDuration(duration),
    mSearchRadius(searchRadius),
    mIntThreshold(intThreshold)
{
    initialize();
}

void YangNoiseFilter::initialize() {
    mLastTimestamps.assign(mWidth, std::vector<int64_t>(mHeight, 0));
    mLastPolarities.assign(mWidth, std::vector<uint8_t>(mHeight, 0));
}

size_t YangNoiseFilter::calculateDensity(const Metavision::EventCD &event) {
    size_t density = 0;

    // Calculate spatio-temporal density
    for (int16_t dy = -mSearchRadius; dy <= (int16_t)mSearchRadius; ++dy) {
        for (int16_t dx = -mSearchRadius; dx <= (int16_t)mSearchRadius; ++dx) {
            int16_t x = event.x + dx;
            int16_t y = event.y + dy;

            if (x >= 0 && x < mWidth && y >= 0 && y < mHeight) {
                if (event.t - mLastTimestamps[x][y] <= mDuration) {
                    if (event.p == mLastPolarities[x][y]) {
                        density++;
                    }
                }
            }
        }
    }

    return density;
}

bool YangNoiseFilter::evaluate(const Metavision::EventCD &event) {
    // calculate density in spatio-temporal neighborhood
    size_t density = calculateDensity(event);

    // evaluate
    bool isSignal = (density >= mIntThreshold);

    // update matrix
    mLastTimestamps[event.x][event.y] = event.t;
    mLastPolarities[event.x][event.y] = event.p;

    return isSignal;
}

std::vector<Metavision::EventCD> YangNoiseFilter::process_events(const std::vector<Metavision::EventCD> &events) {
    std::vector<Metavision::EventCD> retained_events;
    for (const auto& event : events) {
        if (retain(event)) {
            retained_events.push_back(event);
        }
    }
    return retained_events;
}

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta
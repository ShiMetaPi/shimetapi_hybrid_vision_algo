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
#include "denoise/double_window_filter.h"

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

DoubleWindowFilter::DoubleWindowFilter(
    const size_t bufferSize,
    const size_t searchRadius,
    const size_t intThreshold
) :
    mBufferSize(bufferSize),
    mSearchRadius(searchRadius),
    mIntThreshold(intThreshold)
{
    initialize();
}

void DoubleWindowFilter::initialize() {
    lastRealEvents.clear();
    lastNoiseEvents.clear();
    lastRealEvents.resize(mBufferSize);
    lastNoiseEvents.resize(mBufferSize);
}

size_t DoubleWindowFilter::countNearbyEvents(const Metavision::EventCD &event) {
    size_t count = 0;

    // count events in real events window
    for (const auto &real : lastRealEvents) {
        // Check if the event is valid (not default constructed empty event)
        // Assuming default constructed EventCD has timestamp 0 or some invalid state
        if (real.t != 0) {
            if (std::abs(event.x - real.x) + std::abs(event.y - real.y) <= mSearchRadius) {
                count++;
                if (count >= mIntThreshold) {
                    return count;
                }
            }
        }
    }

    // count events in noise events window
    for (const auto &noise : lastNoiseEvents) {
         // Check if the event is valid
        if (noise.t != 0) {
            if (std::abs(event.x - noise.x) + std::abs(event.y - noise.y) <= mSearchRadius) {
                count++;
                if (count >= mIntThreshold) {
                    return count;
                }
            }
        }
    }

    return count;
}

bool DoubleWindowFilter::evaluate(const Metavision::EventCD &event) {
    // count related events in window
    size_t count = countNearbyEvents(event);

    // evaluate
    bool isSignal = (count >= mIntThreshold);

    // update
    if (isSignal) {
        if (lastRealEvents.size() == mBufferSize) {
            lastRealEvents.pop_front();
        }
        lastRealEvents.push_back(event);
    } else {
         if (lastNoiseEvents.size() == mBufferSize) {
            lastNoiseEvents.pop_front();
        }
        lastNoiseEvents.push_back(event);
    }

    return isSignal;
}

std::vector<Metavision::EventCD> DoubleWindowFilter::process_events(const std::vector<Metavision::EventCD> &events) {
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
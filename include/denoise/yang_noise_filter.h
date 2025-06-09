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
#ifndef SHIMETA_SDK_ALGORITHM_DENOISE_YANG_NOISE_FILTER_H
#define SHIMETA_SDK_ALGORITHM_DENOISE_YANG_NOISE_FILTER_H

#include <vector>
#include <cmath>
#include <deque>

#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/base/events/event2d.h>

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

/// @brief Yang Noise Filter for CD events.
/// @details This filter uses a spatio-temporal density approach to classify events as real or noise.
class YangNoiseFilter {
private:
    int16_t mWidth;
    int16_t mHeight;
    int64_t mDuration;
    size_t mSearchRadius;
    size_t mIntThreshold;

    std::vector<std::vector<int64_t>> mLastTimestamps;
    std::vector<std::vector<uint8_t>> mLastPolarities;

public:
    /// @brief Constructor
    /// @param width Sensor width.
    /// @param height Sensor height.
    /// @param duration Time window duration in microseconds.
    /// @param searchRadius Maximum L1 distance for spatio-temporal search.
    /// @param intThreshold Minimum number of nearby events to classify an event as real.
    explicit YangNoiseFilter(
        const int16_t width,
        const int16_t height,
        const int64_t duration = 10000,
        const size_t searchRadius = 1,
        const size_t intThreshold = 2
    );

    /// @brief Initialize the filter.
    void initialize();

    /// @brief Calculate spatio-temporal density around an event.
    /// @param event The event to check.
    /// @return The number of nearby events within the spatio-temporal window.
    size_t calculateDensity(const Metavision::EventCD &event);

    /// @brief Evaluate if an event is a signal or noise.
    /// @param event The event to evaluate.
    /// @return True if the event is a signal, false otherwise.
    bool evaluate(const Metavision::EventCD &event);

    /// @brief Process a single event.
    /// @param event The event to process.
    /// @return True if the event is retained (classified as signal), false otherwise.
    inline bool retain(const Metavision::EventCD &event) noexcept {
        return evaluate(event);
    }

    /// @brief Process a vector of events.
    /// @param events The vector of events to process.
    /// @return A vector containing only the retained events.
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
};

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta

#endif // SHIMETA_SDK_ALGORITHM_DENOISE_YANG_NOISE_FILTER_H
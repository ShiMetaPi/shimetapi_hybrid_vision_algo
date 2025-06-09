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
#ifndef SHIMETA_SDK_ALGORITHM_DENOISE_DOUBLE_WINDOW_FILTER_H
#define SHIMETA_SDK_ALGORITHM_DENOISE_DOUBLE_WINDOW_FILTER_H

#include <vector>
#include <cmath>
#include <deque>

#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/base/events/event_cd_vector.h>
#include <metavision/sdk/base/events/event2d.h>

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

/// @brief Double Window Filter for CD events.
/// @details This filter uses two circular buffers (windows) to classify events as real or noise.
class DoubleWindowFilter {
private:
    size_t mSearchRadius;
    size_t mIntThreshold;
    size_t mBufferSize;

    std::deque<Metavision::EventCD> lastRealEvents;
    std::deque<Metavision::EventCD> lastNoiseEvents;

public:
    /// @brief Constructor
    /// @param bufferSize Size of the circular buffers.
    /// @param searchRadius Maximum L1 distance to consider events nearby.
    /// @param intThreshold Minimum number of nearby events to classify an event as real.
    explicit DoubleWindowFilter(
        const size_t bufferSize = 36,
        const size_t searchRadius = 9,
        const size_t intThreshold = 1
    );

    /// @brief Initialize the filter.
    void initialize();

    /// @brief Count nearby events in both windows.
    /// @param event The event to check.
    /// @return The number of nearby events.
    size_t countNearbyEvents(const Metavision::EventCD &event);

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

#endif // SHIMETA_SDK_ALGORITHM_DENOISE_DOUBLE_WINDOW_FILTER_H
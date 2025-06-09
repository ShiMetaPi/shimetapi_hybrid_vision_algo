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
#include "denoise/event_flow_filter.h"

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

EventFlowFilter::EventFlowFilter(
    const size_t bufferSize,
    const size_t searchRadius,
    const double floatThreshold,
    const int64_t duration
) :
    mBufferSize(bufferSize),
    mSearchRadius(searchRadius),
    mFloatThreshold(floatThreshold),
    mDuration(duration)
{
    initialize();
}

void EventFlowFilter::initialize() {
    mDeque.clear();
    mDeque.resize(mBufferSize);
}

double EventFlowFilter::fitEventFlow(const Metavision::EventCD &event) {
    double flow = std::numeric_limits<double>::max();
    std::vector<Metavision::EventCD> candidateEvents;
    for (const auto &deque : mDeque) {
        if ((std::abs(static_cast<int>(event.x) - static_cast<int>(deque.x)) <= static_cast<int>(mSearchRadius)) &&
            (std::abs(static_cast<int>(event.y) - static_cast<int>(deque.y)) <= static_cast<int>(mSearchRadius))) {
            if (deque.t != 0) {
                candidateEvents.push_back(deque);
            }
        }
    }
    if (candidateEvents.size() > 3) {
        Eigen::MatrixXd A(candidateEvents.size(), 3);
        Eigen::MatrixXd b(candidateEvents.size(), 1);
        for (size_t i = 0; i < candidateEvents.size(); i++) {
            A(i, 0) = candidateEvents[i].x;
            A(i, 1) = candidateEvents[i].y;
            A(i, 2) = 1.0;
            b(i)    = (static_cast<double>(candidateEvents[i].t) - static_cast<double>(event.t)) * 1E-3;
        }
        Eigen::Vector3d X = A.colPivHouseholderQr().solve(b);
        if (X[0] != 0 && X[1] != 0) {
            flow = std::sqrt(std::pow(-1.0 / X[0], 2) + std::pow(-1.0 / X[1], 2));
        }
    }
    return flow;
}

bool EventFlowFilter::evaluate(const Metavision::EventCD &event) {
    double flow = fitEventFlow(event);
    bool isSignal = (flow <= mFloatThreshold);
    while (!mDeque.empty()) {
        if (static_cast<int64_t>(event.t) - static_cast<int64_t>(mDeque.front().t) >= mDuration) {
            mDeque.pop_front();
        } else {
            break;
        }
    }
    if (mDeque.size() == mBufferSize) {
        mDeque.pop_front();
    }
    mDeque.push_back(event);
    return isSignal;
}

std::vector<Metavision::EventCD> EventFlowFilter::process_events(const std::vector<Metavision::EventCD> &events) {
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
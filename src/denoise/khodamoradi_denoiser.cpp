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
#include "denoise/khodamoradi_denoiser.h"

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

KhodamoradiDenoiser::KhodamoradiDenoiser(uint16_t width, uint16_t height, Metavision::timestamp duration, size_t int_threshold)
    : width_(width)
    , height_(height)
    , duration_(duration)
    , int_threshold_(int_threshold)
    , last_event_x_(width)
    , last_event_y_(height) {
    // Initialize last_event_x_ and last_event_y_ with default events or a specific initial state if needed
    // For now, default constructor of EventCD is used, which might be sufficient.
}

void KhodamoradiDenoiser::initialize() {
    std::fill(last_event_x_.begin(), last_event_x_.end(), Metavision::EventCD());
    std::fill(last_event_y_.begin(), last_event_y_.end(), Metavision::EventCD());
}

std::vector<Metavision::EventCD> KhodamoradiDenoiser::process_events(const std::vector<Metavision::EventCD> &events) {
    std::vector<Metavision::EventCD> retained_events;
    for (const auto &event : events) {
        if (filter(event)) {
            retained_events.push_back(event);
        }
    }
    return retained_events;
}

size_t KhodamoradiDenoiser::searchCorrelation(const Metavision::EventCD& event) {
    size_t support = 0;
    bool xMinusOne  = (event.x > 0);
    bool xPlusOne   = (event.x < (width_ - 1));
    bool yMinusOne  = (event.y > 0);
    bool yPlusOne   = (event.y < (height_ - 1));

    if (xMinusOne) {
        const auto &xPrev = last_event_x_[event.x - 1];
        if ((event.t - xPrev.t) <= duration_ && event.p == xPrev.p) {
            if ((yMinusOne && (xPrev.y == (event.y - 1))) || (xPrev.y == event.y) || (yPlusOne && (xPrev.y == (event.y + 1)))) {
                support++;
            }
        }
    }

    const auto &xCell = last_event_x_[event.x];
    if ((event.t - xCell.t) <= duration_ && event.p == xCell.p) {
        if ((yMinusOne && (xCell.y == (event.y - 1))) || (yPlusOne && (xCell.y == (event.y + 1)))) {
            support++;
        }
    }

    if (xPlusOne) {
        const auto &xNext = last_event_x_[event.x + 1];
        if ((event.t - xNext.t) <= duration_ && event.p == xNext.p) {
            if ((yMinusOne && (xNext.y == (event.y - 1))) || (xNext.y == event.y) || (yPlusOne && (xNext.y == (event.y + 1)))) {
                support++;
            }
        }
    }

    if (yMinusOne) {
        const auto &yPrev = last_event_y_[event.y - 1];
        if ((event.t - yPrev.t) <= duration_ && event.p == yPrev.p) {
            if ((xMinusOne && (yPrev.x == (event.x - 1))) || (yPrev.x == event.x) || (xPlusOne && (yPrev.x == (event.x + 1)))) {
                support++;
            }
        }
    }

    const auto &yCell = last_event_y_[event.y];
    if ((event.t - yCell.t) <= duration_ && event.p == yCell.p) {
        if ((xMinusOne && (yCell.x == (event.x - 1))) || (xPlusOne && (yCell.x == (event.x + 1)))) {
            support++;
        }
    }

    if (yPlusOne) {
        const auto &yNext = last_event_y_[event.y + 1];
        if ((event.t - yNext.t) <= duration_ && event.p == yNext.p) {
            if ((xMinusOne && (yNext.x == (event.x - 1))) || (yNext.x == event.x) || (xPlusOne && (yNext.x == (event.x + 1)))) {
                support++;
            }
        }
    }

    return support;
}

bool KhodamoradiDenoiser::filter(const Metavision::EventCD& event) {
    // calculate density in spatio-temporal neighborhood
    size_t support = searchCorrelation(event);

    // evaluate
    bool isSignal = (support >= int_threshold_);

    // update matrix
    last_event_x_[event.x] = event;
    last_event_y_[event.y] = event;

    return isSignal;
}

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta
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
#include <metavision/sdk/stream/camera.h>
#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>
#include <metavision/sdk/ui/utils/window.h>
#include <metavision/sdk/ui/utils/event_loop.h>

#include <hv_algo/denoise/event_flow_filter.h>

int main(int argc, char *argv[]) {
    Metavision::Camera cam;
    if (argc >= 2) {
        cam = Metavision::Camera::from_file(argv[1]);
    } else {
        cam = Metavision::Camera::from_first_available();
    }
    int camera_width  = cam.geometry().get_width();
    int camera_height = cam.geometry().get_height();

    // 创建 EventFlowFilter 实例，参数可根据实际需求调整
    Shimeta::Algorithm::Denoise::EventFlowFilter eff_filter(36, 9, 1); // 示例参数

    const std::uint32_t acc = 20000;
    double fps = 50;
    auto frame_gen = Metavision::PeriodicFrameGenerationAlgorithm(camera_width, camera_height, acc, fps);

    Metavision::Window window("Metavision SDK EventFlow Denoising", camera_width, camera_height,
                              Metavision::BaseWindow::RenderMode::BGR);
    window.set_keyboard_callback(
        [&window](Metavision::UIKeyEvent key, int scancode, Metavision::UIAction action, int mods) {
            if (action == Metavision::UIAction::RELEASE &&
                (key == Metavision::UIKeyEvent::KEY_ESCAPE || key == Metavision::UIKeyEvent::KEY_Q)) {
                window.set_close_flag();
            }
        });

    cam.cd().add_callback([&](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
        std::vector<Metavision::EventCD> denoised_events;
        std::vector<Metavision::EventCD> input_events(begin, end);
        denoised_events = eff_filter.process_events(input_events);
        if (!denoised_events.empty()) {
            frame_gen.process_events(denoised_events.begin(), denoised_events.end());
        }
    });

    frame_gen.set_output_callback([&](Metavision::timestamp, cv::Mat &frame) { window.show(frame); });
    cam.start();
    while (cam.is_running() && !window.should_close()) {
        static constexpr std::int64_t kSleepPeriodMs = 20;
        Metavision::EventLoop::poll_and_dispatch(kSleepPeriodMs);
    }
    cam.stop();
    return 0;
}
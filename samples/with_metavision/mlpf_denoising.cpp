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

#include <hv_algo/denoise/multi_layer_perceptron_filter.h>

// main loop
int main(int argc, char *argv[]) {
    Metavision::Camera cam; // create the camera
    
    // Default parameters for MLP filter
    std::filesystem::path modelPath="/home/diskb/ljx/workspace/Shimeta_el/models/MLPF_2xMSEO1H20_linear_7.pt";
    size_t batchSize = 5000;
    int64_t duration = 100000;  // 100ms in microseconds
    double threshold = 0.8;
    // std::string device = "cuda:0";
    std::string device = "cpu";

    if (argc >= 2) {
        // if we passed a file path, open it
        cam = Metavision::Camera::from_file(argv[1]);
    } else {
        // open the first available camera
        cam = Metavision::Camera::from_first_available();
    }
    
    // Parse additional command line arguments for MLP filter
    if (argc > 2) {
        modelPath = std::filesystem::path(argv[2]);
    }
    if (argc > 3) {
        batchSize = std::stoul(argv[3]);
    }
    if (argc > 4) {
        threshold = std::stod(argv[4]);
    }

    // Get camera geometry
    int camera_width  = cam.geometry().get_width();
    int camera_height = cam.geometry().get_height();
    
    std::pair<int, int> resolution = {camera_width, camera_height};

    // Create a Multi-Layer Perceptron Filter instance
    Shimeta::Algorithm::Denoise::MultiLayerPerceptronFilter mlp_filter(
        resolution, modelPath, batchSize, duration, threshold, device
    );

    // Create a frame generator for visualization (optional)
    const std::uint32_t acc = 20000; // Accumulation time in microseconds (20ms)
    double fps              = 50;    // Frames per second
    auto frame_gen = Metavision::PeriodicFrameGenerationAlgorithm(camera_width, camera_height, acc, fps);

    // Create a window for visualization (optional)
    Metavision::Window window("Metavision SDK MLP Denoising", camera_width, camera_height,
                              Metavision::BaseWindow::RenderMode::BGR);

    // Set a callback on the window to close it when Escape or Q is pressed (optional)
    window.set_keyboard_callback(
        [&window](Metavision::UIKeyEvent key, int scancode, Metavision::UIAction action, int mods) {
            if (action == Metavision::UIAction::RELEASE &&
                (key == Metavision::UIKeyEvent::KEY_ESCAPE || key == Metavision::UIKeyEvent::KEY_Q)) {
                window.set_close_flag();
            }
        });

    // Add a callback to the camera's CD event stream
    cam.cd().add_callback([&](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
        // Use a vector to store the denoised events
        std::vector<Metavision::EventCD> denoised_events;
        // Process events with the MLP filter
        std::vector<Metavision::EventCD> input_events(begin, end);
        denoised_events = mlp_filter.process_events(input_events);

        // Pass denoised events to the frame generator for visualization (optional)
        if (!denoised_events.empty()) {
            frame_gen.process_events(denoised_events.begin(), denoised_events.end());
        }
    });

    // Set a callback on the frame generator to display the frame (optional)
    frame_gen.set_output_callback([&](Metavision::timestamp, cv::Mat &frame) { window.show(frame); });

    // Start the camera
    cam.start();

    // Keep running until the camera is off, the recording is finished or the window is closed
    while (cam.is_running() && !window.should_close()) {
        // Poll events from the system (keyboard, mouse etc.)
        static constexpr std::int64_t kSleepPeriodMs = 20;
        Metavision::EventLoop::poll_and_dispatch(kSleepPeriodMs);
    }

    // Stop the camera
    cam.stop();

    return 0;
}
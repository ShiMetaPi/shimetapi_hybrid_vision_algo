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
#ifndef SHIMETA_SDK_ALGORITHM_DENOISE_MULTI_LAYER_PERCEPTRON_FILTER_H
#define SHIMETA_SDK_ALGORITHM_DENOISE_MULTI_LAYER_PERCEPTRON_FILTER_H

#include <vector>
#include <cmath>
#include <filesystem>
#include <memory>

#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/base/events/event_cd_vector.h>
#include <metavision/sdk/base/events/event2d.h>

#include <torch/cuda.h>
#include <torch/script.h>
#include <torch/torch.h>

namespace fs = std::filesystem;

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

/// @brief Multi-Layer Perceptron Filter for CD events.
/// @details This filter uses a pre-trained neural network to classify events as real or noise.
class MultiLayerPerceptronFilter {
private:
    int16_t mWidth;
    int16_t mHeight;
    fs::path mModelPath;
    bool mModelIsLoad;
    int32_t mBatchSize;
    int64_t mDuration;
    double mFloatThreshold;

    const int16_t mInputDepth = 2;
    const int16_t mInputWidth = 7;
    const int16_t mInputHeight = 7;
    const int16_t mInputArea = mInputWidth * mInputHeight;
    const int16_t mInputVolume = mInputDepth * mInputWidth * mInputHeight;

    // Time surface for maintaining event history
    std::vector<std::vector<Metavision::EventCD>> mTimeSurface;
    
    // Offset patterns for neighborhood lookup
    std::vector<std::pair<int, int>> mOffsets;
    
    torch::Device mDevice;
    torch::jit::script::Module mPreTrainedModel;

    // Event batch buffer
    std::vector<Metavision::EventCD> mEventBuffer;

    /// @brief Parse device string to torch::Device
    /// @param deviceStr Device string ("cpu", "cuda:0", etc.)
    /// @return Corresponding torch::Device object
    torch::Device parseDeviceString(const std::string &deviceStr);
    
    /// @brief Initialize the offset pattern for neighborhood lookup
    void initializeOffsets();
    
    /// @brief Initialize the time surface
    void initializeTimeSurface();
    
    /// @brief Calculate logarithmic time difference
    /// @param fromTime The reference time
    /// @param toTime The comparison time
    /// @return Logarithmic time difference
    double logarithmicTimeDiff(const int64_t &fromTime, const int64_t &toTime);
    
    /// @brief Build input tensor from event batch
    /// @param events Vector of events to process
    /// @return Input tensor for neural network
    torch::Tensor buildInputTensor(const std::vector<Metavision::EventCD> &events);
    
    /// @brief Process a batch of events through the neural network
    /// @param events Vector of events to process
    /// @return Vector of events classified as signal
    std::vector<Metavision::EventCD> processBatch(const std::vector<Metavision::EventCD> &events);

public:
    /// @brief Constructor
    /// @param resolution Resolution of the sensor (width, height)
    /// @param modelPath Path to the pre-trained PyTorch model
    /// @param batchSize Number of events to process in each batch
    /// @param duration Time duration for temporal features (in microseconds)
    /// @param floatThreshold Threshold for neural network output
    /// @param device Device name ("cpu" for CPU, "cuda:0" for first GPU, etc.)
    explicit MultiLayerPerceptronFilter(
        const std::pair<int, int> &resolution,
        const fs::path &modelPath = fs::path(),
        const size_t batchSize = 5000,
        const int64_t duration = 100000,
        const double floatThreshold = 0.8,
        const std::string &device = "cuda:0"
    );

    /// @brief Initialize the filter
    void initialize();

    /// @brief Evaluate if an event is a signal or noise
    /// @param event The event to evaluate
    /// @return True if the event is a signal, false otherwise
    bool evaluate(const Metavision::EventCD &event);

    /// @brief Alias for evaluate method to maintain compatibility
    /// @param event The event to retain or discard
    /// @return True if the event should be retained, false otherwise
    bool retain(const Metavision::EventCD &event) {
        return evaluate(event);
    }

    /// @brief Process a vector of events
    /// @param events Input events to process
    /// @return Vector of events classified as signal
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
};

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta

#endif // SHIMETA_SDK_ALGORITHM_DENOISE_MULTI_LAYER_PERCEPTRON_FILTER_H
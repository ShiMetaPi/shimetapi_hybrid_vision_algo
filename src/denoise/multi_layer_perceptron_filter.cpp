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
#include "denoise/multi_layer_perceptron_filter.h"
#include <string>
#include <stdexcept>

namespace Shimeta {
namespace Algorithm {
namespace Denoise {

MultiLayerPerceptronFilter::MultiLayerPerceptronFilter(
    const std::pair<int, int> &resolution,
    const fs::path &modelPath,
    const size_t batchSize,
    const int64_t duration,
    const double floatThreshold,
    const std::string &device
) :
    mWidth(resolution.first),
    mHeight(resolution.second),
    mModelPath(modelPath),
    mModelIsLoad(false),
    mBatchSize(batchSize),
    mDuration(duration),
    mFloatThreshold(floatThreshold),
    mDevice(parseDeviceString(device))
{
    initialize();
}

torch::Device MultiLayerPerceptronFilter::parseDeviceString(const std::string &deviceStr) {
    if (deviceStr == "cpu") {
        return torch::kCPU;
    } else if (deviceStr.substr(0, 4) == "cuda") {
        if (deviceStr == "cuda") {
            return torch::kCUDA;
        } else if (deviceStr.length() > 5 && deviceStr[4] == ':') {
            try {
                int deviceId = std::stoi(deviceStr.substr(5));
                return torch::Device(torch::kCUDA, deviceId);
            } catch (const std::invalid_argument& e) {
                throw std::invalid_argument("Invalid CUDA device format: " + deviceStr);
            } catch (const std::out_of_range& e) {
                throw std::out_of_range("CUDA device ID out of range: " + deviceStr);
            }
        } else {
            throw std::invalid_argument("Invalid CUDA device format: " + deviceStr);
        }
    } else {
        throw std::invalid_argument("Unsupported device type: " + deviceStr);
    }
}

void MultiLayerPerceptronFilter::initialize() {
    // Initialize time surface
    initializeTimeSurface();
    
    // Initialize offset patterns
    initializeOffsets();
    
    // Load the neural network model
    if (!mModelPath.empty() && !mModelIsLoad) {
        try {
            mPreTrainedModel = torch::jit::load(mModelPath, mDevice);
            mModelIsLoad = true;
        } catch (const std::exception& e) {
            // If the specified device is not available, try fallback to CPU
            if (mDevice.type() != torch::kCPU) {
                try {
                    mDevice = torch::kCPU;
                    mPreTrainedModel = torch::jit::load(mModelPath, mDevice);
                    mModelIsLoad = true;
                    // Log warning about device fallback (could be implemented if logging is available)
                } catch (const std::exception& cpu_e) {
                    throw std::runtime_error("Failed to load model on both specified device and CPU: " + std::string(cpu_e.what()));
                }
            } else {
                throw std::runtime_error("Failed to load model on CPU: " + std::string(e.what()));
            }
        }
    }
    
    // Reserve space for event buffer
    mEventBuffer.reserve(mBatchSize);
}

void MultiLayerPerceptronFilter::initializeTimeSurface() {
    mTimeSurface.resize(mWidth);
    for (auto& column : mTimeSurface) {
        column.resize(mHeight);
        for (auto& event : column) {
            event.x = 0;
            event.y = 0;
            event.p = 0;
            event.t = 0;
        }
    }
}

void MultiLayerPerceptronFilter::initializeOffsets() {
    mOffsets.clear();
    int halfWidth = mInputWidth / 2;
    int halfHeight = mInputHeight / 2;
    
    for (int dy = -halfHeight; dy <= halfHeight; ++dy) {
        for (int dx = -halfWidth; dx <= halfWidth; ++dx) {
            mOffsets.emplace_back(dx, dy);
        }
    }
}

double MultiLayerPerceptronFilter::logarithmicTimeDiff(const int64_t &fromTime, const int64_t &toTime) {
    double deltaTime = fromTime - toTime;
    double maxDeltaTime = 5000000.0;
    double minDeltaTime = 150.0;
    
    deltaTime = std::min(deltaTime, maxDeltaTime);
    deltaTime = std::max(deltaTime, minDeltaTime);

    return std::log((deltaTime + 1.0) / (minDeltaTime + 1.0));
}

torch::Tensor MultiLayerPerceptronFilter::buildInputTensor(const std::vector<Metavision::EventCD> &events) {
    // Create empty input tensor
    torch::Tensor inputTensor = torch::empty({static_cast<long>(events.size()), mInputVolume});

    // Build input tensor
    for (size_t batchInd = 0; batchInd < events.size(); ++batchInd) {
        const auto &event = events[batchInd];
        
        // Construct single input
        std::vector<double> single(mInputVolume, 0.0);
        
        // Look up neighborhood
        for (size_t k = 0; k < mOffsets.size(); ++k) {
            int x = event.x + mOffsets[k].first;
            int y = event.y + mOffsets[k].second;

            if (x < 0 || y < 0 || x >= mWidth || y >= mHeight) {
                single[k] = 0.0;
                single[k + mInputArea] = 0.0;
            } else {
                // Get the last event at this pixel
                const auto &lastEvent = mTimeSurface[x][y];
                
                // Calculate temporal feature
                if (lastEvent.t != 0) {
                    single[k] = 1.0 - static_cast<double>(event.t - lastEvent.t) / mDuration;
                } else {
                    single[k] = 0.0;
                }
                
                // Calculate polarity feature
                single[k + mInputArea] = 2.0 * event.p - 1.0;
            }
        }
        
        inputTensor[batchInd] = torch::from_blob(single.data(), {mInputVolume}, torch::kFloat);
        
        // Update time surface
        mTimeSurface[event.x][event.y] = event;
    }

    return inputTensor;
}

std::vector<Metavision::EventCD> MultiLayerPerceptronFilter::processBatch(const std::vector<Metavision::EventCD> &events) {
    std::vector<Metavision::EventCD> retainedEvents;
    
    if (!mModelIsLoad || events.empty()) {
        return retainedEvents;
    }
    
    try {
        // Build input tensor
        torch::Tensor inputTensor = buildInputTensor(events);
        
        // Forward pass through neural network
        torch::Tensor outputTensor = mPreTrainedModel.forward({inputTensor.to(mDevice)}).toTensor().to(torch::kCPU);
        
        // Filter events based on neural network output
        for (size_t i = 0; i < events.size() && i < static_cast<size_t>(outputTensor.size(0)); ++i) {
            if (outputTensor[i][0].item<double>() >= mFloatThreshold) {
                retainedEvents.push_back(events[i]);
            }
        }
    } catch (const std::exception& e) {
        // If neural network fails, return all events (fail-safe mode)
        return events;
    }
    
    return retainedEvents;
}

bool MultiLayerPerceptronFilter::evaluate(const Metavision::EventCD &event) {
    // Add event to buffer
    mEventBuffer.push_back(event);
    
    // Process batch when buffer is full
    if (mEventBuffer.size() >= mBatchSize) {
        auto retainedEvents = processBatch(mEventBuffer);
        
        // Check if the current event is in the retained list
        bool isRetained = std::find_if(retainedEvents.begin(), retainedEvents.end(),
            [&event](const Metavision::EventCD& e) {
                return e.x == event.x && e.y == event.y && e.t == event.t && e.p == event.p;
            }) != retainedEvents.end();
        
        // Clear buffer
        mEventBuffer.clear();
        
        return isRetained;
    }
    
    // If buffer is not full, assume event is valid (batch processing limitation)
    return true;
}

std::vector<Metavision::EventCD> MultiLayerPerceptronFilter::process_events(const std::vector<Metavision::EventCD> &events) {
    std::vector<Metavision::EventCD> retainedEvents;
    
    if (!mModelIsLoad) {
        // If model is not loaded, return all events
        return events;
    }
    
    // Process events in batches
    for (size_t i = 0; i < events.size(); i += mBatchSize) {
        size_t endIdx = std::min(i + mBatchSize, events.size());
        std::vector<Metavision::EventCD> batch(events.begin() + i, events.begin() + endIdx);
        
        auto batchResults = processBatch(batch);
        retainedEvents.insert(retainedEvents.end(), batchResults.begin(), batchResults.end());
    }
    
    return retainedEvents;
}

} // namespace Denoise
} // namespace Algorithm
} // namespace Shimeta
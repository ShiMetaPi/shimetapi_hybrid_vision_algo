# HVAlgo API Reference Documentation

## Language / 语言

- [English](API_EN.md) | [中文](API.md)

## Overview

HVAlgo is a high-performance algorithm library specifically designed for Event Cameras, providing various advanced event denoising algorithms. This document provides detailed instructions on how to use all public APIs in the library.

## Namespaces

All algorithms are located under the `Shimeta::Algorithm` namespace, further divided by functional modules:

- `Shimeta::Algorithm::Denoise` - Denoising algorithm module
- `Shimeta::Algorithm::CV` - Computer vision module
- `Shimeta::Algorithm::CV3D` - 3D vision module
- `Shimeta::Algorithm::Restoration` - Image restoration module

## Denoising Algorithm Module (Denoise)

### 1. DoubleWindowFilter

The double window filter uses two circular buffers to classify CD events.

#### Class Definition
```cpp
class DoubleWindowFilter {
public:
    explicit DoubleWindowFilter(
        const size_t bufferSize = 36,
        const size_t searchRadius = 9,
        const size_t intThreshold = 1
    );
    
    void initialize();
    size_t countNearbyEvents(const Metavision::EventCD &event);
    bool evaluate(const Metavision::EventCD &event);
    bool retain(const Metavision::EventCD &event) noexcept;
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
};
```

#### Constructor Parameters
- `bufferSize`: Circular buffer size (default: 36)
- `searchRadius`: Search radius, maximum L1 distance for considering nearby events (default: 9)
- `intThreshold`: Minimum number of nearby events to classify an event as real (default: 1)

#### Main Methods
- `initialize()`: Initialize the filter
- `countNearbyEvents()`: Count nearby events in both windows
- `evaluate()`: Evaluate whether an event is signal or noise
- `retain()`: Inline method for processing single events
- `process_events()`: Batch process event vectors

#### Usage Example
```cpp
#include <denoise/double_window_filter.h>

// Create filter
Shimeta::Algorithm::Denoise::DoubleWindowFilter filter(36, 9, 1);
filter.initialize();

// Process single event
Metavision::EventCD event;
bool isSignal = filter.evaluate(event);

// Batch processing
std::vector<Metavision::EventCD> events;
auto filteredEvents = filter.process_events(events);
```

### 2. EventFlowFilter

A noise suppression filter based on event flow density and velocity characteristics.

#### Class Definition
```cpp
class EventFlowFilter {
public:
    explicit EventFlowFilter(
        const size_t bufferSize = 100,
        const size_t searchRadius = 1,
        const double floatThreshold = 20.0,
        const int64_t duration = 2000
    );
    
    void initialize();
    double fitEventFlow(const Metavision::EventCD &event);
    bool evaluate(const Metavision::EventCD &event);
    bool retain(const Metavision::EventCD &event) noexcept;
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
};
```

#### Constructor Parameters
- `bufferSize`: Buffer size (default: 100)
- `searchRadius`: Spatial neighborhood radius (default: 1)
- `floatThreshold`: Flow velocity threshold (default: 20.0)
- `duration`: Time window in microseconds (default: 2000)

#### Main Methods
- `initialize()`: Initialize the filter
- `fitEventFlow()`: Calculate event flow velocity
- `evaluate()`: Determine if an event is signal
- `retain()`: Inline method for retention interface
- `process_events()`: Batch process events

### 3. KhodamoradiDenoiser

Classic denoising algorithm based on spatiotemporal neighborhoods, suitable for Metavision CD events.

#### Class Definition
```cpp
class KhodamoradiDenoiser {
public:
    explicit KhodamoradiDenoiser(
        uint16_t width,
        uint16_t height,
        Metavision::timestamp duration = 2000,
        size_t int_threshold = 2
    );
    
    void initialize();
    bool filter(const Metavision::EventCD &event);
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
};
```

#### Constructor Parameters
- `width`: Sensor width
- `height`: Sensor height
- `duration`: Time window length in microseconds (default: 2000)
- `int_threshold`: Support pixel threshold (default: 2)

#### Main Methods
- `initialize()`: Initialize the filter
- `filter()`: Filter single event
- `process_events()`: Batch process events

### 4. MultiLayerPerceptronFilter

Deep learning filter that uses pre-trained neural networks to classify events.

#### Class Definition
```cpp
class MultiLayerPerceptronFilter {
public:
    explicit MultiLayerPerceptronFilter(
        const std::pair<int, int> &resolution,
        const fs::path &modelPath = fs::path(),
        const size_t batchSize = 5000,
        const int64_t duration = 100000,
        const double floatThreshold = 0.8,
        const std::string &device = "cuda:0"
    );
    
    void initialize();
    bool evaluate(const Metavision::EventCD &event);
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
};
```

#### Constructor Parameters
- `resolution`: Sensor resolution (width, height)
- `modelPath`: Pre-trained PyTorch model path
- `batchSize`: Number of events processed per batch (default: 5000)
- `duration`: Time feature duration in microseconds (default: 100000)
- `floatThreshold`: Neural network output threshold (default: 0.8)
- `device`: Device name ("cpu" or "cuda:0" etc., default: "cuda:0")

#### Main Methods
- `initialize()`: Initialize the filter
- `evaluate()`: Evaluate whether an event is signal or noise
- `process_events()`: Batch process events

#### Dependencies
- Requires PyTorch C++ library support
- Compilation requires enabling `ENABLE_TORCH=ON`

### 5. ReclusiveEventDenoisor

Reclusive Event Denoisor (RED), supports batch processing of Metavision event formats.

#### Class Definition
```cpp
class ReclusiveEventDenoisor {
public:
    ReclusiveEventDenoisor(int width, int height, int tau, int n);
    
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
    void reset();
};
```

#### Constructor Parameters
- `width`: Sensor width
- `height`: Sensor height
- `tau`: Time constant in microseconds
- `n`: Spatial neighborhood radius

#### Main Methods
- `process_events()`: Process a batch of events, return denoised events
- `reset()`: Reset internal state

### 6. TimeSurfaceDenoisor

Denoises events based on spatiotemporal neighborhood time surface features.

#### Class Definition
```cpp
class TimeSurfaceDenoisor {
public:
    TimeSurfaceDenoisor(
        int width, 
        int height, 
        double decay = 20000, 
        size_t searchRadius = 1, 
        double floatThreshold = 0.2
    );
    
    void initialize();
    bool evaluate(const Metavision::EventCD &event);
    bool retain(const Metavision::EventCD &event) noexcept;
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
};
```

#### Constructor Parameters
- `width`: Image width
- `height`: Image height
- `decay`: Time decay constant in microseconds (default: 20000)
- `searchRadius`: Search radius (default: 1)
- `floatThreshold`: Decision threshold (default: 0.2)

#### Main Methods
- `initialize()`: Initialize surface
- `evaluate()`: Determine if a single event is signal
- `retain()`: Inline method for processing single events
- `process_events()`: Batch process events

### 7. YangNoiseFilter

Noise filter that classifies events using spatiotemporal density methods.

#### Class Definition
```cpp
class YangNoiseFilter {
public:
    explicit YangNoiseFilter(
        const int16_t width,
        const int16_t height,
        const int64_t duration = 10000,
        const size_t searchRadius = 1,
        const size_t intThreshold = 2
    );
    
    void initialize();
    size_t calculateDensity(const Metavision::EventCD &event);
    bool evaluate(const Metavision::EventCD &event);
    bool retain(const Metavision::EventCD &event) noexcept;
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
};
```

#### Constructor Parameters
- `width`: Sensor width
- `height`: Sensor height
- `duration`: Time window duration in microseconds (default: 10000)
- `searchRadius`: Maximum L1 distance for spatiotemporal search (default: 1)
- `intThreshold`: Minimum number of nearby events to classify an event as real (default: 2)

#### Main Methods
- `initialize()`: Initialize the filter
- `calculateDensity()`: Calculate spatiotemporal density around an event
- `evaluate()`: Evaluate whether an event is signal or noise
- `retain()`: Inline method for processing single events
- `process_events()`: Batch process event vectors

## Common Interface Design

### Event Types

All algorithms use standard event types from Metavision SDK:
- `Metavision::EventCD`: Change detection events
- `std::vector<Metavision::EventCD>`: Event vectors

### Common Method Patterns

Most filters follow the following interface pattern:

1. **Constructor**: Accepts algorithm-specific parameters
2. **initialize()**: Initialize internal state
3. **evaluate()**: Evaluate single events
4. **retain()**: Inline version of single event processing
5. **process_events()**: Batch process event vectors

### Performance Recommendations

1. **Batch Processing**: Prefer using `process_events()` for batch processing to achieve better performance
2. **Inline Methods**: For real-time processing, use `retain()` inline methods
3. **Parameter Tuning**: Adjust algorithm parameters according to specific application scenarios
4. **GPU Acceleration**: For MLP filters, using CUDA devices can significantly improve performance

## Compilation Requirements

### Basic Dependencies
- C++17 compatible compiler
- CMake >= 3.16
- Metavision SDK (base, core components)
- Eigen3 linear algebra library

### Optional Dependencies
- PyTorch C++ library (for MLP filter)
- CUDA (GPU acceleration support)

### Compilation Options
```bash
# Basic compilation
cmake ..
make -j$(nproc)

# Enable PyTorch support
cmake -DENABLE_TORCH=ON ..
make -j$(nproc)
```

## Usage Examples

### Basic Usage
```cpp
#include <denoise/double_window_filter.h>
#include <metavision/sdk/base/events/event_cd.h>

int main() {
    // Create filter
    Shimeta::Algorithm::Denoise::DoubleWindowFilter filter(36, 9, 1);
    filter.initialize();
    
    // Process events
    std::vector<Metavision::EventCD> events;
    // ... populate event data
    
    auto filteredEvents = filter.process_events(events);
    
    return 0;
}
```

### Multi-Algorithm Combination
```cpp
#include <denoise/double_window_filter.h>
#include <denoise/yang_noise_filter.h>

int main() {
    // Create multiple filters
    Shimeta::Algorithm::Denoise::DoubleWindowFilter dwf(36, 9, 1);
    Shimeta::Algorithm::Denoise::YangNoiseFilter ynf(640, 480, 10000, 1, 2);
    
    dwf.initialize();
    ynf.initialize();
    
    std::vector<Metavision::EventCD> events;
    // ... populate event data
    
    // Sequential processing
    auto step1 = dwf.process_events(events);
    auto step2 = ynf.process_events(step1);
    
    return 0;
}
```

## Error Handling

### Common Errors
1. **Model file not found**: MLP filter requires valid model file path
2. **Device unavailable**: Falls back to CPU when CUDA device is unavailable
3. **Out of memory**: May occur when processing large batches of events
4. **Invalid parameters**: Sensor dimensions, thresholds, etc. need to be within reasonable ranges

### Debugging Suggestions
1. Check validity of event data
2. Verify sensor parameter settings
3. Monitor memory usage
4. Use appropriate batch sizes

## Version Information

Current API version is based on HVAlgo v0.1.0. API may change in future versions. Thorough testing is recommended before production use.

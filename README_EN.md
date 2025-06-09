# HVAlgo - High-Performance Event Vision Algorithm Library

[中文](README.md) | **English**

## Introduction

HVAlgo is a high-performance algorithm library designed specifically for Event Cameras by the ShiMetaPi technology community. It provides various advanced event denoising, computer vision, and image restoration algorithms. Built on the ShiMetaPi SDK, it supports real-time event stream processing and is suitable for applications in robotic vision, autonomous driving, high-speed motion analysis, and more.

## Block Diagram

![框图](./assets/imgs/block_diagram.png)


## Key Features

- 🚀 **High Performance**: Optimized C++17 implementation supporting real-time event stream processing
- 🧠 **Multiple Denoising Algorithms**: Including traditional and deep learning methods
- 🔧 **Easy Integration**: Provides CMake configuration and pkg-config support
- 📦 **Modular Design**: Clear module division for easy extension and maintenance
- 🎯 **Practical Examples**: Rich sample code and documentation

## Algorithm Modules

### Denoising Algorithms

1. **Double Window Filter**
   - Uses two circular buffers to classify events
   - Suitable for real-time processing in low-noise environments

2. **Event Flow Filter**
   - Noise suppression based on event flow density and velocity characteristics
   - Suitable for denoising in motion scenarios

3. **Khodamoradi Denoiser**
   - Classic denoising algorithm based on spatiotemporal neighborhoods
   - General solution balancing performance and effectiveness

4. **Reclusive Event Denoiser**
   - Recursive event processing algorithm
   - Suitable for complex noise environments

5. **Time Surface Denoiser**
   - Denoising method based on time surfaces
   - Preserves event temporal characteristics

6. **Yang Noise Filter**
   - Noise filtering algorithm proposed by Yang et al.
   - Efficient real-time processing capability

7. **Multi-Layer Perceptron Filter (MLP Filter)** *(Optional)*
   - Intelligent denoising based on deep learning
   - Requires PyTorch support
   - Suitable for high-precision denoising in complex scenarios

### Computer Vision (CV)
- Basic image processing algorithms
- Feature extraction and matching

### 3D Vision (CV3D)
- Stereo vision algorithms
- Depth estimation
- 3D reconstruction

### Image Restoration
- Image enhancement algorithms
- Deblurring and super-resolution

## System Requirements

### Required Dependencies
- **Ubuntu** == 22.04
- **CMake** >= 3.16
- **C++17** compatible compiler
- **Metavision SDK**
- **Eigen3**

### Optional Dependencies
- **PyTorch** (for MLP filter)
- **CUDA** (GPU acceleration support)

## Installation Guide

### Install Dependencies

#### Ubuntu/Debian
```bash
# Install basic dependencies
sudo apt update
sudo apt install cmake build-essential libeigen3-dev

# Install Metavision
curl -L https://propheseeai.jfrog.io/artifactory/api/security/keypair/prophesee-gpg/public >/tmp/propheseeai.jfrog.op.asc
sudo cp /tmp/propheseeai.jfrog.op.asc /etc/apt/trusted.gpg.d
sudo add-apt-repository 'https://propheseeai.jfrog.io/artifactory/openeb-debian/'
sudo apt update
sudo apt -y install metavision-openeb
```

#### Optional: PyTorch Support
```bash
# Download LibTorch
wget https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-2.7.0%2Bcpu.zip
unzip libtorch-cxx11-abi-shared-with-deps-2.7.0+cpu.zip
export CMAKE_PREFIX_PATH=/path/to/libtorch:$CMAKE_PREFIX_PATH
```

### Install Shimeta DVS Camera SDK (Optional)

This project includes the HV Toolkit submodule for simplified event data processing. Follow these steps to install:

```bash
# Pull submodules
git submodule update --init

# Enter submodule directory
cd external/hv_toolkit

# Create build directory
mkdir build && cd build

# Configure CMake
cmake ..

# Compile project
make -j$(nproc)

# Install library files
sudo make install
```

### Build and Install

```bash
mkdir build && cd build

# Basic build
cmake ..
make -j$(nproc)

# Enable PyTorch support (optional)
cmake -DENABLE_TORCH=ON ..
make -j$(nproc)

# Install to system
sudo make install
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_TORCH` | OFF | Enable PyTorch support |
| `BUILD_SAMPLES` | OFF | Build sample programs |
| `BUILD_TESTING` | OFF | Build test programs |
| `CMAKE_BUILD_TYPE` | Release | Build type (Debug/Release) |

## Usage

### CMake Integration

```cmake
find_package(HVAlgo REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app HVAlgo::hv_algo)
```

### pkg-config Integration

```bash
# Compile
g++ -o my_app main.cpp `pkg-config --cflags --libs hv_algo`
```

### Basic Usage Example

```cpp
#include <denoise/double_window_filter.h>
#include <metavision/sdk/base/events/event_cd.h>

int main() {
    // Create double window filter
    Shimeta::Algorithm::Denoise::DoubleWindowFilter filter(36, 9, 1);
    
    // Process events
    Metavision::EventCD event;
    bool is_signal = filter.evaluate(event);
    
    if (is_signal) {
        // Process valid events
    }
    
    return 0;
}
```

## Sample Programs

The project provides rich sample programs demonstrating the usage of various algorithms:

```bash
# Build samples
cd samples/with_hv_toolkit
mkdir build && cd build
cmake ..
make
# Run sample
./dwf_denoising [event_file.raw]
```

### Available Samples

- `dwf_denoising`: Double Window Filter denoising example
- `event_flow_denoising`: Event Flow Filter example
- `khodamoradi_denoising`: Khodamoradi denoiser example
- `mlpf_denoising`: MLP filter example (requires PyTorch)
- `re_denoising`: Reclusive Event Denoiser example
- `ts_denoising`: Time Surface Denoiser example
- `y_denoising`: Yang filter example

## Project Structure

```
hv_algo/
├── include/                 # Header files
│   ├── denoise/            # Denoising algorithms
│   ├── cv/                 # Computer vision
│   ├── cv3d/               # 3D vision
│   └── restoration/        # Image restoration
├── src/                    # Source code
├── samples/                # Sample programs
│   ├── with_metavision/    # Metavision SDK samples
│   └── with_hv_toolkit/    # HV Toolkit samples
├── cmake/                  # CMake configuration files
├── data/                   # Test data
├── models/                 # Pre-trained models
└── external/               # External dependencies
```

## Acknowledgments

- [Metavision SDK](https://docs.prophesee.ai/) - Event camera development framework
- [Eigen](https://eigen.tuxfamily.org/) - Linear algebra library
- [PyTorch](https://pytorch.org/) - Deep learning framework

## Contact

Open Hardware Website: www.shimetapi.cn (China) | www.shimetapi.com (International)  
Online Technical Documentation: https://forum.shimetapi.cn/wiki/zh/  
Online Technical Community: https://forum.shimetapi.cn  

**Note**: This project is still under active development, and APIs may change. It is recommended to conduct thorough testing before using in production environments.
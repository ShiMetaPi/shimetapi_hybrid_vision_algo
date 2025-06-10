# Hybrid Vision Algo (HVAlgo) - High-Performance Event Vision Algorithm Library

**Chinese** | [English](README_EN.md)

## Introduction

HVAlgo is a high-performance algorithm library designed by ShiMetaPi for event cameras (Event Camera), providing various advanced event denoising, computer vision, and image restoration algorithms. The library is built based on the ShiMetaPi Hybird Vision SDK and supports real-time event stream processing, making it suitable for applications such as robotic vision, autonomous driving, and high-speed motion analysis.

The ShiMetaPi Hybrid Vision SDK consists of two independent SDKs, hybrid\_vision\_toolkit and hybrid\_vision\_algo, which respectively implement the interface control and algorithm processing of the visual fusion camera.

## Project Diagram

![框图](./assets/imgs/block_diagram.png)

## **`shimetapi_Hybrid_vision_algo `**

* **Location:** This is the core algorithm processing layer of the SDK, located in the middle layer of the architecture (the yellow part).
* **Core Function:** Focuses on processing raw data streams from the event camera, executing advanced computer vision algorithms to enhance data quality, extract useful information, or perform 3D understanding.
* **Included modules:**
  * **Noise Reduction (`Denoise`):** Removes noise from the event stream to improve signal quality.
  * **Interpolation (`Interpolation`):** Used to generate intermediate data points between events, or for spatiotemporal alignment/fusion with standard frame images.
  * **Restoration (`Restoration`):** Used for handling missing or anomalous data.
  * **Computer Vision/3D Vision (`CV/CV3D`):** Includes algorithms for event camera-specific or fused applications, such as object detection, tracking, 3D reconstruction, pose estimation, etc.
  * **Samples/data (`Samples/data`):** Provides sample code, models, or necessary datasets.
  * **External Dependencies (`external`):** By integrating third-party libraries, it provides users with more device options, such as: Shimetapi Hybrid vision toolkit\`, OpenEb (installed via command) and others.
* **Function:** It provides processed, enhanced, or interpreted event data information to upper-layer applications (`APP`). Application developers primarily call the interfaces of this layer to implement advanced functionalities of event cameras (such as playback, recording, analysis).

## **`shimetapi_Hybrid_vision_toolkit`**

* **Positioning:** This is the Hardware Abstraction Layer (HAL) and the basic utilities layer of the SDK, located below the algorithm layer (deep blue section).
* **Core Functions:** It provides functionalities for interacting with hardware and basic data flow operations. It is responsible for reading raw data from physical interfaces, performing preliminary processing (such as encoding/decoding), and passing the data to the upper-layer algorithm layer. It also receives control commands from the upper layer to manage hardware.
* **Key Modules/Functions Included:**
  * **Read** (`Read`): Obtain the original event data stream from hardware interfaces (such as USB).
  * **Write** (`Write`): Used to send processed data or control commands back to the hardware (if supported).
  * **Setting** (`Setting`): Configure camera parameters (such as resolution, sensitivity, etc.).
  * **Decoder** (`Decoder`): If the event data is transmitted in a specific encoded format, it is responsible for decoding it into a processable format.
  * **Encoder (`Encoder`):** Used to encode processed data or recorded streams into a specific format.
* **Function:** It abstracts the details of the underlying hardware, providing a unified and relatively hardware-agnostic interface for the upper algorithm layer (`shimetapi_Hybrid_vision_algo`) to access and control the event camera data stream. It handles low-level communication protocols, data transportation, and basic format conversion.

## **Summary of the relationship and collaboration between the two:**

1. **`shimetapi_Hybrid_vision_toolkit :`**
   * Directly interacts with the hardware and the underlying system SDK.
   * Responsible for acquiring raw data (`Read`), performing basic format conversion (`Decoder`), controlling hardware (`Setting`), and outputting data (`Write`, `Encoder`), etc.
   * Provides a standardized data access interface to the upper layer (`algo` layer).
2. **`shimetapi_Hybrid_vision_algo :`**
   * Built on top of the `toolkit` layer.
   * Receives standardized event data streams from the `toolkit` layer.
   * Utilizes its advanced algorithm modules (such as `Denoise`, `Interpolation`, `CV/CV3D`, etc.) to process, enhance, analyze, and understand the data.
   * Delivers processed, more valuable information to the topmost application layer (`APP`) for use (such as displaying `UI`, `Player` playback, `Recorder` recording, `Analyzer` analysis).
   * Compatible with interfaces from different manufacturers (currently supports ShiMetaPi HV Toolkit and Openeb).

## Main Features

* 🚀 **High Performance**: Optimized C++17 implementation, supports real-time event stream processing
* 🧠 **Multiple Denoising Algorithms**: Includes traditional and deep learning methods
* 🔧 **Easy Integration**: Provides CMake configuration and pkg-config support
* 📦 **Modular Design**: Clear module division, facilitates extension and maintenance
* 🎯 **Practical Examples**: Rich example code and documentation

## Algorithm module

### Denoise algorithm

1. **Double window filter**

   * Classify events using two circular buffers
   * Real-time processing suitable for low-noise environments
2. **Event Flow Filter**

   * Noise suppression based on event flow density and velocity characteristics
   * Denoising suitable for motion scenarios
3. **Khodamoradi Denoiser**

   * Classic denoising algorithms based on spatiotemporal neighborhoods
   * A general solution that balances performance and effectiveness
4. **Reclusive Event Denoiser**

   * Recursive event processing algorithm
   * Suitable for complex noise environments
5. **Time Surface Denoiser**

   * Denoising method based on time surface
   * Maintain the temporal characteristics of events
6. **Yang noise filter**

   * The noise filtering algorithm proposed by Yang et al.
   * Efficient real-time processing capability
7. **Multi-Layer Perceptron Filter (MLP Filter)***(Optional)*

   * Deep Learning-based Smart Denoising
   * Requires PyTorch Support
   * High-precision Denoising for Complex Scenes

### Computer Vision (CV)

* Basic image processing algorithms
* Feature extraction and matching

### 3D Vision (CV3D)

* Stereo vision algorithms
* Depth estimation
* 3D reconstruction

### Image restoration

* Image enhancement algorithms
* Deblurring and super-resolution

## System requirements

### Required dependencies

* **Ubuntu** == 22.04
* **CMake** >= 3.16
* **C++17** compatible compiler
* **Openeb SDK**
* **Eigen3**

### Optional dependencies

* **PyTorch** (for MLP filters)
* **CUDA** (GPU Acceleration Supported)

## Installation Guide

### 1\. Install Dependencies

#### Ubuntu/Debian

```
# install base lib
sudo apt update
sudo apt install cmake build-essential libeigen3-dev

# install HV Toolkits
git submodule update --init
cd external/shimetapi_hybrid_vision_toolkit/
sudo ./install_libs.sh

# install Openeb
curl -L https://propheseeai.jfrog.io/artifactory/api/security/keypair/prophesee-gpg/public >/tmp/propheseeai.jfrog.op.asc
sudo cp /tmp/propheseeai.jfrog.op.asc /etc/apt/trusted.gpg.d
sudo add-apt-repository 'https://propheseeai.jfrog.io/artifactory/openeb-debian/'
sudo apt update
sudo apt -y install metavision-openeb
```

#### Optional: PyTorch Support

```bash
# download LibTorch
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.7.1%2Bcpu.zip
unzip libtorch-cxx11-abi-shared-with-deps-2.7.1+cpu.zip
export CMAKE_PREFIX_PATH=/path/to/libtorch:$CMAKE_PREFIX_PATH
```

### 2\. Compile and install

```bash
mkdir build && cd build

# build
cmake ..
make -j$(nproc)

# enable PyTorch (options)
cmake -DENABLE_TORCH=ON ..
make -j$(nproc)

# install
sudo make install
```

### 3\. Compilation options

| Options            | Default values | Description                |
| ------------------ | -------------- | -------------------------- |
| ENABLE\_TORCH      | OFF            | Enable PyTorch support     |
| BUILD\_SAMPLES     | OFF            | Compile sample program     |
| BUILD\_TESTING     | OFF            | Compile test program       |
| CMAKE\_BUILD\_TYPE | Release        | Build Type (Debug/Release) |

## Usage

### CMake Integration

```cmake
find_package(HVAlgo REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app HVAlgo::hv_algo)
```

### pkg-config Integration

```bash
# 编译
g++ -o my_app main.cpp `pkg-config --cflags --libs hv_algo`
```

### Basic usage examples

```cpp
#include <denoise/double_window_filter.h>
#include <metavision/sdk/base/events/event_cd.h>

int main() {
    // 创建双窗口滤波器
    Shimeta::Algorithm::Denoise::DoubleWindowFilter filter(36, 9, 1);
  
    // 处理事件
    Metavision::EventCD event;
    bool is_signal = filter.evaluate(event);
  
    if (is_signal) {
        // 处理有效事件
    }
  
    return 0;
}
```

## Example programs

The project provides a rich set of example programs demonstrating the use of various algorithms:

```bash
# 编译示例
cd samples/with_hv_toolkit
mkdir build && cd build
cmake ..
make
# 运行示例
./dwf_denoising ../../../data/events.raw
```

Screenshot of the program running![替代文本](./assets/imgs/run01.jpg)

### Available examples

* `dwf_denoising`: Example of dual-window filter denoising
* `event_flow_denoising`: Example of event flow filter denoising
* `khodamoradi_denoising`: Example of Khodamoradi denoiser
* `mlpf_denoising`: Example of MLP filter denoising (requires PyTorch)
* `re_denoising`: Example of a recursive event denoiser
* `ts_denoising`: Example of a time surface denoiser
* `y_denoising`: Example of a Yang filter

## Project Structure

```
hv_algo/
├── include/                # header files
│   ├── denoise/            # Denoise Algorithm
│   ├── cv/                 # Computer Vision
│   ├── cv3d/               # 3D Vision
│   └── restoration/        # Image Restoration
├── src/                    # Source Code
├── samples/                # Example Programs
│   ├── with_metavision/    # Openeb SDK Example
│   └── with_hv_toolkit/    # HV Toolkit Example
├── cmake/                  # cmake configuration file
├── data/                   # Test Data
├── models/                 # pre trained models
└── external/               # External Dependency
```

## Acknowledgements

* [Openeb SDK](https://docs.prophesee.ai/) - Event camera development framework
* [Eigen](https://eigen.tuxfamily.org/) - Linear algebra library
* [PyTorch](https://pytorch.org/) - Deep learning framework

## Contact Information

Open source hardware website: [https://www.shimetapi.cn](https://www.shimetapi.cn) (Domestic) [https://www.shimetapi.com](https://www.shimetapi.com) (Overseas) Online technical documentation: [https://forum.shimetapi.cn/wiki/zh/](https://forum.shimetapi.cn/wiki/zh/) Online technical community: [https://forum.shimetapi.cn](https://forum.shimetapi.cn)

**Attention**: This project is still under active development, and the API may change. It is recommended to conduct thorough testing before using it in a production environment.

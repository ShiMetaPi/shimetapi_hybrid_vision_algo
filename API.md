# HVAlgo API 参考文档

## Language / 语言

- [English](API_EN.md) | [中文](API.md)

## 概述

HVAlgo 是一个专为事件相机（Event Camera）设计的高性能算法库，提供了多种先进的事件去噪算法。本文档详细介绍了库中所有公开 API 的使用方法。

## 命名空间

所有算法都位于 `Shimeta::Algorithm` 命名空间下，按功能模块进一步划分：

- `Shimeta::Algorithm::Denoise` - 去噪算法模块
- `Shimeta::Algorithm::CV` - 计算机视觉模块
- `Shimeta::Algorithm::CV3D` - 三维视觉模块
- `Shimeta::Algorithm::Restoration` - 图像恢复模块

## 去噪算法模块 (Denoise)

### 1. DoubleWindowFilter

双窗口滤波器使用两个循环缓冲区对 CD 事件进行分类。

#### 类定义
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

#### 构造函数参数
- `bufferSize`: 循环缓冲区大小（默认：36）
- `searchRadius`: 搜索半径，考虑附近事件的最大 L1 距离（默认：9）
- `intThreshold`: 将事件分类为真实事件的最小附近事件数（默认：1）

#### 主要方法
- `initialize()`: 初始化滤波器
- `countNearbyEvents()`: 计算两个窗口中的附近事件数量
- `evaluate()`: 评估事件是信号还是噪声
- `retain()`: 处理单个事件的内联方法
- `process_events()`: 批量处理事件向量

#### 使用示例
```cpp
#include <denoise/double_window_filter.h>

// 创建滤波器
Shimeta::Algorithm::Denoise::DoubleWindowFilter filter(36, 9, 1);
filter.initialize();

// 处理单个事件
Metavision::EventCD event;
bool isSignal = filter.evaluate(event);

// 批量处理
std::vector<Metavision::EventCD> events;
auto filteredEvents = filter.process_events(events);
```

### 2. EventFlowFilter

基于事件流密度和流速特征的噪声抑制滤波器。

#### 类定义
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

#### 构造函数参数
- `bufferSize`: 缓冲区大小（默认：100）
- `searchRadius`: 空间邻域半径（默认：1）
- `floatThreshold`: 流速阈值（默认：20.0）
- `duration`: 时间窗口，单位微秒（默认：2000）

#### 主要方法
- `initialize()`: 初始化滤波器
- `fitEventFlow()`: 计算事件的流速
- `evaluate()`: 判断事件是否为信号
- `retain()`: 保留接口的内联方法
- `process_events()`: 批量处理事件

### 3. KhodamoradiDenoiser

基于时空邻域的经典去噪算法，适用于 Metavision CD 事件。

#### 类定义
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

#### 构造函数参数
- `width`: 传感器宽度
- `height`: 传感器高度
- `duration`: 时窗长度，单位微秒（默认：2000）
- `int_threshold`: 支持像素阈值（默认：2）

#### 主要方法
- `initialize()`: 初始化滤波器
- `filter()`: 过滤单个事件
- `process_events()`: 批量处理事件

### 4. MultiLayerPerceptronFilter

使用预训练神经网络对事件进行分类的深度学习滤波器。

#### 类定义
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

#### 构造函数参数
- `resolution`: 传感器分辨率 (width, height)
- `modelPath`: 预训练 PyTorch 模型路径
- `batchSize`: 每批处理的事件数量（默认：5000）
- `duration`: 时间特征持续时间，单位微秒（默认：100000）
- `floatThreshold`: 神经网络输出阈值（默认：0.8）
- `device`: 设备名称（"cpu" 或 "cuda:0" 等，默认："cuda:0"）

#### 主要方法
- `initialize()`: 初始化滤波器
- `evaluate()`: 评估事件是信号还是噪声
- `process_events()`: 批量处理事件

#### 依赖要求
- 需要 PyTorch C++ 库支持
- 编译时需要启用 `ENABLE_TORCH=ON`

### 5. ReclusiveEventDenoisor

递归事件去噪器（RED），支持 Metavision 事件格式的批量处理。

#### 类定义
```cpp
class ReclusiveEventDenoisor {
public:
    ReclusiveEventDenoisor(int width, int height, int tau, int n);
    
    std::vector<Metavision::EventCD> process_events(const std::vector<Metavision::EventCD> &events);
    void reset();
};
```

#### 构造函数参数
- `width`: 传感器宽度
- `height`: 传感器高度
- `tau`: 时间常数，单位微秒
- `n`: 空间邻域半径

#### 主要方法
- `process_events()`: 处理一批事件，返回去噪后的事件
- `reset()`: 重置内部状态

### 6. TimeSurfaceDenoisor

基于时空邻域的时间表面特征对事件进行去噪。

#### 类定义
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

#### 构造函数参数
- `width`: 图像宽度
- `height`: 图像高度
- `decay`: 时间衰减常数，单位微秒（默认：20000）
- `searchRadius`: 搜索半径（默认：1）
- `floatThreshold`: 判定阈值（默认：0.2）

#### 主要方法
- `initialize()`: 初始化表面
- `evaluate()`: 判断单个事件是否为信号
- `retain()`: 处理单个事件的内联方法
- `process_events()`: 批量处理事件

### 7. YangNoiseFilter

使用时空密度方法对事件进行分类的噪声滤波器。

#### 类定义
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

#### 构造函数参数
- `width`: 传感器宽度
- `height`: 传感器高度
- `duration`: 时间窗口持续时间，单位微秒（默认：10000）
- `searchRadius`: 时空搜索的最大 L1 距离（默认：1）
- `intThreshold`: 将事件分类为真实事件的最小附近事件数（默认：2）

#### 主要方法
- `initialize()`: 初始化滤波器
- `calculateDensity()`: 计算事件周围的时空密度
- `evaluate()`: 评估事件是信号还是噪声
- `retain()`: 处理单个事件的内联方法
- `process_events()`: 批量处理事件向量

## 通用接口设计

### 事件类型

所有算法都使用 Metavision SDK 的标准事件类型：
- `Metavision::EventCD`: 变化检测事件
- `std::vector<Metavision::EventCD>`: 事件向量

### 通用方法模式

大多数滤波器都遵循以下接口模式：

1. **构造函数**: 接受算法特定的参数
2. **initialize()**: 初始化内部状态
3. **evaluate()**: 评估单个事件
4. **retain()**: 内联版本的单事件处理
5. **process_events()**: 批量处理事件向量

### 性能建议

1. **批量处理**: 优先使用 `process_events()` 进行批量处理以获得更好的性能
2. **内联方法**: 对于实时处理，使用 `retain()` 内联方法
3. **参数调优**: 根据具体应用场景调整算法参数
4. **GPU 加速**: 对于 MLP 滤波器，使用 CUDA 设备可显著提升性能

## 编译要求

### 基础依赖
- C++17 兼容编译器
- CMake >= 3.16
- Metavision SDK (base, core 组件)
- Eigen3 线性代数库

### 可选依赖
- PyTorch C++ 库（用于 MLP 滤波器）
- CUDA（GPU 加速支持）

### 编译选项
```bash
# 基础编译
cmake ..
make -j$(nproc)

# 启用 PyTorch 支持
cmake -DENABLE_TORCH=ON ..
make -j$(nproc)
```

## 使用示例

### 基础使用
```cpp
#include <denoise/double_window_filter.h>
#include <metavision/sdk/base/events/event_cd.h>

int main() {
    // 创建滤波器
    Shimeta::Algorithm::Denoise::DoubleWindowFilter filter(36, 9, 1);
    filter.initialize();
    
    // 处理事件
    std::vector<Metavision::EventCD> events;
    // ... 填充事件数据
    
    auto filteredEvents = filter.process_events(events);
    
    return 0;
}
```

### 多算法组合
```cpp
#include <denoise/double_window_filter.h>
#include <denoise/yang_noise_filter.h>

int main() {
    // 创建多个滤波器
    Shimeta::Algorithm::Denoise::DoubleWindowFilter dwf(36, 9, 1);
    Shimeta::Algorithm::Denoise::YangNoiseFilter ynf(640, 480, 10000, 1, 2);
    
    dwf.initialize();
    ynf.initialize();
    
    std::vector<Metavision::EventCD> events;
    // ... 填充事件数据
    
    // 串联处理
    auto step1 = dwf.process_events(events);
    auto step2 = ynf.process_events(step1);
    
    return 0;
}
```

## 错误处理

### 常见错误
1. **模型文件不存在**: MLP 滤波器需要有效的模型文件路径
2. **设备不可用**: CUDA 设备不可用时会回退到 CPU
3. **内存不足**: 处理大批量事件时可能出现内存不足
4. **参数无效**: 传感器尺寸、阈值等参数需要在合理范围内

### 调试建议
1. 检查事件数据的有效性
2. 验证传感器参数设置
3. 监控内存使用情况
4. 使用适当的批处理大小

## 版本信息

当前 API 版本基于 HVAlgo v0.1 .0，API 可能在后续版本中发生变化。建议在生产环境使用前进行充分测试。

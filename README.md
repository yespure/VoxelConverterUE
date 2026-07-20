# Voxel Converter for Unreal Engine

一个基于 Unreal Engine 编辑器工具控件（Editor Utility Widget）制作的体素化工具。它会在指定区域内采样场景碰撞，并将命中的空间位置按体素尺寸离散化，生成可预览、可分块管理的体素结果。

> 本项目是 **Unreal Engine 5.4** 工程，核心工具使用蓝图实现，无需编译 C++。

## 功能

- 在指定的三维扫描区域内采样场景
- 通过碰撞检测识别实体表面
- 按可调的体素尺寸生成体素坐标与数据
- 按 Chunk 分组创建体素结果，便于管理较大范围
- 提供生成前预览、生成体素、清理结果三个操作
- 随工程附带示例关卡和体素 Chunk / Preview 蓝图

## 环境要求

- Unreal Engine **5.4**
- Windows（工程默认使用 DirectX 12 / Shader Model 6）
- 启用 **Modeling Tools Editor Mode** 插件（项目已在 `.uproject` 中声明）

## 快速开始

1. 克隆或下载本仓库。
2. 使用 Unreal Engine 5.4 打开 `MaptoVoxelEditorTool/MaptoVoxelEditorTool.uproject`。
3. 若编辑器提示启用插件，启用 **Modeling Tools Editor Mode** 后重启编辑器。
4. 在内容浏览器中定位到：

   `Content/EditorUtilites/EUW_VoxelConverter`

5. 右键该资源，选择 **Run Editor Utility Widget**。
6. 在打开的工具窗口中设置扫描范围与体素大小，然后依次使用扫描、预览和转换功能。

## 使用流程

### 1. 准备场景

将需要体素化的静态物体放入关卡，并确保其碰撞可被查询。工具通过场景射线检测采样；没有可查询碰撞的对象不会产生有效体素数据。

示例资源位于：

- `Content/Level/TestLevel1`
- `Content/BluePrint/BP_VoxelChunk`
- `Content/BluePrint/BP_VoxelPreview`

### 2. 设置参数

在 **EUW_VoxelConverter** 窗口中设置：

| 参数 | 说明 |
| --- | --- |
| Scan Area | 体素化的三维扫描范围 |
| Voxel Size | 单个体素的世界尺寸；数值越小，细节越高、采样和生成耗时越长 |

### 3. 扫描与预览

- 点击 **Scan**：在指定范围内采样并记录有效体素位置。
- 点击 **Visualize**：以调试球体显示当前采样结果，方便检查范围和密度。
- 调整扫描范围或体素大小后，请重新扫描。

### 4. 生成与清理

- 点击 **Convert**：根据扫描结果生成体素 Chunk 与对应数据。
- 点击 **Clear**：删除该工具本次生成的 Actor，并清空当前生成结果。

## 项目结构

```text
VoxelConverterUE/
├─ MaptoVoxelEditorTool/
│  ├─ Config/                         # Unreal 项目配置
│  ├─ Content/
│  │  ├─ EditorUtilites/
│  │  │  └─ EUW_VoxelConverter        # 主编辑器工具控件
│  │  ├─ BluePrint/
│  │  │  ├─ BP_VoxelChunk             # 体素块蓝图
│  │  │  ├─ BP_VoxelPreview           # 预览蓝图
│  │  │  └─ Data/                     # Voxel / Chunk / Group 数据结构
│  │  └─ Level/
│  │     └─ TestLevel1                # 示例关卡
│  └─ MaptoVoxelEditorTool.uproject
└─ README.md
```

## 注意事项

- **体素尺寸与性能**：扫描量会随着扫描范围增大、体素尺寸减小而快速增加。建议先使用较大的体素尺寸验证范围，再逐步提高精度。
- **碰撞设置**：工具依赖场景碰撞进行采样。若扫描结果为空，请先检查目标模型的碰撞和碰撞响应。
- **结果管理**：重复转换前建议先执行 **Clear**，避免关卡中保留多批生成结果。
- **版本兼容性**：工程目标版本为 UE 5.4；用其他版本打开时，编辑器可能要求升级资源或项目文件。
- **未完成内容**: 目前版本 VoxelSize 和 GroupSize 的 spin 分别只支持 100.0 和 4.0 两个数值,若在其中填入其他数值会导致Bug。

## 许可证

本项目采用 [MIT License](LICENSE)。

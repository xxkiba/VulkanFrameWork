# VulkanLearning

一个使用 Vulkan API 进行图形渲染学习的项目。

## 构建要求

- Visual Studio 2019 或更高版本
- CMake 3.20 或更高版本
- Vulkan SDK

## 构建步骤

### 1. 下载并安装 Vulkan SDK

1. 访问 [Vulkan SDK 官网](https://vulkan.lunarg.com/sdk/home)
2. 下载适合你系统的最新版本 Vulkan SDK
3. 按照安装向导完成安装

### 2. 设置 Vulkan 库文件

由于 Vulkan 库文件较大，项目中没有包含这些文件。你需要：

1. 找到你安装的 Vulkan SDK 目录（通常在 `C:\VulkanSDK\[版本号]\`）
2. 将整个 Vulkan SDK 文件夹复制到项目的 `Code/ThirdParty/` 目录下
3. 确保目录结构如下：
   ```
   Code/
   └── ThirdParty/
       └── VulkanSDK/
           ├── Include/
           ├── Lib/
           └── ...
   ```

### 3. 使用 Visual Studio 构建项目

1. 打开 Visual Studio
2. 选择 "打开本地文件夹" 或 "Open a local folder"
3. 导航到并选择 `Code` 文件夹（包含 CMakeLists.txt 的文件夹）
4. Visual Studio 会自动检测 CMake 项目并开始配置
5. 在工具栏中选择 **x64-Debug** 配置
6. 点击 "生成" 或按 `Ctrl+Shift+B` 编译项目

### 4. 运行项目

首次运行可能会遇到找不到资源文件的问题。解决方法：

1. 编译成功后，找到生成的可执行文件目录，通常在：
   ```
   out/build/x64-Debug/
   ```
   或
   ```
   out/x64-Debug/
   ```

2. 将以下文件夹复制到可执行文件所在目录：
   - `Code/shaders/` → 复制到可执行文件目录
   - `Code/assets/` → 复制到可执行文件目录

3. 确保目录结构如下：
   ```
   [可执行文件目录]/
   ├── VulkanLearning.exe
   ├── shaders/
   └── assets/
   ```

4. 现在可以运行项目了

## 常见问题

### Q: 编译时提示找不到 Vulkan 头文件
**A:** 确保已正确安装 Vulkan SDK，并将 SDK 文件夹放置在 `Code/ThirdParty/` 目录下。

### Q: 运行时程序崩溃或黑屏
**A:** 检查是否已将 `shaders` 和 `assets` 文件夹复制到可执行文件目录。

### Q: 链接错误
**A:** 确保使用 x64 配置进行编译，32位配置可能会出现链接问题。

## 开发环境

- **图形API**: Vulkan
- **构建系统**: CMake
- **编译器**: MSVC (Visual Studio)
- **平台**: Windows x64

## 许可证

[在此添加你的许可证信息]

## 贡献

欢迎提交 Issue 和 Pull Request！

---

**注意**: 确保你的显卡支持 Vulkan API。可以通过运行 `vulkaninfo` 命令来检查 Vulkan 支持情况。

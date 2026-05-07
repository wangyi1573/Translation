# 火山翻译 Notepad-- 插件

基于火山翻译 API 的 Notepad-- 文本翻译插件。

## 版本信息

- **版本**: v2.0.0
- **架构**: 原生 C++ 插件（使用 Qt 库）
- **接口**: NDD 插件接口（`NDD_PROC_IDENTIFY` / `NDD_PROC_MAIN`）

## 功能特性

- ✅ 多语言翻译（支持火山翻译 API 支持的所有语言）
- ✅ 自动语言检测
- ✅ 翻译结果缓存（1小时有效期）
- ✅ API 密钥加密存储
- ✅ 菜单集成 + 快捷键支持（Ctrl+Alt+T）

## 构建方法

### 方式一：使用 qmake（推荐）

```bash
# 设置 Qt 环境
set QTDIR=C:\Qt\5.15.2\msvc2019_64
set PATH=%QTDIR%\bin;%PATH%

# 生成 Makefile
qmake VolcengineTranslationNDD.pro -spec win32-msvc CONFIG+=release

# 编译
nmake
```

### 方式二：使用 CMake

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64
cmake --build . --config Release
```

## 安装方法

1. 编译生成 `VolcengineTranslation.dll`
2. 将 DLL 复制到 Notepad-- 的插件目录：
   - `Notepad--/plugins/VolcengineTranslation/`
3. 重启 Notepad--

## 使用方法

1. 首次使用需要配置 API 密钥：
   - 菜单: 火山翻译 → 配置API密钥...
   - 输入火山引擎的 Access Key ID 和 Access Key Secret

2. 翻译文本：
   - 选中要翻译的文本
   - 按 `Ctrl+Alt+T` 或菜单: 火山翻译 → 翻译选中文本

## 配置文件

配置文件位于：`Notepad--/plugins/config/volcengine_translation.ini`

包含：
- Access Key ID
- Access Key Secret（加密存储）
- 源语言/目标语言设置
- 超时设置

## 项目结构

```
Translation/
├── include/
│   ├── NddPluginInterface.h      # NDD 插件接口定义
│   └── PluginCore/
│       ├── HttpClient.h           # HTTP 客户端
│       ├── SignatureGenerator.h   # API 签名生成器
│       ├── TranslationEngine.h    # 翻译引擎
│       ├── ConfigManager.h        # 配置管理
│       └── TranslationResult.h    # 翻译结果结构
├── src/
│   ├── NddPluginMain.cpp          # 插件入口（NDD 接口实现）
│   └── PluginCore/
│       ├── HttpClient.cpp
│       ├── SignatureGenerator.cpp
│       ├── TranslationEngine.cpp
│       └── ConfigManager.cpp
├── CMakeLists.txt                 # CMake 构建文件
├── VolcengineTranslationNDD.pro   # qmake 构建文件
└── README.md
```

## API 文档

火山翻译 API 文档：https://www.volcengine.com/docs/4640

## 依赖

- Qt 5.11+ (Core, Gui, Network, Widgets)
- QScintilla (由 Notepad-- 提供)
- Windows API (可选，用于加密功能)

## 许可证

MIT License

## 更新日志

### v2.0.0 (2026-05-07)
- 重构为 NDD 原生插件接口
- 移除独立 UI，集成到 Notepad-- 菜单
- 优化 API 签名生成
- 添加翻译结果缓存
- 支持 Access Key Secret 加密存储
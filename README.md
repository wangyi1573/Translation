# Notepad-- 火山翻译插件

## 项目简介

这是一个基于Qt 5.15.2开发的Notepad--编辑器插件，集成了火山翻译API，提供文本翻译功能。插件支持Windows、Linux和macOS平台，采用轻量级设计，确保低耦合和易用性。

## 功能特点

- **三层架构设计**：主插件模块 + 配置面板 + 翻译核心，核心逻辑与UI解耦
- **多平台支持**：Windows (DLL)、Linux/macOS (动态库)
- **多种触发方式**：
  - 快捷键触发 (Ctrl+Alt+T)
  - 右键菜单触发
  - 工具栏按钮触发
- **独立配置窗口**：支持填写火山翻译的AccessKey ID、AccessKey Secret，并提供保存/重置/测试连接功能
- **多语言支持**：支持多种常用语言的互译
- **自动语言检测**：可自动检测源语言

## 系统要求

- Qt 5.15.2 或更高版本
- Notepad-- 编辑器
- 火山翻译API账号和密钥

## 安装方法

### Windows

1. 下载 `VolcengineTranslation.dll` 文件
2. 将DLL文件复制到Notepad--安装目录下的 `plugins` 文件夹中
3. 重启Notepad--编辑器

### Linux

1. 下载 `libVolcengineTranslation.so` 文件
2. 将动态库文件复制到Notepad--安装目录下的 `plugins` 文件夹中
3. 确保动态库有执行权限：`chmod +x libVolcengineTranslation.so`
4. 重启Notepad--编辑器

### macOS

1. 下载 `libVolcengineTranslation.dylib` 文件
2. 将动态库文件复制到Notepad--安装目录下的 `plugins` 文件夹中
3. 确保动态库有执行权限：`chmod +x libVolcengineTranslation.dylib`
4. 重启Notepad--编辑器

## 使用说明

### 配置API密钥

1. 点击菜单栏的 "插件" -> "火山翻译配置"
2. 在弹出的配置窗口中，输入您的火山翻译API的AccessKey ID和AccessKey Secret
3. 点击 "测试连接" 按钮验证API密钥是否有效
4. 点击 "保存" 按钮保存配置

### 翻译文本

方法一：使用快捷键
1. 选中要翻译的文本
2. 按下快捷键 `Ctrl+Alt+T`
3. 在弹出的翻译结果窗口中查看翻译结果

方法二：使用右键菜单
1. 选中要翻译的文本
2. 右键点击选中的文本
3. 在弹出的菜单中选择 "火山翻译"
4. 在弹出的翻译结果窗口中查看翻译结果

方法三：使用工具栏按钮
1. 选中要翻译的文本
2. 点击工具栏中的 "火山翻译" 按钮
3. 在弹出的翻译结果窗口中查看翻译结果

## 开发环境搭建

### 准备工作

1. 安装Qt 5.15.2或更高版本
2. 安装Visual Studio 2019 (Windows) 或 GCC/Clang (Linux/macOS)
3. 安装Notepad--源码和开发环境

### 编译步骤

1. 克隆代码仓库
2. 使用Qt Creator打开 `VolcengineTranslation.pro` 文件
3. 选择合适的构建配置 (Debug/Release)
4. 点击 "构建" 按钮编译项目
5. 编译完成后，在 `bin` 目录下找到生成的插件文件

## 项目结构

```
VolcengineTranslation/
├── include/                 # 头文件目录
│   ├── VolcengineTranslationPlugin.h    # 插件主类头文件
│   ├── TranslationCore.h                # 翻译核心类头文件
│   ├── ConfigManager.h                  # 配置管理类头文件
│   ├── ApiClient.h                      # API客户端类头文件
│   ├── SignatureGenerator.h             # 签名生成器类头文件
│   ├── TranslationAction.h              # 翻译动作类头文件
│   ├── TranslationToolbar.h             # 翻译工具栏类头文件
│   ├── ContextMenuHandler.h             # 右键菜单处理类头文件
│   ├── ConfigWindow.h                   # 配置窗口类头文件
│   ├── ConnectionTester.h               # 连接测试器类头文件
│   ├── ConfigStorage.h                  # 配置存储类头文件
│   └── TranslationResult.h              # 翻译结果数据结构头文件
├── src/                     # 源文件目录
│   ├── VolcengineTranslationPlugin.cpp  # 插件主类源文件
│   ├── TranslationCore.cpp              # 翻译核心类源文件
│   ├── ConfigManager.cpp                # 配置管理类源文件
│   ├── ApiClient.cpp                    # API客户端类源文件
│   ├── SignatureGenerator.cpp           # 签名生成器类源文件
│   ├── TranslationAction.cpp            # 翻译动作类源文件
│   ├── TranslationToolbar.cpp           # 翻译工具栏类源文件
│   ├── ContextMenuHandler.cpp           # 右键菜单处理类源文件
│   ├── ConfigWindow.cpp                 # 配置窗口类源文件
│   ├── ConnectionTester.cpp             # 连接测试器类源文件
│   └── ConfigStorage.cpp                # 配置存储类源文件
├── resources/               # 资源文件目录
│   └── volcengine_translation.qrc       # Qt资源文件
├── VolcengineTranslation.pro            # Qt项目文件
└── README.md                            # 项目说明文档
```

## 注意事项

1. 使用本插件需要您拥有火山翻译API的账号和密钥
2. 请妥善保管您的API密钥，不要泄露给他人
3. 火山翻译API可能会产生费用，请参考火山引擎官方定价
4. 本插件仅作为学习和开发用途，使用时请遵守相关法律法规

## 许可证

[MIT License](LICENSE)

## 联系方式

如有问题或建议，请联系项目维护者。
# VolcengineTranslationNDD.pro
# Notepad-- 插件版本 - 使用 NDD 插件接口

QT += core gui network widgets
QT -= dbus

TEMPLATE = lib
CONFIG += dll c++17
CONFIG -= plugin

# 插件名称
TARGET = VolcengineTranslation

# 定义导出宏
DEFINES += NDD_EXPORTDLL

# 包含路径
INCLUDEPATH += $$PWD/include

# 头文件
HEADERS += \
    $$PWD/include/NddPluginInterface.h \
    $$PWD/include/PluginCore/HttpClient.h \
    $$PWD/include/PluginCore/SignatureGenerator.h \
    $$PWD/include/PluginCore/TranslationEngine.h \
    $$PWD/include/PluginCore/ConfigManager.h \
    $$PWD/include/PluginCore/TranslationResult.h

# 源文件
SOURCES += \
    $$PWD/src/NddPluginMain.cpp \
    $$PWD/src/PluginCore/HttpClient.cpp \
    $$PWD/src/PluginCore/SignatureGenerator.cpp \
    $$PWD/src/PluginCore/TranslationEngine.cpp \
    $$PWD/src/PluginCore/ConfigManager.cpp

# Windows 特定设置
win32 {
    # 链接 crypt32 库（用于加密功能）
    LIBS += -lcrypt32
    
    # 输出目录
    DESTDIR = $$PWD/bin/win32
    
    # 运行时库配置
    CONFIG(release, debug|release) {
        DEFINES += NDEBUG
    }
}

# 输出信息
message(Building VolcengineTranslation NDD Plugin)
message(Qt version: $$[QT_VERSION])
message(Target: $$TARGET)
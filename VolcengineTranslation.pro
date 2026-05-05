# VolcengineTranslation.pro
# 火山翻译插件项目文件

QT += core gui network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VolcengineTranslation
TEMPLATE = lib
CONFIG += plugin

# 版本信息
VERSION = 1.0.0

# 包含路径
INCLUDEPATH += $$PWD/include

# 源文件
SOURCES += \
    src/VolcengineTranslationPlugin.cpp \
    src/TranslationCore.cpp \
    src/ConfigManager.cpp \
    src/ApiClient.cpp \
    src/SignatureGenerator.cpp \
    src/TranslationAction.cpp \
    src/TranslationToolbar.cpp \
    src/ContextMenuHandler.cpp \
    src/ConfigWindow.cpp \
    src/ConnectionTester.cpp \
    src/ConfigStorage.cpp

# 头文件
HEADERS += \
    include/VolcengineTranslationPlugin.h \
    include/TranslationCore.h \
    include/ConfigManager.h \
    include/ApiClient.h \
    include/SignatureGenerator.h \
    include/TranslationAction.h \
    include/TranslationToolbar.h \
    include/ContextMenuHandler.h \
    include/ConfigWindow.h \
    include/ConnectionTester.h \
    include/ConfigStorage.h \
    include/TranslationResult.h

# 资源文件
RESOURCES += \
    resources/volcengine_translation.qrc

# 目标路径
DESTDIR = $$PWD/bin

# 构建目录
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui
OBJECTS_DIR = $$PWD/build/obj

# 平台特定设置
win32 {
    # Windows平台设置
    LIBS += -lcrypt32
    DESTDIR = $$PWD/bin/win32
    CONFIG(debug, debug|release) {
        TARGET = $$join(TARGET,,d)
    }
}

unix {
    # Linux/macOS平台设置
    DESTDIR = $$PWD/bin/unix
    CONFIG(debug, debug|release) {
        TARGET = $$join(TARGET,,d)
    }
}

macx {
    # macOS特定设置
    DESTDIR = $$PWD/bin/macx
    QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/
}
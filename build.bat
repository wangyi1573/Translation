#!/bin/bash
set -e  # 任何命令失败立即退出

# ===================== 步骤1：环境初始化与检查 =====================
echo "【步骤1】初始化编译环境"
# 切换到代码检出目录（CodeArts 工作目录）
cd C:\workspace\workspace\j_9N4UIJSS

# 检查 Qt 5.11.2 环境是否可用
if [ ! -d "$QTDIR" ]; then
    echo "错误：未找到 Qt 5.11.2 环境，请检查 QTDIR 环境变量"
    exit 1
fi
echo "Qt 路径：$QTDIR"

# 检查项目文件是否存在
if [ ! -f "VolcengineTranslation.pro" ]; then
    echo "错误：未找到 VolcengineTranslation.pro 项目文件"
    ls -l  # 列出当前目录文件，便于排查路径问题
    exit 1
fi

# ===================== 步骤2：使用 qmake 生成 VS2019 解决方案 =====================
echo "【步骤2】生成 VS 解决方案文件"
# 调用 Qt 5.11.2 的 qmake，生成适配 VS2019 的 .sln 文件
"$QTDIR/bin/qmake.exe" ^
    VolcengineTranslation.pro ^
    -spec win32-msvc ^
    CONFIG+=release ^
    CONFIG+=x64 ^
    -o VolcengineTranslation.sln

# 检查是否生成成功
if [ ! -f "VolcengineTranslation.sln" ]; then
    echo "错误：qmake 未生成解决方案文件"
    exit 1
fi
echo "VS 解决方案生成成功：VolcengineTranslation.sln"

# ===================== 步骤3：修正 MSBuild 命令（核心修复） =====================
echo "【步骤3】执行 MSBuild 编译"
# 1. 修正 Platform 拼写错误（原 Plateform → Platform）
# 2. 明确指定解决方案文件路径
# 3. 指定 Qt 路径，避免编译时找不到 Qt 库
msbuild.exe ^
    VolcengineTranslation.sln ^
    /p:Configuration=Release ^
    /p:Platform=x64 ^
    /p:Qt5_DIR="$QTDIR/lib/cmake/Qt5" ^
    /m  # 多线程编译，提升速度

# ===================== 步骤4：检查编译结果并压缩 =====================
echo "【步骤4】压缩编译产物"
# 定义输出目录（与 .pro 文件中 DESTDIR 一致）
OUTPUT_DIR="./bin/win64/Release"

# 检查输出目录是否存在
if [ ! -d "$OUTPUT_DIR" ]; then
    echo "错误：编译输出目录不存在，编译失败"
    ls -l ./bin  # 列出 bin 目录内容，排查编译产物路径
    exit 1
fi

# 压缩产物（Force 参数覆盖已有文件）
powershell -Command "Compress-Archive -Path '$OUTPUT_DIR/*' -DestinationPath ./archive.zip -Force"

# ===================== 步骤5：验证结果 =====================
echo "【步骤5】验证编译结果"
if [ -f "./archive.zip" ]; then
    echo "编译成功！产物已压缩为 archive.zip"
    ls -l ./archive.zip
else
    echo "错误：压缩包生成失败"
    exit 1
fi

echo "编译流程全部完成"
exit 0
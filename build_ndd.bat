@echo off
chcp 65001 >nul
echo ========================================
echo 火山翻译 Notepad-- 插件构建脚本
echo ========================================
echo.

REM 设置 Qt 路径（请根据实际安装路径修改）
if "%QTDIR%"=="" (
    echo 警告: QTDIR 环境变量未设置
    echo 请设置 QTDIR 环境变量，例如:
    echo set QTDIR=C:\Qt\5.15.2\msvc2019_64
    echo.
    set /p QTDIR="请输入 Qt 安装路径: "
)

if not exist "%QTDIR%\bin\qmake.exe" (
    echo 错误: 在 %QTDIR% 中找不到 qmake.exe
    pause
    exit /b 1
)

echo Qt 路径: %QTDIR%
echo.

REM 设置 PATH
set PATH=%QTDIR%\bin%;%PATH%

REM 清理旧的构建文件
if exist "build_ndd" (
    echo 清理旧的构建文件...
    rmdir /s /q build_ndd
)

REM 创建构建目录
mkdir build_ndd
cd build_ndd

echo.
echo [1/3] 运行 qmake...
"%QTDIR%\bin\qmake.exe" ..\VolcengineTranslationNDD.pro -spec win32-msvc CONFIG+=release

if %ERRORLEVEL% neq 0 (
    echo 错误: qmake 失败
    cd ..
    pause
    exit /b 1
)

echo.
echo [2/3] 编译项目...
nmake release

if %ERRORLEVEL% neq 0 (
    echo 错误: 编译失败
    cd ..
    pause
    exit /b 1
)

echo.
echo [3/3] 检查输出...
if exist "..\bin\win32\VolcengineTranslation.dll" (
    echo.
    echo ========================================
    echo 构建成功！
    echo 输出文件: bin\win32\VolcengineTranslation.dll
    echo ========================================
    echo.
    echo 安装方法:
    echo 1. 将 VolcengineTranslation.dll 复制到 Notepad-- 的插件目录
    echo 2. 重启 Notepad--
    echo.
) else (
    echo 警告: 未找到输出 DLL 文件
)

cd ..
pause
@echo off
setlocal enabledelayedexpansion

echo ========================================
echo UE Project Auto-Build Tool
echo ========================================

:: 获取脚本所在目录（tool文件夹）和项目根目录
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%~dp0..\"

:: 切换到项目根目录
cd /d "%PROJECT_ROOT%"

echo Project Root: %PROJECT_ROOT%

:: 查找 .uproject 文件
set "UPROJECT_FILE="
for %%i in (*.uproject) do set "UPROJECT_FILE=%%i"

if "%UPROJECT_FILE%"=="" (
    echo Error: No .uproject file found in project root directory!
    echo Expected at: %PROJECT_ROOT%
    pause
    exit /b 1
)

echo Found project file: %UPROJECT_FILE%

:: 查找 UE4/UE5 引擎路径
set "ENGINE_PATH=F:\game\epic game\UE_5.4\"

:: 方法1: 检查注册表 (UE4)
for /f "tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine" /v "INSTALLDIR" 2^>nul') do set "ENGINE_PATH=%%b"

:: 方法2: 检查注册表 (UE5)
if "%ENGINE_PATH%"=="" (
    for /f "tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\UnrealEngine" /v "INSTALLDIR" 2^>nul') do set "ENGINE_PATH=%%b"
)

:: 方法3: 常见安装路径
if "%ENGINE_PATH%"=="" (
    if exist "D:\UE\UE5\UE_5.4\Engine\Binaries\DotNET\UnrealBuildTool.exe" (
        set "ENGINE_PATH=D:\UE\UE5\UE_5.4"
    )
)

if "%ENGINE_PATH%"=="" (
    echo Error: Could not find Unreal Engine installation!
    echo Please set ENGINE_PATH manually in this script.
    pause
    exit /b 1
)

echo Using Engine: %ENGINE_PATH%

:: 步骤1: 生成项目文件
echo.
echo [1/3] Generating project files...
"%ENGINE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="%PROJECT_ROOT%%UPROJECT_FILE%" -game -rocket -progress

if %errorlevel% neq 0 (
    echo Error: Failed to generate project files!
    pause
    exit /b 1
)

:: 步骤2: 构建解决方案
echo.
echo [2/3] Building solution...

:: 获取项目名称（不含扩展名）
for %%i in ("%UPROJECT_FILE%") do set "PROJECT_NAME=%%~ni"

:: 尝试使用 MSBuild
set "BUILD_SUCCESS=0"
if exist "%PROJECT_ROOT%%PROJECT_NAME%.sln" (
    echo Building %PROJECT_NAME%.sln...
    
    :: 查找 MSBuild
    set "MSBUILD_PATH="
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    )
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    )
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    )
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    )
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
    )
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
    )
    
    if defined MSBUILD_PATH (
        echo Using MSBuild: %MSBUILD_PATH%
        "%MSBUILD_PATH%" "%PROJECT_ROOT%%PROJECT_NAME%.sln" /p:Configuration=Development /p:Platform=Win64 /m /v:minimal /clp:Summary
        if !errorlevel! equ 0 set "BUILD_SUCCESS=1"
    ) else (
        echo MSBuild not found, skipping solution build...
    )
)

:: 步骤3: 使用 UnrealBuildTool 构建项目
echo.
echo [3/3] Building with UnrealBuildTool...
"%ENGINE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" %PROJECT_NAME%Editor Win64 Development -Project="%PROJECT_ROOT%%UPROJECT_FILE%" -WaitMutex -FromMsBuild

if %errorlevel% neq 0 (
    echo Error: Build failed with exit code: %errorlevel%
    pause
    exit /b 1
)

:: 完成
echo.
echo ========================================
echo Build process completed successfully!
echo ========================================

:: 返回工具目录
cd /d "%SCRIPT_DIR%"

pause
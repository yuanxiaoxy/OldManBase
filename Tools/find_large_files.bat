@echo off
setlocal enabledelayedexpansion

echo ========================================
echo        Large Files Finder Tool
echo ========================================
echo.

:: Set parameters
set "target_dir=%~1"
set "output_file=%~2"
set "min_size_mb=100"

:: Use defaults if parameters not provided
if "%target_dir%"=="" set "target_dir=."
if "%output_file%"=="" set "output_file=large_files.txt"

:: Calculate minimum size in bytes (100MB)
set /a min_size_bytes=%min_size_mb%*1024*1024

echo Scanning directory: %target_dir%
echo Output file: %output_file%
echo File size threshold: %min_size_mb% MB
echo.

:: Check if directory exists
if not exist "%target_dir%" (
    echo ERROR: Directory does not exist - "%target_dir%"
    pause
    exit /b 1
)

:: Delete existing output file
if exist "%output_file%" del "%output_file%"

echo Starting scan, please wait...
echo.

set "count=0"
set "total_size=0"

:: Use for /r to recursively scan all files
for /r "%target_dir%" %%F in (*) do (
    if exist "%%F" (
        :: Get file size
        for %%S in ("%%F") do set "file_size=%%~zS"
        
        :: Check if file size exceeds threshold
        if !file_size! gtr %min_size_bytes% (
            :: Write file path to output file
            echo %%F>> "%output_file%"
            
            :: Calculate file size in MB
            set /a file_size_mb=!file_size!/1024/1024
            set /a total_size+=!file_size!
            set /a count+=1
            
            :: Display progress
            echo [!count!] %%F (!file_size_mb! MB)
        )
    )
)

:: Calculate total size in GB
set /a total_size_gb=!total_size!/1024/1024/1024
set /a total_size_mb=!total_size!/1024/1024

echo.
echo ========================================
echo Scan completed!
echo Found !count! files larger than %min_size_mb% MB
echo Total space used: !total_size_mb! MB (!total_size_gb! GB)
echo Results saved to: %output_file%
echo ========================================
echo.

:: Ask to open result file
set /p "open_file=Open result file? (y/n): "
if /i "!open_file!"=="y" (
    if exist "%output_file%" (
        notepad "%output_file%"
    ) else (
        echo ERROR: Output file not found
    )
)

pause
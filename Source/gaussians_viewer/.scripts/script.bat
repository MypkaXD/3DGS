@echo off
setlocal enabledelayedexpansion 

set SDK_DIR=%cd%/../sdk

set GLFW_URL=https://github.com/glfw/glfw/archive/refs/heads/master.zip
set GLFW_ZIP=glfw-master.zip
set GLFW_INSTALL_DIR=glfw-master

set IMGUI_URL=https://github.com/ocornut/imgui/archive/refs/heads/master.zip
set IMGUI_ZIP=imgui-master.zip
set IMGUI_INSTALL_DIR=imgui-master

set GLM_URL=https://github.com/g-truc/glm/archive/refs/heads/master.zip
set GLM_ZIP=glm-master.zip
set GLM_INSTALL_DIR=glm-master

set URLS[0]=%GLFW_URL%
set URLS[1]=%IMGUI_URL%
set URLS[2]=%GLM_URL%

set ZIPS[0]=%GLFW_ZIP%
set ZIPS[1]=%IMGUI_ZIP%
set ZIPS[2]=%GLM_ZIP%

set DIRS[0]=%GLFW_INSTALL_DIR%
set DIRS[1]=%IMGUI_INSTALL_DIR%
set DIRS[2]=%GLM_INSTALL_DIR%

for /l %%i in (0, 1, 2) do (
    
    echo Downloading !ZIPS[%%i]! from !URLS[%%i]!
    powershell -Command "(New-Object Net.WebClient).DownloadFile('!URLS[%%i]!', '!ZIPS[%%i]!')"

    echo Unzip !ZIPS[%%i]!
    powershell -Command "Expand-Archive -Path '!ZIPS[%%i]!' -DestinationPath '%SDK_DIR%'"

    echo Creating folders...
    if not exist "%SDK_DIR%/!DIRS[%%i]!/mybuild" mkdir "%SDK_DIR%/!DIRS[%%i]!/mybuild"
    if not exist "%SDK_DIR%/!DIRS[%%i]!/myinstall" mkdir "%SDK_DIR%/!DIRS[%%i]!/myinstall"

    echo Cleaning up...
    del !ZIPS[%%i]!

)


cmake -S "%SDK_DIR%/%GLFW_INSTALL_DIR%" -B "%SDK_DIR%/%GLFW_INSTALL_DIR%/mybuild" -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="%SDK_DIR%/%GLFW_INSTALL_DIR%/myinstall" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_WIN32=ON -DGLFW_INSTALL=ON -DGLFW_USE_HYBRID_HPG=OFF -USE_MSVC_RUNTIME_LIBRARY_DLL=OFF

cmake --build "%SDK_DIR%/%GLFW_INSTALL_DIR%/mybuild" --config Release --target INSTALL

cmake -S "%SDK_DIR%/%GLM_INSTALL_DIR%" -B "%SDK_DIR%/%GLM_INSTALL_DIR%/mybuild" -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="%SDK_DIR%/%GLM_INSTALL_DIR%/myinstall" -DGLM_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF

cmake --build "%SDK_DIR%/%GLM_INSTALL_DIR%/mybuild" --config Release --target INSTALL
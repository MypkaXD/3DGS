@echo off
setlocal enabledelayedexpansion 

set GLEW_URL=https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.zip
set GLEW_ZIP=glew-2.1.0.zip
set GLEW_INSTALL_DIR=glew-2.1.0

set GLFW_URL=https://github.com/glfw/glfw/archive/refs/heads/master.zip
set GLFW_ZIP=glfw-master.zip
set GLFW_INSTALL_DIR=glfw-master

set ZLIB_URL=https://github.com/madler/zlib/releases/download/v1.3.1/zlib131.zip
set ZLIB_ZIP=zlib131.zip
set ZLIB_INSTALL_DIR=zlib-1.3.1

set LIBPNG_URL=https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.47.zip
set LIBPNG_ZIP=libpng-1.6.47.zip
set LIBPNG_INSTALL_DIR=libpng-1.6.47

set GEOMVIEW_URL=https://github.com/dafadey/geomView/archive/refs/heads/main.zip
set GEOMVIEW_ZIP=geomView-main.zip
set GEOMVIEW_INSTALL_DIR=geomView-main

set URLS[0]=%GLEW_URL%
set URLS[1]=%GLFW_URL%
set URLS[2]=%ZLIB_URL%
set URLS[3]=%LIBPNG_URL%
set URLS[4]=%GEOMVIEW_URL%

set ZIPS[0]=%GLEW_ZIP%
set ZIPS[1]=%GLFW_ZIP%
set ZIPS[2]=%ZLIB_ZIP%
set ZIPS[3]=%LIBPNG_ZIP%
set ZIPS[4]=%GEOMVIEW_ZIP%

set DIRS[0]=%GLEW_INSTALL_DIR%
set DIRS[1]=%GLFW_INSTALL_DIR%
set DIRS[2]=%ZLIB_INSTALL_DIR%
set DIRS[3]=%LIBPNG_INSTALL_DIR%
set DIRS[4]=%GEOMVIEW_INSTALL_DIR%

for /l %%i in (0, 1, 4) do (
    
    echo Downloading !ZIPS[%%i]! from !URLS[%%i]!
    powershell -Command "(New-Object Net.WebClient).DownloadFile('!URLS[%%i]!', '!ZIPS[%%i]!')"

    echo Unzip !ZIPS[%%i]!
    powershell -Command "Expand-Archive -Path '!ZIPS[%%i]!' -DestinationPath '%cd%'"

    echo Creating folders...
    if not exist "%cd%/!DIRS[%%i]!/mybuild" mkdir "%cd%/!DIRS[%%i]!/mybuild"
    if not exist "%cd%/!DIRS[%%i]!/myinstall" mkdir "%cd%/!DIRS[%%i]!/myinstall"

    echo Cleaning up...
    del !ZIPS[%%i]!

)

cmake -S "%cd%/%GLEW_INSTALL_DIR%/build/cmake" -B "%cd%/%GLEW_INSTALL_DIR%/mybuild" -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="%cd%/%GLEW_INSTALL_DIR%/myinstall" -DBUILD_UTILS=OFF -DGLEW_OSMESA=OFF -DGLEW_REGAL=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build "%cd%/%GLEW_INSTALL_DIR%/mybuild" --config Release --target INSTALL

cmake -S "%cd%/%GLFW_INSTALL_DIR%" -B "%cd%/%GLFW_INSTALL_DIR%/mybuild" -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="%cd%/%GLFW_INSTALL_DIR%/myinstall" -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_WIN32=ON -DGLFW_INSTALL=ON -DGLFW_USE_HYBRID_HPG=OFF -USE_MSVC_RUNTIME_LIBRARY_DLL=OFF

cmake --build "%cd%/%GLFW_INSTALL_DIR%/mybuild" --config Release --target INSTALL


cmake -S "%cd%/%ZLIB_INSTALL_DIR%" -B "%cd%/%ZLIB_INSTALL_DIR%/mybuild" -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="%cd%/%ZLIB_INSTALL_DIR%/myinstall" -DINSTALL_BIN_DIR=%cd%/%ZLIB_INSTALL_DIR%/myinstall/bin -DINSTALL_INC_DIR=%cd%/%ZLIB_INSTALL_DIR%/myinstall/include -DINSTALL_LIB_DIR=%cd%/%ZLIB_INSTALL_DIR%/myinstall/lib -DINSTALL_MAN_DIR=%cd%/%ZLIB_INSTALL_DIR%/myinstall/share/man -DINSTALL_PKGCONFIG_DIR=%cd%/%ZLIB_INSTALL_DIR%/myinstall/share/pkgconfig -DZLIB_BUILD_EXAMPLES=OFF

cmake --build "%cd%/%ZLIB_INSTALL_DIR%/mybuild" --config Release --target INSTALL

cmake -S "%cd%/%LIBPNG_INSTALL_DIR%" -B "%cd%/%LIBPNG_INSTALL_DIR%/mybuild" -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="%cd%/%LIBPNG_INSTALL_DIR%/myinstall" -DPNG_BUILD_ZLIB=OFF -DPNG_DEBUG=OFF -DPNG_EXECUTABLES=ON -DPNG_HARDWARE_OPTIMIZATIONS=OFF -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DZLIB_ROOT="%cd%/%ZLIB_INSTALL_DIR%/myinstall" -DPNG_TOOLS=ON

cmake --build "%cd%/%LIBPNG_INSTALL_DIR%/mybuild" --config Release --target INSTALL

cmake -S "%cd%/%GEOMVIEW_INSTALL_DIR%" -B "%cd%/%GEOMVIEW_INSTALL_DIR%/mybuild" -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="%cd%/%GEOMVIEW_INSTALL_DIR%/myinstall" -DGLEW_DIR=%cd%/%GLEW_INSTALL_DIR%/myinstall/lib/cmake/glew -DPNG_INCLUDE_DIR=%cd%/%LIBPNG_INSTALL_DIR%/myinstall/include -DPNG_LIBRARY=%cd%/%LIBPNG_INSTALL_DIR%/myinstall/lib/libpng16_static.lib -DZLIB_LIBRARY=%cd%/%ZLIB_INSTALL_DIR%/myinstall/lib/zlibstatic.lib -Dglfw3_DIR=%cd%/%GLFW_INSTALL_DIR%/myinstall/lib/cmake/glfw3 -DCMAKE_CXX_FLAGS=/D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR

cmake --build "%cd%/%GEOMVIEW_INSTALL_DIR%/mybuild" --config Debug --target INSTALL
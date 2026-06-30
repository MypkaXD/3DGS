#!/bin/bash

sdk_dir=${PWD}/../sdk

urls=(
    https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.zip
    https://github.com/glfw/glfw/archive/refs/heads/master.zip
    https://github.com/ocornut/imgui/archive/refs/heads/master.zip
    https://github.com/madler/zlib/releases/download/v1.3.1/zlib131.zip
    https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.47.zip
    https://github.com/dafadey/geomView/archive/refs/heads/main.zip
    https://github.com/g-truc/glm/archive/refs/heads/master.zip
)

names=(
    glew-2.1.0
    glfw-master
    imgui-master
    zlib-1.3.1
    libpng-1.6.47
    geomView-main
    glm-master
)

for index in {0..6}; do
    
    curl -L -o "${sdk_dir}/${names[$index]}.zip" "${urls[$index]}"
    unzip -qq "${sdk_dir}/${names[$index]}" -d "${sdk_dir}"

    rm "${sdk_dir}/${names[$index]}.zip"

    mkdir -p "${sdk_dir}/${names[$index]}/my_build"
    mkdir -p "${sdk_dir}/${names[$index]}/my_install"
    
done

# glew
cmake -S "${sdk_dir}/glew-2.1.0/build/cmake" \
      -B "${sdk_dir}/glew-2.1.0/my_build" \
      -G "Visual Studio 17 2022" -A x64 \
      -DCMAKE_INSTALL_PREFIX="${sdk_dir}/glew-2.1.0/my_install" \
      -DBUILD_UTILS=OFF \
      -DGLEW_OSMESA=OFF \
      -DGLEW_REGAL=OFF \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build "${sdk_dir}/glew-2.1.0/my_build" --config Release --target INSTALL

# glfw
cmake -S "${sdk_dir}/glfw-master" \
      -B "${sdk_dir}/glfw-master/my_build" \
      -G "Visual Studio 17 2022" -A x64 \
      -DCMAKE_INSTALL_PREFIX="${sdk_dir}/glfw-master/my_install" \
      -DBUILD_SHARED_LIBS=OFF \
      -DGLFW_BUILD_DOCS=OFF \
      -DGLFW_BUILD_EXAMPLES=OFF \
      -DGLFW_BUILD_TESTS=OFF \
      -DGLFW_BUILD_WIN32=ON \
      -DGLFW_INSTALL=ON \
      -DGLFW_USE_HYBRID_HPG=OFF \
      -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF

cmake --build "${sdk_dir}/glfw-master/my_build" --config Release --target INSTALL

# zlib
cmake -S "${sdk_dir}/zlib-1.3.1" \
      -B "${sdk_dir}/zlib-1.3.1/my_build" \
      -G "Visual Studio 17 2022" -A x64 \
      -DCMAKE_INSTALL_PREFIX="${sdk_dir}/zlib-1.3.1/my_install" \
      -DINSTALL_BIN_DIR="${sdk_dir}/zlib-1.3.1/my_install/bin" \
      -DINSTALL_INC_DIR="${sdk_dir}/zlib-1.3.1/my_install/include" \
      -DINSTALL_LIB_DIR="${sdk_dir}/zlib-1.3.1/my_install/lib" \
      -DINSTALL_MAN_DIR="${sdk_dir}/zlib-1.3.1/my_install/share/man" \
      -DINSTALL_PKGCONFIG_DIR="${sdk_dir}/zlib-1.3.1/my_install/share/pkgconfig" \
      -DZLIB_BUILD_EXAMPLES=OFF

cmake --build "${sdk_dir}/zlib-1.3.1/my_build" --config Release --target INSTALL

# libpng
cmake -S "${sdk_dir}/libpng-1.6.47" \
      -B "${sdk_dir}/libpng-1.6.47/my_build" \
      -G "Visual Studio 17 2022" -A x64 \
      -DCMAKE_INSTALL_PREFIX="${sdk_dir}/libpng-1.6.47/my_install" \
      -DPNG_BUILD_ZLIB=OFF \
      -DPNG_DEBUG=OFF \
      -DPNG_EXECUTABLES=ON \
      -DPNG_HARDWARE_OPTIMIZATIONS=OFF \
      -DPNG_SHARED=OFF \
      -DPNG_STATIC=ON \
      -DPNG_TESTS=OFF \
      -DZLIB_ROOT="${sdk_dir}/zlib-1.3.1/my_install" \
      -DPNG_TOOLS=ON

cmake --build "${sdk_dir}/libpng-1.6.47/my_build" --config Release --target INSTALL

# geom_view 
cmake -S "${sdk_dir}/geomView-main" \
      -B "${sdk_dir}/geomView-main/my_build" \
      -G "Visual Studio 17 2022" -A x64 \
      -DCMAKE_INSTALL_PREFIX="${sdk_dir}/geomView-main/my_install" \
      -DGLEW_DIR="${sdk_dir}/glew-2.1.0/my_install/lib/cmake/glew" \
      -DPNG_INCLUDE_DIR="${sdk_dir}/libpng-1.6.47/my_install/include" \
      -DPNG_LIBRARY="${sdk_dir}/libpng-1.6.47/my_install/lib/libpng16_static.lib" \
      -DZLIB_LIBRARY="${sdk_dir}/zlib-1.3.1/my_install/lib/zlibstatic.lib" \
      -Dglfw3_DIR="${sdk_dir}/glfw-master/my_install/lib/cmake/glfw3"

cmake --build "${sdk_dir}/geomView-main/my_build" --config Debug --target INSTALL


# glm
cmake -S "${sdk_dir}/glm-master" \
      -B "${sdk_dir}/glm-master/my_build" \
      -G "Visual Studio 17 2022" -A x64 \
      -DCMAKE_INSTALL_PREFIX="${sdk_dir}/glm-master/my_install" \
      -DGLM_BUILD_TESTS=OFF \
      -DBUILD_SHARED_LIBS=OFF

cmake --build "${sdk_dir}/glm-master/my_build" --config Release --target INSTALL
#!/bin/bash
mkdir -p build
pushd build
cmake .. -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix` -DCMAKE_BUILD_TYPE=release -Wno-dev -Wno-deprecated-declarations
popd

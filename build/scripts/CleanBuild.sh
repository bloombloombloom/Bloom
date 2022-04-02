#!/bin/bash

cd /home/nav/Projects/Bloom || exit;

rm -fr build/cmake-build-debug/*; rm -fr build/cmake-build-release/*;

rm -fr release;
rm -fr Bloom-*.deb;
rm -fr Bloom-*.rpm;
rm -fr "_CPack_Packages";

export CMAKE_PREFIX_PATH=/opt/Qt/6.1.2/gcc_64/

cd build/cmake-build-release/ && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/local/install/bin/g++ ../../
cd /home/nav/Projects/Bloom/ && cmake --build /home/nav/Projects/Bloom/build/cmake-build-release --target clean

cmake --build /home/nav/Projects/Bloom/build/cmake-build-release --target Bloom
cmake --install /home/nav/Projects/Bloom/build/cmake-build-release

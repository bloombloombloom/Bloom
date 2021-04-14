#!/bin/bash

cd /home/nav/Projects/Bloom || exit;

rm -fr build/cmake-build-debug/*; rm -fr build/cmake-build-release/*;

rm -fr release;
rm -fr Bloom-*.deb;
rm -fr "_CPack_Packages";

export CMAKE_PREFIX_PATH=/opt/Qt/5.12.10/gcc_64/

cd build/cmake-build-release/ && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++-9 ../../
cd /home/nav/Projects/Bloom/ && cmake --build /home/nav/Projects/Bloom/build/cmake-build-release --target clean

cmake --build /home/nav/Projects/Bloom/build/cmake-build-release --target Bloom
cmake --install /home/nav/Projects/Bloom/build/cmake-build-release --target Bloom
mkdir Build
cd Build
mkdir Lib

mkdir build64_debug
cd build64_debug
rm * -rf

cmake -Wno-dev -DCMAKE_BUILD_TYPE=Debug  -DCMAKE_BUILD_GAME:BOOL=True ../..
make -j4

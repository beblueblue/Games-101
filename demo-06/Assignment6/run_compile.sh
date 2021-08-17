mkdir -p ./build
pushd ./build
source ~/.bashrc
cmake ..
make
echo "***compile***"
./Raytracing
popd
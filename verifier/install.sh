#! /bin/bash

mkdir build && cd build
cmake .. 
cmake --build . --target clam-seadsa && cmake ..
cmake --build . --target clam-seallvm && cmake ..
cmake --build . --target ntt-clam && cmake ..
cmake --build . --target crab && cmake ..
cmake --build . --target install

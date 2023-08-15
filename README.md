# Tsoding_MachineLearning_in_C

to make use of pdflatex in wsl2

1. install texlive-full

to make use of raylib

1. inside /software/raylib directory delete "build" directory and make new "build" directory.

2. cd in to build directory and "cmake -DCMAKE_INSTALL_PREFIX=$HOME/opt/raylib/ .." to build and set installation path.

3. "make -j3" to build with 3 threads.

4. "make install" to install at location we mentioned at 2.
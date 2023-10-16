# Tsoding_MachineLearning_in_C

to make use of pdflatex in wsl2

1. install texlive-full

to make use of raylib

1. inside /software/raylib directory delete "build" directory and make new "build" directory.

2. cd in to build directory and "cmake -DCMAKE_INSTALL_PREFIX=$HOME/opt/raylib/ .." to build and set installation path.

3. when you run above command it will tell what is missing in log , install them.
3.5 "sudo apt-get install libglfw3-dev" 

4. "make -j3" to build with 3 threads.

5. might complain there is no glfw.

6. "make install" to install at location we mentioned at 2.

7. create ~/opt/raylib/raylib.env file and write 
export PKG_CONFIG_PATH=$HOME/opt/raylib/lib/pkgconfig/

8. install pkg-config

9. when building raylib stuff in playground with build.sh apply
source ~/opt/raylib/raylib.env && ./build.sh
# pfs-grpc

C++ wrapper for gRPC

## Dependences
```sh
$ mkdir 3rdparty
$ cd 3rdparty
$ git submodule add https://github.com/grpc/grpc.git grpc
```

## Build
```sh
$ git clone git@github.com:semenovf/pfs-grpc.git
$ cd pfs-grpc
$ git submodule update --init --recursive
$ cd ..
$ mkdir -p builds/pfs-grpc
$ cd builds/pfs-grpc
$ cmake -DCMAKE_BUILD_TYPE=Debug ../../pfs-grpc
$ cmake --build .
```

### Build for Windows
```sh
cmake -G "Visual Studio 14 2015" -DCMAKE_CXX_STANDARD=11 -Wno-dev -A x64 <pfs-grpc source path>
cmake --build .

```
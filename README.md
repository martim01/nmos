# nmos
C++ library to implement AMWA-NMOS

# Introduction

An implementation of the [AMWA NMOS specifications](https://github.com/AMWA-TV/nmos/wiki) in C++. 
The aim is to implement all of the specifications but currently IS-04 and IS-05 are targetted.

## Current Status
### IS-04
- NodeApi - complete. Supports v1.1, v1.2
- RegistryApi - complete. Supports v1.1, v1.2
- QueryApi - No implemented.
- Client - P2P IS-04 discovery complete. IS-04 Target complete. IS-05 single connections complete. Need to implemenet Query

### IS-05
- Complete.



# Dependencies
The library is intended to be platform agnostic and where possible use widely used and public licenced external libraries.
- [microhttpd](https://www.gnu.org/software/libmicrohttpd/) for the webserver
- [libcurl](https://curl.haxx.se/libcurl/) for registering resources and performing queries
- Linux: [avahi](http://avahi.org/) for service discovery and publishing
- Windows: mDNSRepsonder for service discovery and publishing
All other external code is included in the external directory

# Building
There are CodeBlocks project files to build the library and a number of test applications. 
Alternatively there is a CMakeLists.txt file which can be used to build the library (not yet the test applications) on Linux (Windows etc still to come). To use this:
```
cd build
cmake ..
cmake --build .
```






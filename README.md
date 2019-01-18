# nmos
C++ library to implement AMWA-NMOS

# Introduction

An implementation of the [AMWA NMOS specifications](https://github.com/AMWA-TV/nmos/wiki) in C++. 
The aim is to implement all of the specifications but currently IS-04 and IS-05 are targetted.

# Dependencies
The library is intended to be platform agnostic and where possible use widely used and opened licenced external libraries.
- You need a C++11 compatible compiler (tested and known to work with gcc and MinGW)
- microhttpd
- avahi (for linux)
- mDNSRepsonder (for Windows)


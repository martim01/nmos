# nmos
C++ library to implement AMWA-NMOS

# Introduction

An implementation of the [AMWA NMOS specifications](https://github.com/AMWA-TV/nmos/wiki) in C++. 
The aim is to implement all of the specifications but currently IS-04 and IS-05 are targetted.

# Dependencies
The library is intended to be platform agnostic and where possible use widely used and opened licenced external libraries.
- [microhttpd](https://www.gnu.org/software/libmicrohttpd/) for the webserver
- [libcurl](https://curl.haxx.se/libcurl/) for registering resources and performing queries
- Linux: [avahi](http://avahi.org/) for service discovery and publishing
- Linux: [libuuid](https://linux.die.net/man/3/libuuid) to generate the unique identifiers for the resources
- Windows: mDNSRepsonder for service discovery and publishing





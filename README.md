# nmos
C++ library to implement AMWA-NMOS

# Introduction

An implementation of the [AMWA NMOS specifications](https://github.com/AMWA-TV/nmos/wiki) in C++. 
The aim is to implement all of the specifications but currently IS-04 and IS-05 are targetted.

## Current Status
### IS-04
- NodeApi - almost complete (link between control management and IS-05 not fully implemented). Supports v1.1, v1.2, v1.3
- RegistryApi - complete. Supports v1.1, v1.2, v1.3
- QueryApi - Websockets not yet implemented. Paged queries not yet implemented.
- Client - P2P IS-04 discovery complete. IS-04 Target complete. IS-05 single connections complete. Need to implemenet Query

### IS-05
- Bulk connections not yet implemented.



# Dependencies
The library is intended to be platform agnostic and where possible use widely used and public licenced external libraries.
- [microhttpd](https://www.gnu.org/software/libmicrohttpd/) for the webserver
- [libcurl](https://curl.haxx.se/libcurl/) for registering resources and performing queries
- Linux: [avahi](http://avahi.org/) for service discovery and publishing
- Linux: [libuuid](https://linux.die.net/man/3/libuuid) to generate the unique identifiers for the resources
- Windows: mDNSRepsonder for service discovery and publishing





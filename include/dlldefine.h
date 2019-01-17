#pragma once

#ifdef __GNUWIN32__
    #ifdef NMOS_EXPORT
        #define NMOS_EXPOSE __declspec(dllexport)
    #else
        #define NMOS_EXPOSE __declspec(dllimport)
    #endif // NMOS_EXPORT
#else
#define NMOS_EXPOSE
#endif

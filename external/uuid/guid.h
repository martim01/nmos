#pragma once

#include "copyrt.h"
#undef uuid_t

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    unsigned long  time_low;
    unsigned short  time_mid;
    unsigned short  time_hi_and_version;
    unsigned char   clock_seq_hi_and_reserved;
    unsigned char   clock_seq_low;
    unsigned char    node[6];
} uuid_t;



/* uuid_create_md5_from_name -- create a version 3 (MD5) UUID using a
   "name" from a "name space" */
void uuid_create_md5_from_name(uuid_t *uuid,         /* resulting UUID */
    uuid_t nsid,          /* UUID of the namespace */
    const char name[],           /* the name from which to generate a UUID */
    int namelen           /* the length of the name */
);

void format_uuid_v3or5(uuid_t *uuid, unsigned char hash[16], int v);


#ifdef __cplusplus
}
#endif // __cplusplus

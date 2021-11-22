#pragma once
#include <string>
#include <array>


namespace pml
{
    namespace nmos
    {
        enum enumConnection { SENDER_STAGED=0,
                                SENDER_ACTIVE,
                                SENDER_TRANSPORTFILE,
                                SENDER_CONSTRAINTS,
                                RECEIVER_STAGED,
                                RECEIVER_ACTIVE,
                                RECEIVER_CONSTRAINTS,
                                SENDER_PATCH,
                                RECEIVER_PATCH,
                                TARGET};


    };
};

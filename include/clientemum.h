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
                                RECEIVER_STAGED,
                                RECEIVER_ACTIVE,
                                SENDER_PATCH,
                                RECEIVER_PATCH,
                                TARGET};


    };
};

#pragma once

#include "ITimerChannel.h"

namespace TeensyTimerTool
{
    class ITimerModule
    {
     public:
        virtual ITimerChannel* getChannel() = 0;
        virtual void setPriority(){};
        virtual void setPrescale(){};
    };
};
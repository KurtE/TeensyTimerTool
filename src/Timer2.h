#include "config.h"

namespace TeensyTimerTool
{
    class Timer2
    {
     public:
        Timer2(ITimerModule* _module)
            : module(_module), timerChannel(nullptr)
        {
            Serial.printf("timer2 constr %p\n", module);
        }

        inline errorCode beginPeriodic(callback_t cb, uint32_t period)
        {
            Serial.printf("timer2 begin  %p\n", cb);
            if (timerChannel == nullptr) timerChannel = module->getChannel();

            if (timerChannel != nullptr)
            {
                Serial.printf("timer2 chan:  %p\n", timerChannel);
                timerChannel->begin(cb, period, true);
                return errorCode::OK;
            }
            Serial.println("no channel");
            return errorCode::noFreeChannel;
        }

     protected:
        ITimerModule* module;
        ITimerChannel* timerChannel;
    };

    // IMPLEMENTATION =======================================================

};
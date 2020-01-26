#include "config.h"

#include "Teensy/TMR2/TMR2.h"

namespace TeensyTimerTool
{
    ITimerModule* TMR2_1 = new TMR2_t<0>();
    ITimerModule* TMR2_2 = new TMR2_t<2>();

    ITimerModule* ttPool[]{TMR2_1, TMR2_1};
};

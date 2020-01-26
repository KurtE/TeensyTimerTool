#pragma once

#include "../../ITimerModule.h"
#include "Tmr2Channel.h"
#include "imxrt.h"

namespace TeensyTimerTool
{
    template <unsigned mNr>
    class TMR2_t : public ITimerModule
    {
     public:
        ITimerChannel* getChannel() override;

     protected:
        static Tmr2Channel channels[4];
        static bool isInitialized;
        static void isr();

        // the following constants are calculated at compile time
        static_assert(mNr < 4, "Module number < 4 required");
        static constexpr IRQ_NUMBER_t irq = mNr == 0 ? IRQ_QTIMER1 : mNr == 1 ? IRQ_QTIMER2 : mNr == 2 ? IRQ_QTIMER3 : IRQ_QTIMER4;
        static constexpr IMXRT_TMR_t* pTMR = mNr == 0 ? &IMXRT_TMR1 : mNr == 1 ? &IMXRT_TMR2 : mNr == 2 ? &IMXRT_TMR3 : &IMXRT_TMR4;
    };

    // IMPLEMENTATION ======================================

    template <unsigned m>
    ITimerChannel* TMR2_t<m>::getChannel()
    {
        if (!isInitialized)  //first time we touch registers (don't grab module if not requested)
        {
            for (unsigned chNr = 0; chNr < 4; chNr++)
            {
                channels[chNr].initialize();
            }
            attachInterruptVector(irq, isr);
            NVIC_ENABLE_IRQ(irq);
            isInitialized = true;
        }

        for (unsigned chNr = 0; chNr < 4; chNr++)
        {
            Tmr2Channel* channel = &channels[chNr];
            if (!channel->isActive)
            {
                channel->isActive = true;
                Serial.printf("activate %d %d\n", chNr, channel->isActive);
                return channel;
            }
        }
        return nullptr;
    };

    template <unsigned m>
    void TMR2_t<m>::isr()
    {
        for (unsigned i = 0; i < 4; i++)
        {
            channels[i].isr();
        }
        asm volatile("dsb");
    }

    template <unsigned m>
    bool TMR2_t<m>::isInitialized = false;

    template <unsigned m>
    Tmr2Channel TMR2_t<m>::channels[4]{Tmr2Channel(&pTMR->CH[0]), Tmr2Channel(&pTMR->CH[1]), Tmr2Channel(&pTMR->CH[2]), Tmr2Channel(&pTMR->CH[3])};
}
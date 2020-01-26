#pragma once

#include "../../ITimerChannel.h"
#include "imxrt.h"
#include "Arduino.h"

namespace TeensyTimerTool
{
    class Tmr2Channel : public ITimerChannel
    {
     public:
        inline Tmr2Channel(IMXRT_TMR_CH_t* regs);

        inline void initialize()
        {
            regs->CTRL = 0;  // stop timer
        }

        void begin(callback_t cb, uint32_t period, bool periodic) override
        {
            double t = period * (150.0 / 128.0);
            uint16_t reload = t > 0xFFFF ? 0xFFFF : (uint16_t)t - 1;
            Serial.printf("chan begin %p %d", regs, reload);

            regs->CTRL = 0x0000;
            regs->LOAD = 0x0000;
            regs->COMP1 = reload;
            regs->CMPLD1 = reload;
            regs->CNTR = 0x0000;
            regs->CSCTRL &= ~TMR_CSCTRL_TCF1;
            regs->CSCTRL |= TMR_CSCTRL_TCF1EN;

            if (periodic)
                regs->CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(0b1111) | TMR_CTRL_LENGTH;
            else
                regs->CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(0b1111) | TMR_CTRL_ONCE | TMR_CTRL_LENGTH;

            callback = cb;
        }

        inline void isr()
        {
            digitalWriteFast(1, HIGH);
            if (callback != nullptr && regs->CSCTRL & TMR_CSCTRL_TCF1)
            {
                 regs->CSCTRL &= ~TMR_CSCTRL_TCF1;
                 callback();
                 regs->CSCTRL = regs->CSCTRL;
            }
            digitalWriteFast(1, LOW);
        }

        void trigger(uint32_t delay) override
        {}

        bool isActive = false;

     protected:
        IMXRT_TMR_CH_t* regs;
        callback_t callback = nullptr;
    };

    // IMPLEMENTATION ==============================================

    Tmr2Channel::Tmr2Channel(IMXRT_TMR_CH_t* regs)
        : ITimerChannel(nullptr)
    {
        this->regs = regs;
    }

}
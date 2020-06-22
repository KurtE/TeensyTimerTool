#pragma once

#include "Arduino.h"

#include "../../ITimerChannel.h"
#include "ErrorHandling/error_codes.h"
#include "core_pins.h"

namespace TeensyTimerTool
{
    class TCK_t;

#if !defined(ARDUINO_TEENSYLC) // T-LC doesn't have a cycle counter, nees special treatment

    class TckChannel : public ITimerChannel
    {
     public:
        inline TckChannel() { triggered = false; }
        inline virtual ~TckChannel(){}; //TBD

        inline errorCode begin(callback_t cb, uint32_t period, bool periodic) override;
        inline errorCode start() override;
        inline errorCode stop() override;
        inline errorCode trigger(uint32_t delay_us) override;

        inline errorCode setPeriod(uint32_t microSeconds) override;
        inline errorCode setCurrentPeriod(uint32_t microSeconds) override;
        inline errorCode setNextPeriod(uint32_t microSeconds) override;

        inline float getMaxPeriod();

     protected:
        inline bool tick();

        callback_t callback;
        uint32_t startCNT, currentPeriod, nextPeriod;
        bool triggered;
        bool periodic;

        bool block = false;

        static inline uint32_t microsecondToCPUCycles(const uint32_t microSecond);
        static inline uint32_t CPUCyclesToMicroseond(const uint32_t cpuCycles);

        inline errorCode _setCurrentPeriod(const uint32_t period);
        inline void _setNextPeriod(const uint32_t period);

        friend TCK_t;
    };

    // IMPLEMENTATION ==============================================

    errorCode TckChannel::begin(callback_t cb, uint32_t period, bool periodic)
    {
        this->triggered = false;
        this->periodic = periodic;
        this->currentPeriod = microsecondToCPUCycles(period);

        this->nextPeriod = this->currentPeriod;
        this->callback = cb;

        this->startCNT = ARM_DWT_CYCCNT;

        return errorCode::OK;
    }

    errorCode TckChannel::start()
    {
        this->startCNT = ARM_DWT_CYCCNT;
        this->triggered = true;
        return errorCode::OK;
    }

    errorCode TckChannel::stop()
    {
        this->triggered = false;
        return errorCode::OK;
    }

    errorCode TckChannel::trigger(uint32_t delay) // µs
    {
        this->startCNT = ARM_DWT_CYCCNT;
        this->nextPeriod = microsecondToCPUCycles(delay);
        this->currentPeriod = this->nextPeriod - 68; //??? compensating for cycles spent computing?

        this->triggered = true;

        return errorCode::OK;
    }

    errorCode TckChannel::setPeriod(uint32_t microSeconds)
    {
        const uint32_t period = microsecondToCPUCycles(microSeconds);

        this->_setNextPeriod(period);
        return this->_setCurrentPeriod(period);
    }

    errorCode TckChannel::setCurrentPeriod(uint32_t microSeconds)
    {
        const uint32_t period = microsecondToCPUCycles(microSeconds);
        return this->_setCurrentPeriod(period);
    }

    errorCode TckChannel::setNextPeriod(uint32_t microSeconds)
    {
        const uint32_t period = microsecondToCPUCycles(microSeconds);
        this->_setNextPeriod(period);
        return errorCode::OK;
    }

    float TckChannel::getMaxPeriod()
    {
        return CPUCyclesToMicroseond(0xFFFF'FFFF);
    }

    bool TckChannel::tick()
    {
        static bool lock = false;

        if (!lock && this->currentPeriod != 0 && this->triggered && (ARM_DWT_CYCCNT - this->startCNT) >= this->currentPeriod)
        {
            lock = true;
            this->startCNT = ARM_DWT_CYCCNT;
            this->triggered = this->periodic; // i.e., stays triggerd if periodic, stops if oneShot
            callback();
            lock = false;
            return true;
        } else
        {
            return false;
        }
    }

    uint32_t TckChannel::microsecondToCPUCycles(const uint32_t microSecond)
    {
        return microSecond * (F_CPU / 1'000'000);
    }

    uint32_t TckChannel::CPUCyclesToMicroseond(const uint32_t cpuCycles)
    {
        return (1'000'000.0f / F_CPU) * cpuCycles;
    }

    void TckChannel::_setNextPeriod(const uint32_t period)
    {
        this->nextPeriod = period;
    }

    errorCode TckChannel::_setCurrentPeriod(const uint32_t period)
    {
        this->currentPeriod = period;
        const bool hasTicked = this->tick();

        if (hasTicked)
        {
            return errorCode::triggeredLate;
        } else
        {
            return errorCode::OK;
        }
    }

#else

    class TckChannel : public ITimerChannel
    {
     public:
        inline TckChannel() { triggered = false; }
        inline virtual ~TckChannel(){};

        inline errorCode begin(callback_t cb, uint32_t period, bool periodic)
        {
            triggered = false;
            this->periodic = periodic;
            this->period = period;
            this->callback = cb;

            startCNT = micros();

            return errorCode::OK;
        }

        inline void start()
        {
            this->startCNT = micros();
            this->triggered = true;
        }

        inline errorCode stop()
        {
            this->triggered = false;
            return errorCode::OK;
        }

        inline errorCode setPeriod(uint32_t microSeconds) override;
        inline uint32_t getPeriod(void);

        inline errorCode trigger(uint32_t delay) // µs
        {
            this->startCNT = micros();
            this->period = delay;
            this->triggered = true;
            return errorCode::OK;
        }

     protected:
        uint32_t startCNT, period;
        callback_t callback;
        bool triggered;
        bool periodic;

        inline void tick();
        bool block = false;

        friend TCK_t;
    };

    // IMPLEMENTATION ==============================================

    void TckChannel::tick()
    {
        static bool lock = false;

        if (!lock && period != 0 && triggered && (micros() - startCNT) >= period)
        {
            lock = true;
            startCNT = micros();
            triggered = periodic; // i.e., stays triggerd if periodic, stops if oneShot
            callback();
            lock = false;
        }
    }
    errorCode TckChannel::setPeriod(uint32_t microSeconds)
    {
        period = microSeconds;
        return errorCode::OK;
    }
    uint32_t TckChannel::getPeriod()
    {
        return period;
    }

#endif

} // namespace TeensyTimerTool

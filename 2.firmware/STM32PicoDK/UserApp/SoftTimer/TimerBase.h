//
// Created by moon on 2023/11/10.
//

#ifndef STM32PICODK_TIMERBASE_H
#define STM32PICODK_TIMERBASE_H

#include "stdint.h"

class TimerBase {
public:
    typedef void (*Callback)(void *data);

    enum Mode {
        OnceMode = 0,     //!< Timer once run
        CirculateMode    //!< Timer cycle run
    };

public:
    bool       isStarted; //!< Timer run status
    bool       isDynamic; //!< Dynamic memory
    uint32_t   time;      //!< Current timestamp
    uint32_t   timeLeft;  //!< Left callback time
    const char *name;     //!< Timer name
    uint32_t   period;    //!< Timer period
    Callback   callback;  //!< Timer IRQ callback function
    void       *data;     //!< User defined data object pointer
    Mode       mode;      //!< Timer run mode

public:
    explicit TimerBase(const char *name_, uint32_t period_,
                       Callback callback_, void *data_ = nullptr, Mode mode_ = OnceMode) :
            name(name_), period(period_), callback(callback_), data(data_), mode(mode_) {

        isDynamic = false;
    }

    ~TimerBase() {}

    void start(uint32_t current_time);
    void stop();

    uint32_t calcTimeLeft(uint32_t current_time);
};

#endif //STM32PICODK_TIMERBASE_H

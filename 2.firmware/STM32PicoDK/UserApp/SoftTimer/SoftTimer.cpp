//
// Created by moon on 2023/11/10.
//

#include "SoftTimer.h"
#include <cstring>
#include "algorithm"

void SoftTimer::add(TimerBase *timer) {
    if (isExists(timer))
        return;

    timer->isDynamic = false;
    list.push_back(timer);
}

TimerBase *SoftTimer::create(const char *name, uint32_t period, TimerBase::Callback callback,
                             void *data, TimerBase::Mode mode) {
    if (isExists(name))
        return nullptr;

    TimerBase *timer = nullptr;

        timer = new TimerBase(name, period, callback, data, mode);
        timer->isDynamic = true;
        list.push_back(timer);


    return timer;
}

TimerBase *SoftTimer::isExists(TimerBase *timer) {
    auto item = std::find(list.begin(), list.end(), timer);

    if (item != list.end()) {
        return *item;
    }

    return nullptr;
}

TimerBase *SoftTimer::isExists(const char *name) {
//    if (list.empty())
//        return nullptr;
//
//    for (auto &timer: list) {
//        if (strcmp(timer->name, name) == 0) {
//            return timer;
//        }
//    }

    auto item = std::find_if(list.begin(), list.end(), [name](TimerBase *timer) {
        return timer->name == name;
    });

    if (item != list.end()) {
        return *item;
    }

    return nullptr;
}

void SoftTimer::remove(TimerBase *timer) {
    TimerBase *t = isExists(timer);

    if (t != nullptr && t->isDynamic) {
        delete t;
    }
}

void SoftTimer::remove(const char *name) {
    TimerBase *t = isExists(name);

    if (t != nullptr && t->isDynamic) {
        delete t;
    }
}

void SoftTimer::run() {
    if (list.empty() || !getTimestamp) {
        return;
    }

    uint32_t time = getTimestamp();

    for (auto &timer: list) {
        if (timer->isStarted && timer->calcTimeLeft(time) <= 0) {
            timer->callback((timer->data == nullptr) ? timer : timer->data);
            timer->time = time;

            if (timer->mode == TimerBase::OnceMode)
                timer->stop();
        }
    }
}

uint32_t SoftTimer::freeTime() {
    uint32_t freeTime = UINT32_MAX;

    for (auto &timer: list) {
        if (!timer->isStarted) {
            continue;
        }

        uint32_t time = timer->timeLeft;
        if (time < freeTime)
            freeTime = time;
    }

    return (freeTime == UINT32_MAX) ? 0 : freeTime;
}

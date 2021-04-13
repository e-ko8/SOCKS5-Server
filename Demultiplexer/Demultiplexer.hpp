#pragma once

#include <sys/types.h>
#include <sys/event.h>
#include <ctime>
#include <stdexcept>
#include <unistd.h>

enum class TimerCoeffs {SecondsCoeff = 1000, MinutesCoeff = SecondsCoeff * 60, HoursCoeff = MinutesCoeff * 60};

class Demultiplexer
{

public:

    Demultiplexer();
    Demultiplexer(Demultiplexer&& other) noexcept;
    Demultiplexer(const Demultiplexer& other) = delete;

    Demultiplexer& operator=(Demultiplexer&& other) noexcept;
    Demultiplexer& operator=(const Demultiplexer& other) = delete;

    struct kevent WaitEvent();

    void WaitForReadEvent(u_long desc);
    void WaitForWriteEvent(u_long desc);

    void StopReadWaiting(u_long desc);
    void StopWriteWaiting(u_long desc);

    void StopAllEventsWaiting(u_long desc);

    void StartTimer(u_long desc, u_long duration, TimerCoeffs coeff);
    void StopTimer(u_long desc);

    [[nodiscard]] int GetKqueueId() const;

    ~Demultiplexer();

private:

    int id;

    struct kevent event{};
    struct kevent new_event{};
    struct timespec timeout{};
};



#pragma once

#include <sys/types.h>
#include <sys/event.h>
#include <ctime>
#include <stdexcept>
#include <unistd.h>

class Demultiplexer
{

public:

    Demultiplexer();

    struct kevent WaitEvent();

    void WaitForReadEvent(int desc);
    void WaitForWriteEvent(int desc);

    void StopReadWaiting(int desc);
    void StopWriteWaiting(int desc);

    void StopAllEventsWaiting(int desc);

    void StartTimer(int desc, int duration);
    void StopTimer(int desc);

    ~Demultiplexer();

private:

    int id;

    struct kevent event;
    struct kevent new_event;
    struct timespec timeout;
};



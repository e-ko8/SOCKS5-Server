#include "Demultiplexer.hpp"

Demultiplexer::Demultiplexer()  :  id{kqueue()}, event{}, new_event{}, timeout{1,0}
{
    if (id < 0)
    {
        throw std::runtime_error("Can't create kqueue instance...");
    }
}

void Demultiplexer::WaitForReadEvent(int desc)
{
    EV_SET(&event, desc, EVFILT_READ, EV_ADD, 0, 0, nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

void Demultiplexer::WaitForWriteEvent(int desc)
{
    EV_SET(&event, desc, EVFILT_WRITE, EV_ADD, 0, 0, nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

void Demultiplexer::StopReadWaiting(int desc)
{
    EV_SET(&event, desc, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

void Demultiplexer::StopWriteWaiting(int desc)
{
    EV_SET(&event, desc, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

void Demultiplexer::StopAllEventsWaiting(int desc)
{
    StopReadWaiting(desc);
    StopWriteWaiting(desc);
    StopTimer(desc);
}

void Demultiplexer::StartTimer(int desc, int duration)
{
    EV_SET(&event, desc, EVFILT_TIMER, EV_ADD, 0, (duration), nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

void Demultiplexer::StopTimer(int desc)
{
    EV_SET(&event, desc, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

Demultiplexer::~Demultiplexer()
{
    if (id>=0)
    {
        close(id);
    }
}

struct kevent Demultiplexer::WaitEvent()
{
    while(kevent(id, nullptr, 0, &new_event, 1, &timeout) < 1);
    return new_event;
}


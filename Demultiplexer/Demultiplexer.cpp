#include "Demultiplexer.hpp"
#include <iostream>

Demultiplexer::Demultiplexer()  :  id{kqueue()}, event{}, new_event{}, timeout{1,0}
{
    if (id < 0)
    {
        throw std::runtime_error("Can't create kqueue instance...");
    }
}

void Demultiplexer::WaitForReadEvent(u_long desc)
{
    EV_SET(&event, desc, EVFILT_READ, EV_ADD, 0, 0, nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

void Demultiplexer::WaitForWriteEvent(u_long desc)
{
    EV_SET(&event, desc, EVFILT_WRITE, EV_ADD, 0, 0, nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

void Demultiplexer::StopReadWaiting(u_long desc)
{
    EV_SET(&event, desc, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

void Demultiplexer::StopWriteWaiting(u_long desc)
{
    EV_SET(&event, desc, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

void Demultiplexer::StopAllEventsWaiting(u_long desc)
{
    StopReadWaiting(desc);
    StopWriteWaiting(desc);
    StopTimer(desc);
}

void Demultiplexer::StartTimer(u_long desc, u_long duration, TimerCoeffs coeff)
{
    EV_SET(&event, desc, EVFILT_TIMER, EV_ADD, 0, (duration * static_cast<int>(coeff)), nullptr);
    kevent(id, &event, 1, nullptr, 0, nullptr);
}

void Demultiplexer::StopTimer(u_long desc)
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
    kevent(id, nullptr, 0, &new_event, 1, &timeout);
    return new_event;
}

Demultiplexer::Demultiplexer(Demultiplexer &&other) noexcept
{
    id = other.id;
    other.id = -1;

    event = other.event;
    other.event = {};

    new_event = other.new_event;
    other.new_event = {};

    timeout = other.timeout;
    other.timeout = {};
}

Demultiplexer &Demultiplexer::operator=(Demultiplexer &&other) noexcept
{
    if(this!=&other)
    {
        id = other.id;
        other.id = -1;

        event = other.event;
        other.event = {};

        new_event = other.new_event;
        other.new_event = {};

        timeout = other.timeout;
        other.timeout = {};
    }

    return *this;
}

int Demultiplexer::GetKqueueId() const
{
    return id;
}


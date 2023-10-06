#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include "Core.hpp"
#include "Common.hpp"
#include "IEventHandler.hpp"

class Signal : public IEventHandler {
public:
	virtual void EventHandler(struct kevent &currentEvent);
	void RegisterSignalsWithKqueue(void);
	void UnregisterSignalsWithKqueue(void);

private:
	static void signalHandler(int signal);
};


#endif
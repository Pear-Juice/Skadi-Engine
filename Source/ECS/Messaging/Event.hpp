#ifndef EVENT_HPP
#define EVENT_HPP

#include "Lambda.hpp"

template <class ... Props>
class Event {
public:
	Event(){};

	void add() {

	}

	void call(Props ... props) {
		cheapCall(props...);
	}

private:
	// std::vector<CheapCall> cheapCalls;

};

#endif //EVENT_HPP

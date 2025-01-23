#ifndef EVENT_HPP
#define EVENT_HPP

#include "Lambda.hpp"

template <typename T>
class Event {
public:
	Event() = default;

	void add(std::function<T> lambda) {
		calls.push_back(lambda);
	}

	void addOnce(std::function<T> lambda) {
		onceCalls.push_back(lambda);
	}

	bool remove() {
		if (calls.empty()) return false;

		calls.pop_back();
		return true;
	}

	template<class ...Props>
	void call(Props&& ...props) {
		for (std::function<T> lambda : calls) {
			lambda(props...);
		}

		for (std::function<T> lambda : onceCalls) {
			lambda(props...);
		}

		onceCalls.clear();
	}

private:
	std::vector<std::function<T>> calls;
	std::vector<std::function<T>> onceCalls;

};

#endif //EVENT_HPP

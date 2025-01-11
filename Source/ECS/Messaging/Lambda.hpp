#ifndef LAMBDA_HPP
#define LAMBDA_HPP

#include <functional>

template <class ...Props>
class Lambda {
	typedef void (*Call)(Props ...);

	Call callback;
public:
	Lambda(const Call callback) : callback(callback) {}

	void call(Props ... props) {
		callback(props...);
	}
};

#endif //LAMBDA_HPP

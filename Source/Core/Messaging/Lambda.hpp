#ifndef LAMBDA_HPP
#define LAMBDA_HPP

#include <functional>
#include <iostream>

template <typename T>
///Can store functions references or std::functions. Limitations are that it cannot be copied or assigned without a value
class Lambda {
	using Call = const T&;

	const Call callback;
public:
	Lambda(const T& callback) : callback(callback) {}

	template<class ...Props>
	void call(Props&&... props) {
		callback(props...);
	}
};



#endif //LAMBDA_HPP

#pragma once
#include <cassert>

template <typename T>
class optional
{
	private:
		T value;
		bool value_valid;
		
		bool is_initialized() const {return value_valid;}
		
	public:
		optional() : value(T()), value_valid(false) {}
		optional(const T newvalue) : value(newvalue), value_valid(true) {}
		
		operator bool() const {return is_initialized();}
		
		T get() {assert(is_initialized());return value;}
		const T get() const {assert(is_initialized());return value;}
		T get_or_default(T thedefault) {return is_initialized()?get():thedefault;}
};

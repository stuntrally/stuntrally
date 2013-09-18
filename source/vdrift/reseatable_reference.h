#pragma once
#include <cassert>
#include <cstdlib>

template <typename T>
class reseatable_reference
{
	private:
		T * ptr;
		
		bool valid() const {return ptr;}
		
	public:
		reseatable_reference() : ptr(NULL) {}
		reseatable_reference(T & ref) : ptr(&ref) {}
		
		//default copy and assignment are OK
		
		operator bool() const {return valid();}
		
		T & get() {assert(valid());return *ptr;}
		const T & get() const {assert(valid());return *ptr;}
		T & operator*() {return get();}
		const T & operator*() const {return get();}
		T * operator->() {assert(valid());return ptr;}
		const T * operator->() const {assert(valid());return ptr;}
		
		void set(T & ref) {ptr = &ref;}
		reseatable_reference <T> & operator=(T & other)
		{
			set(other);
			return *this;
		}
		
		reseatable_reference <T> & operator=(T * other)
		{
			assert(other);
			set(*other);
			return *this;
		}
		
		void clear() {ptr = NULL;}
};

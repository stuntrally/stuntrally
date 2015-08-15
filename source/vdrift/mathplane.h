#pragma once
//#include <vector>
//#include <cassert>
//#include <cmath>
//#include <iostream>
//#include <cstring>
//#include <sstream>

#include "mathvector.h"

template <class T>
class MATHPLANE
{
	private:
		struct MATHPLANE_ABCD
		{
			T a,b,c,d;
			inline T & operator[](const int i) { return ((T*)this)[i]; }
			inline const T & operator[](const int i) const { return ((T*)this)[i]; }

			MATHPLANE_ABCD() : a(0),b(1),c(0),d(0) {}
			MATHPLANE_ABCD(const T na, const T nb, const T nc, const T nd) : a(na),b(nb),c(nc),d(nd) {}
		} v;
	
	public:
		MATHPLANE()
		{
		}
		
		MATHPLANE(const T a, const T b, const T c, const T d) : v(a,b,c,d)
		{
		}
		
		MATHPLANE(const MATHPLANE <T> & other)
		{
			std::memcpy(&v,&other.v,sizeof(MATHPLANE_ABCD)); //high performance, but portability issues?
		}
		
		template <typename T2>
		MATHPLANE (const MATHPLANE <T2> & other)
		{
			*this = other;
		}
		
		inline void Set(const T a, const T b, const T c, const T d)
		{
			v = MATHPLANE_ABCD(a,b,c,d);
		}
	
		///careful, there's no way to check the bounds of the array
		inline void Set(const T * array_pointer)
		{
			std::memcpy(&v,array_pointer,sizeof(MATHPLANE_ABCD)); //high performance, but portability issues?
		}
	
		inline const T & operator[](const int n) const
		{
			assert(n < 4);
			return v[n];
		}
	
		inline T & operator[](const int n)
		{
			assert(n < 4);
			return v[n];
		}
	
		/*///scalar multiplication
		MATHVECTOR<T,3> operator * (const T & scalar) const
		{
			return MATHVECTOR<T,3> (v.x*scalar,v.y*scalar,v.z*scalar);
		}
	
		///scalar division
		MATHVECTOR<T,3> operator / (const T & scalar) const
		{
			assert(scalar != 0);
			T invscalar = 1.0/scalar;
			return (*this)*invscalar;
		}
	
		MATHVECTOR<T,3> operator + (const MATHVECTOR<T,3> & other) const
		{
			return MATHVECTOR<T,3> (v.x+other.v.x,v.y+other.v.y,v.z+other.v.z);
		}
	
		MATHVECTOR<T,3> operator - (const MATHVECTOR<T,3> & other) const
		{
			return MATHVECTOR<T,3> (v.x-other.v.x,v.y-other.v.y,v.z-other.v.z);;
		}*/
	
		template <typename T2>
		const MATHPLANE <T> & operator = (const MATHPLANE <T2> & other)
		{
			v.a = other[0];
			v.b = other[1];
			v.c = other[2];
			v.d = other[3];
			
			return *this;
		}
	
		template <typename T2>
		inline bool operator== (const MATHPLANE <T2> & other) const
		{
			return (v.a == other[0] && v.b == other[1] && v.c == other[2] && v.d == other[3]);
		}
	
		template <typename T2>
		inline bool operator!= (const MATHPLANE <T2> & other) const
		{
			return !(*this == other);
		}
	
		/*///inversion
		MATHVECTOR<T,3> operator-() const
		{
			return MATHVECTOR<T,3> (-v.x, -v.y, -v.z);
		}*/
		
		T DistanceToPoint(const MATHVECTOR<T,3> & point) const
		{
			T abcsq = v.a*v.a+v.b*v.b+v.c*v.c;
			assert(abcsq != 0);
			return (v.a*point[0]+v.b*point[1]+v.c*point[2]+v.d)/sqrt(abcsq);
		}
};

template <typename T>
std::ostream & operator << (std::ostream &os, const MATHPLANE <T> & v)
{
	for (size_t i = 0; i < 3; ++i)
	{
		os << v[i] << ", ";
	}
	os << v[3];// << std::endl;
	return os;
}

#pragma once
#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
#include <cstring>
#include <sstream>


template <typename T, unsigned int dimension>
class MATHVECTOR
{
private:
	T v[dimension];
	
public:
	typedef size_t size_type;
	
	MATHVECTOR()
	{
		for (size_type i = 0; i < dimension; ++i)
			v[i] = 0;
	}
	
	MATHVECTOR(const T& t)
	{
		for (size_type i = 0; i < dimension; ++i)
			v[i] = t;
	}
	MATHVECTOR(const MATHVECTOR<T, dimension> & other)
	{
		*this = other;
	}
	MATHVECTOR(const T x, const T y)
	{
		assert(dimension==2);
		v[0] = x;
		v[1] = y;
	}
		
	const T Magnitude() const
	{
		return sqrt(MagnitudeSquared());
	}
	const T MagnitudeSquared() const
	{
		T running_total(0);
		for (size_type i = 0; i < dimension; ++i)
			running_total += v[i] * v[i];
		return running_total;
	}
	
	///set all vector values to val1
	void Set(const T & val1)
	{
		//assert(dimension == 1);
		for (size_type i = 0; i < dimension; ++i)
			v[i] = val1;
	}
	void Set(const T & val1, const T & val2)
	{
		assert(dimension == 2);
		v[0] = val1;
		v[1] = val2;
	}
	void Set(const T & val1, const T & val2, const T & val3)
	{
		assert(dimension == 3);
		v[0] = val1;
		v[1] = val2;
		v[2] = val3;
	}
	
	///careful, there's no way to check the bounds of the array
	void Set(const T * array_pointer)
	{
		for (size_t i = 0; i < dimension; ++i)
			v[i] = array_pointer[i];
	}
	
	///return a normalized vector
	MATHVECTOR<T, dimension> Normalize() const
	{
		MATHVECTOR<T, dimension> output;
		
		const T mag = (Magnitude());
		
		assert(mag != 0);
		
		for (size_type i = 0; i < dimension; ++i)
		{
			output[i] = v[i]/mag;
		}
		
		return output;
	}
	
	///return the scalar dot product between this and other
	const T dot(const MATHVECTOR<T, dimension> & other) const
	{
		T output(0);
		for (size_type i = 0; i < dimension; ++i)
		{
			output += v[i]*other.v[i];
		}
		return output;
	}
	
	///return the cross product between this vector and the given vector
	const MATHVECTOR<T,3> cross(const MATHVECTOR<T,3> & other) const
	{
		assert(dimension==3);
		
		MATHVECTOR<T,3> output;
		output[0] = v[1]*other.v[2] - v[2]*other.v[1];
		output[1] = v[2]*other.v[0] - v[0]*other.v[2];
		output[2] = v[0]*other.v[1] - v[1]*other.v[0];
		return output;
	}
	
	///return the reflection of this vector around the given normal (must be unit length)
	const MATHVECTOR<T, dimension> reflect(const MATHVECTOR<T, dimension> & other) const
	{
		MATHVECTOR<T, dimension> output;
		
		output = (*this)-other*T(2.0)*other.dot(*this);
		
		return output;
	}
	
	const T & operator[](size_type n) const
	{
		assert(n < dimension);
		return v[n];
	}
	
	T & operator[](size_type n)
	{
		assert(n < dimension);
		return v[n];
	}
	
	///scalar multiplication
	MATHVECTOR<T, dimension> operator * (const T & scalar) const
	{
		MATHVECTOR<T, dimension> output;
		
		for (size_type i = 0; i < dimension; ++i)
		{
			output[i] = v[i]*scalar;
		}
		
		return output;
	}
	
	///scalar division
	MATHVECTOR<T, dimension> operator / (const T & scalar) const
	{
		assert(scalar != 0);
		
		MATHVECTOR<T, dimension> output;
		
		for (size_type i = 0; i < dimension; ++i)
		{
			output[i] = v[i]/scalar;
		}
		
		return output;
	}
	
	MATHVECTOR<T, dimension> operator + (const MATHVECTOR<T, dimension> & other) const
	{
		MATHVECTOR<T, dimension> output;
		
		for (size_type i = 0; i < dimension; ++i)
		{
			output[i] = v[i] + other.v[i];
		}
		
		return output;
	}
	
	MATHVECTOR<T, dimension> operator - (const MATHVECTOR<T, dimension> & other) const
	{
		MATHVECTOR<T, dimension> output;
		
		for (size_type i = 0; i < dimension; ++i)
		{
			output[i] = v[i] - other.v[i];
		}
		
		return output;
	}
	
	template <typename T2>
	const MATHVECTOR<T, dimension> & operator = (const MATHVECTOR<T2, dimension> & other)
	{
		for (size_type i = 0; i < dimension; ++i)
			v[i] = other[i];
		
		return *this;
	}
	
	template <typename T2>
	bool operator== (const MATHVECTOR<T2, dimension> & other) const
	{
		bool same(true);
		
		for (size_type i = 0; i < dimension; ++i)
		{
			same = same && (v[i] == other.v[i]);
		}
		
		return same;
	}
	
	template <typename T2>
	bool operator!= (const MATHVECTOR<T2, dimension> & other) const
	{
		return !(*this == other);
	}
	
	///inversion
	MATHVECTOR<T, dimension> operator-() const
	{
		MATHVECTOR<T, dimension> output;
		for (size_type i = 0; i < dimension; ++i)
		{
			output.v[i] = -v[i];
		}
		return output;
	}

};

///we need a faster mathvector for 3-space, so specialize
template <class T>
class MATHVECTOR<T,3>
{
	private:
		struct MATHVECTOR_XYZ
		{
			T x,y,z;
			inline T & operator[](const int i) { return ((T*)this)[i]; }
			inline const T & operator[](const int i) const { return ((T*)this)[i]; }

			MATHVECTOR_XYZ() : x(0), y(0), z(0) {}
			MATHVECTOR_XYZ(const T& t) : x(t), y(t), z(t) {}
			MATHVECTOR_XYZ(const T nx, const T ny, const T nz) : x(nx),y(ny),z(nz) {}
		} v;
	
	public:
		MATHVECTOR()
		{
		}
	
		MATHVECTOR(const T& t) : v(t)
		{
		}
		
		MATHVECTOR(const T x, const T y, const T z) : v(x,y,z)
		{
		}
		
		MATHVECTOR(const MATHVECTOR<T,3> & other)
		{
			std::memcpy(&v,&other.v,sizeof(MATHVECTOR_XYZ)); //high performance, but portability issues?
			/*v.x = other.v.x;
			v.y = other.v.y;
			v.z = other.v.z;*/
		}
		
		template <typename T2>
		MATHVECTOR (const MATHVECTOR<T2,3> & other)
		{
			*this = other;
		}
		
		inline const T Magnitude() const
		{
			return sqrt(MagnitudeSquared());
		}
		inline const T MagnitudeSquared() const
		{
			return v.x*v.x+v.y*v.y+v.z*v.z;
		}
	
		///set all vector values to val1
		inline void Set(const T val1)
		{
			v.x = v.y = v.z = val1;
		}

		inline void Set(const T val1, const T val2, const T val3)
		{
			v.x = val1;
			v.y = val2;
			v.z = val3;
		}
	
		///careful, there's no way to check the bounds of the array
		inline void Set(const T * array_pointer)
		{
			std::memcpy(&v,array_pointer,sizeof(MATHVECTOR_XYZ)); //high performance, but portability issues?
			/*v.x = array_pointer[0];
			v.y = array_pointer[1];
			v.z = array_pointer[2];*/
		}
	
		///return a normalized vector
		MATHVECTOR<T,3> Normalize() const
		{
			const T mag = Magnitude();
			assert(mag != 0);
			const T maginv = (1.0/mag);
		
			return MATHVECTOR<T,3> (v.x*maginv, v.y*maginv, v.z*maginv);
		}
	
		///return the scalar dot product between this and other
		inline const T dot(const MATHVECTOR<T,3> & other) const
		{
			return v.x*other.v.x+v.y*other.v.y+v.z*other.v.z;
		}
	
		///return the cross product between this vector and the given vector
		const MATHVECTOR<T,3> cross(const MATHVECTOR<T,3> & other) const
		{
			return MATHVECTOR<T,3> (v[1]*other.v[2] - v[2]*other.v[1],
					v[2]*other.v[0] - v[0]*other.v[2],
					v[0]*other.v[1] - v[1]*other.v[0]);
		}
	
		///return the reflection of this vector around the given normal (must be unit length)
		const MATHVECTOR<T,3> reflect(const MATHVECTOR<T,3> & other) const
		{
			return (*this)-other*T(2.0)*other.dot(*this);
		}
	
		inline const T & operator[](const int n) const
		{
			assert(n < 3);
			return v[n];
		}
	
		inline T & operator[](const int n)
		{
			assert(n < 3);
			return v[n];
		}
	
		///scalar multiplication
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
		}
	
		template <typename T2>
		const MATHVECTOR<T,3> & operator = (const MATHVECTOR<T2,3> & other)
		{
			v.x = other[0];
			v.y = other[1];
			v.z = other[2];
			
			return *this;
		}
	
		template <typename T2>
		inline bool operator== (const MATHVECTOR<T2,3> & other) const
		{
			//return (std::memcmp(&v,&other.v,sizeof(MATHVECTOR_XYZ)) == 0);
			return (v.x == other[0] && v.y == other[1] && v.z == other[2]);
		}
	
		template <typename T2>
		inline bool operator!= (const MATHVECTOR<T2,3> & other) const
		{
			return !(*this == other);
		}
	
		///inversion
		MATHVECTOR<T,3> operator-() const
		{
			return MATHVECTOR<T,3> (-v.x, -v.y, -v.z);
		}
		
		///set all vector components to be positive
		inline void absify()
		{
			/*v.x = v.x > 0 ? v.x : -v.x;
			v.y = v.y > 0 ? v.y : -v.y;
			v.z = v.z > 0 ? v.z : -v.z;*/
			v.x = fabs(v.x);
			v.y = fabs(v.y);
			v.z = fabs(v.z);
		}
		
		///project this vector onto the vector 'vec'.  neither needs to be a unit vector
		MATHVECTOR<T,3> project(const MATHVECTOR<T,3> & vec) const
		{
			T scalar_projection = dot(vec.Normalize());
			return vec.Normalize() * scalar_projection;
		}
};

template <typename T, unsigned int dimension>
std::ostream & operator << (std::ostream &os, const MATHVECTOR<T, dimension> & v)
{
	for (size_t i = 0; i < dimension-1; ++i)
	{
		os << v[i] << ", ";
	}
	os << v[dimension-1];// << std::endl;
	return os;
}

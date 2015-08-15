#pragma once
//#include <iostream>
//#include <cstring>

template <typename T>
class MATRIX4
{
	private:
		typedef size_t size_type;
		T data[16];
	
	public:
		MATRIX4() {LoadIdentity();}
		MATRIX4(const MATRIX4 <T> & other) {Set(other);}
		const MATRIX4 & operator=(const MATRIX4 <T> & other) {Set(other); return *this;}
		
		const T & operator[](size_type n) const
		{
			assert(n < 16);
			return data[n];
		}
	
		T & operator[](size_type n)
		{
			assert(n < 16);
			return data[n];
		}
		
		void DebugPrint(std::ostream & out) const
		{
			for (size_type i = 0; i < 16; ++i)
			{
				out << i << ". " << data[i] << std::endl;
			}
		}
		
		MATRIX4 <T> Multiply(const MATRIX4 <T> & other) const
		{
			MATRIX4 out;
	
			for (int i = 0, i4 = 0; i < 4; ++i,i4+=4)
			{
				for (int j = 0; j < 4; j++)
				{
					out.data[i4+j] = 0;
			
					for (int k = 0, k4 = 0; k < 4; k ++,k4+=4)
						out.data[i4+j] += data[i4+k]*other.data[k4+j];
				}
			}
	
			return out;
		}
		
		void LoadIdentity()
		{
			data[0] = data[5] = data[10] = data[15] = 1;
			data[1] = data[2] = data[3] = data[4] = data[6] = data[7] = data[8] = data[9] = data[11] = data[12] = data[13] = data[14] = 0;
		}
		
		inline void Set(const T * newdata)
		{
			std::memcpy(data,newdata,sizeof(T)*16); //high performance, but portability issues?
			/*for (int i = 0; i < 16; ++i)
				data[i] = newdata[i];*/
		}
		
		inline void Set(const MATRIX4 <T> & other) {Set(other.data);}
		
		void Translate(const float tx, const float ty, const float tz)
		{
			data[12] += tx;
			data[13] += ty;
			data[14] += tz;
		}
		
		bool operator==(const MATRIX4 <T> & other) {return Equals(other);}
		
		bool Equals(const MATRIX4 <T> & other)
		{
			return (memcmp(data,other.data,16*sizeof(T)) == 0); //high performance, but portability issues?
			/*for (int i = 0; i < 16; ++i)
				if (data[i] != other.data[i])
					return false;
			
			return true;*/
		}
		
		void TransformVectorIn(float & x, float & y, float & z) const
		{
			float outx = x * data[0] + y * data[1] + z * data[2];
			float outy = x * data[4] + y * data[5] + z * data[6];
			float outz = x * data[8] + y * data[9] + z * data[10];
			
			x = outx;
			y = outy;
			z = outz;
		}
		
		void TransformVectorOut(float & x, float & y, float & z) const
		{
			float outx = x * data[0] + y * data[4] + z * data[8] + data[12];
			float outy = x * data[1] + y * data[5] + z * data[9] + data[13];
			float outz = x * data[2] + y * data[6] + z * data[10] + data[14];
			
			x = outx;
			y = outy;
			z = outz;
		}
		
		void Scale(T scalar)
		{
			MATRIX4 <T> scalemat;
			scalemat.data[15] = 1;
			scalemat.data[0] = scalemat.data[5] = scalemat.data[10] = scalar;
			scalemat.data[1] = scalemat.data[2] = scalemat.data[3] = scalemat.data[4] = scalemat.data[6] = scalemat.data[7] = scalemat.data[8] = scalemat.data[9] = scalemat.data[11] = scalemat.data[12] = scalemat.data[13] = scalemat.data[14] = 0;
			*this = Multiply(scalemat);
		}
		
		const T * GetArray() const {return data;}
		
		static MATRIX4<T> Perspective(T fovy, T aspect, T znear, T zfar)
		{
			MATRIX4<T> mat;
			
			const T pi = 3.14159265;
			T f = 1.0 / tan(0.5 * fovy * pi / 180.0);
			mat[0] = f / aspect; mat[1] = 0; mat[2] = 0; mat[3] = 0;				
			mat[4] = 0; mat[5] = f; mat[6] = 0; mat[7] = 0;
			mat[8] = 0; mat[9] = 0; mat[10] = (zfar + znear) / (znear - zfar); mat[11] = -1;
			mat[12] = 0; mat[13] = 0; mat[14] = 2 * zfar * znear / (znear - zfar); mat[15] = 0;

			return mat;
		}
		
		static MATRIX4<T> InvPerspective(T fovy, T aspect, T znear, T zfar)
		{
			MATRIX4<T> mat;
			
			const T pi = 3.14159265;
			T f = 1.0 / tan(0.5 * fovy * pi / 180.0);
			mat[0] = aspect / f; mat[1] = 0; mat[2] = 0; mat[3] = 0;				
			mat[4] = 0; mat[5] = 1 / f; mat[6] = 0; mat[7] = 0;
			mat[8] = 0; mat[9] = 0; mat[10] = 0; mat[11] = (znear - zfar) / (2 * zfar * znear);
			mat[12] = 0; mat[13] = 0; mat[14] = -1; mat[15] = (zfar + znear) / (2 * zfar * znear);

			return mat;
		}
		
		MATRIX4<T> Inverse()
		{
			MATRIX4<T> Inv;
			
			T A0 = data[ 0]*data[ 5] - data[ 1]*data[ 4];
			T A1 = data[ 0]*data[ 6] - data[ 2]*data[ 4];
			T A2 = data[ 0]*data[ 7] - data[ 3]*data[ 4];
			T A3 = data[ 1]*data[ 6] - data[ 2]*data[ 5];
			T A4 = data[ 1]*data[ 7] - data[ 3]*data[ 5];
			T A5 = data[ 2]*data[ 7] - data[ 3]*data[ 6];
			T B0 = data[ 8]*data[13] - data[ 9]*data[12];
			T B1 = data[ 8]*data[14] - data[10]*data[12];
			T B2 = data[ 8]*data[15] - data[11]*data[12];
			T B3 = data[ 9]*data[14] - data[10]*data[13];
			T B4 = data[ 9]*data[15] - data[11]*data[13];
			T B5 = data[10]*data[15] - data[11]*data[14];
			
			T Det = A0*B5 - A1*B4 + A2*B3 + A3*B2 - A4*B1 + A5*B0;
			assert (fabs(Det) > 1e-10); //matrix inversion failed
			
			Inv[ 0] = + data[ 5]*B5 - data[ 6]*B4 + data[ 7]*B3;
			Inv[ 4] = - data[ 4]*B5 + data[ 6]*B2 - data[ 7]*B1;
			Inv[ 8] = + data[ 4]*B4 - data[ 5]*B2 + data[ 7]*B0;
			Inv[12] = - data[ 4]*B3 + data[ 5]*B1 - data[ 6]*B0;
			Inv[ 1] = - data[ 1]*B5 + data[ 2]*B4 - data[ 3]*B3;
			Inv[ 5] = + data[ 0]*B5 - data[ 2]*B2 + data[ 3]*B1;
			Inv[ 9] = - data[ 0]*B4 + data[ 1]*B2 - data[ 3]*B0;
			Inv[13] = + data[ 0]*B3 - data[ 1]*B1 + data[ 2]*B0;
			Inv[ 2] = + data[13]*A5 - data[14]*A4 + data[15]*A3;
			Inv[ 6] = - data[12]*A5 + data[14]*A2 - data[15]*A1;
			Inv[10] = + data[12]*A4 - data[13]*A2 + data[15]*A0;
			Inv[14] = - data[12]*A3 + data[13]*A1 - data[14]*A0;
			Inv[ 3] = - data[ 9]*A5 + data[10]*A4 - data[11]*A3;
			Inv[ 7] = + data[ 8]*A5 - data[10]*A2 + data[11]*A1;
			Inv[11] = - data[ 8]*A4 + data[ 9]*A2 - data[11]*A0;
			Inv[15] = + data[ 8]*A3 - data[ 9]*A1 + data[10]*A0;

			T InvDet = 1.0 / Det;
			for (int i = 0; i < 16; ++i) Inv[i] *= InvDet;
						
			return Inv;
		}
};

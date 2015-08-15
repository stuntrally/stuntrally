#pragma once
//#include <iostream>
#include "mathvector.h"
//#include <cstring>

template <typename T>
class MATRIX3
{
	private:
		typedef size_t size_type;
		T data[9];
		
		T myfabs(const T & val) const
		{
			if (val < 0)
				return -val;
			return val;
		}
	
	public:
		MATRIX3() {LoadIdentity();}
		MATRIX3(const MATRIX3 <T> & other) {Set(other);}
		const MATRIX3 & operator=(const MATRIX3 <T> & other) {Set(other); return *this;}
		
		const T & operator[](size_type n) const
		{
			assert(n < 9);
			return data[n];
		}
	
		T & operator[](size_type n)
		{
			assert(n < 9);
			return data[n];
		}
		
		void DebugPrint(std::ostream & out) const
		{
			for (size_type i = 0; i < 9; ++i)
			{
				out << i << ". " << data[i] << std::endl;
			}
		}
		
		MATRIX3 <T> Multiply(const MATRIX3 <T> & other) const
		{
			MATRIX3 out;
	
			for (int i = 0, i3 = 0; i < 3; ++i,i3+=3)
			{
				for (int j = 0; j < 3; j++)
				{
					out.data[i3+j] = 0;
			
					for (int k = 0, k3 = 0; k < 3; k ++,k3+=3)
						out.data[i3+j] += data[i3+k]*other.data[k3+j];
				}
			}
	
			return out;
		}
		
		void LoadIdentity()
		{
			data[0] = data[4] = data[8] = 1;
			data[1] = data[2] = data[3] = data[5] = data[6] = data[7] = 0;
		}
		
		void Set(const T * newdata)
		{
			//std::memcpy(data,newdata,sizeof(T)*9); //high performance, but portability issues?
			for (int i = 0; i < 9; ++i)
				data[i] = newdata[i];
		}
		
		void Set(const MATRIX3 <T> & other) {Set(other.data);}
		
		bool operator==(const MATRIX3 <T> & other) {return Equals(other);}
		
		bool Equals(const MATRIX3 <T> & other)
		{
			//return (memcmp(data,other.data,9*sizeof(T)) == 0); //high performance, but portability issues?
			for (int i = 0; i < 9; ++i)
				if (data[i] != other.data[i])
					return false;
			
			return true;
		}
		
		void Scale(T scalar)
		{
			MATRIX3 <T> scalemat;
			scalemat.data[0] = scalemat.data[4] = scalemat.data[8] = scalar;
			scalemat.data[1] = scalemat.data[2] = scalemat.data[3] = scalemat.data[5] = scalemat.data[6] = scalemat.data[7] = 0;
			*this = Multiply(scalemat);
		}
		
		const T * GetArray() const {return data;}
		
		MATHVECTOR<T,3> Multiply(const MATHVECTOR<T,3> & v) const
		{
			MATHVECTOR<T,3> output;
			output.Set(v[0]*data[0] + v[1]*data[3] + v[2]*data[6],
				   v[0]*data[1] + v[1]*data[4] + v[2]*data[7], 
       				   v[0]*data[2] + v[1]*data[5] + v[2]*data[8]);
			return output;
		}
		
		MATRIX3 <T> Inverse() const
		{
			T a = data[0];//
			T b = data[1];
			T c = data[2];
			T d = data[3];
			T e = data[4];//
			T f = data[5];
			T g = data[6];
			T h = data[7];
			T i = data[8];//
			T Div = -c*e*g+b*f*g+c*d*h-a*f*h-b*d*i+a*e*i;
			const T EPSILON(1e-10);
			//std::cout << "Div: " << Div << std::endl;
			//DebugPrint(std::cout);
			assert (myfabs(Div) > EPSILON); //matrix inversion failed
			
			T invdiv = 1.0 / Div;
			
			MATRIX3 m;
			m.data[0] = -f*h+e*i;
			m.data[1] = c*h-b*i;
			m.data[2] = -c*e+b*f;
			m.data[3] = f*g-d*i;
			m.data[4] = -c*g+a*i;
			m.data[5] = c*d-a*f;
			m.data[6] = -e*g+d*h;
			m.data[7] = b*g-a*h;
			m.data[8] = -b*d+a*e;
			
			for (int i = 0; i < 9; ++i)
				m.data[i] *= invdiv;
			
			return m;
		}
		
		MATRIX3 <T> Transpose() const
		{
			//0 1 2
			//3 4 5
			//6 7 8
			
			MATRIX3 <T> out;
			out[0] = data[0];
			out[1] = data[3];
			out[2] = data[6];
			out[3] = data[1];
			out[4] = data[4];
			out[5] = data[7];
			out[6] = data[2];
			out[7] = data[5];
			out[8] = data[8];
			
			return out;
		}

		// returns false on error
		static bool Diagonalize(MATRIX3<T>& m, MATRIX3<T>& v, MATHVECTOR<T,3>& w)
		{
			double sm[3][3], ev[3][3], ew[3];
			for(int i = 0; i < 3; ++i)
			{
				for(int j = 0; j < 3; j++)
				{
					sm[i][j] = m[i*3 + j];
				}
			}
			
			if(MATRIX3<T>::dsyevj3(sm, ev, ew)) return false;
			
			for(int i = 0; i < 3; ++i)
			{
				for(int j = 0; j < 3; j++)
				{
					v[i*3 + j] = ev[i][j];
				}
			}
			
			w.Set(ew);
			return true;
		}

		// from "Efficient numerical diagonalization of hermitian 3x3 matrices"
		// ----------------------------------------------------------------------------
		private: static int dsyevj3(double A[3][3], double Q[3][3], double w[3])
		// ----------------------------------------------------------------------------
		// Calculates the eigenvalues and normalized eigenvectors of a symmetric 3x3
		// matrix A using the Jacobi algorithm.
		// The upper triangular part of A is destroyed during the calculation,
		// the diagonal elements are read but not destroyed, and the lower
		// triangular elements are not referenced at all.
		// ----------------------------------------------------------------------------
		// Parameters:
		//   A: The symmetric input matrix
		//   Q: Storage buffer for eigenvectors
		//   w: Storage buffer for eigenvalues
		// ----------------------------------------------------------------------------
		// Return value:
		//   0: Success
		//  -1: Error (no convergence)
		// ----------------------------------------------------------------------------
		{
		  const int n = 3;
		  double sd, so;                  // Sums of diagonal resp. off-diagonal elements
		  double s, c, t;                 // sin(phi), cos(phi), tan(phi) and temporary storage
		  double g, h, z, theta;          // More temporary storage
		  double thresh;
		  
		  // Initialize Q to the identitity matrix
		  for (int i=0; i < n; ++i)
		  {
			Q[i][i] = 1.0;
			for (int j=0; j < i; j++)
			  Q[i][j] = Q[j][i] = 0.0;
		  }

		  // Initialize w to diag(A)
		  for (int i=0; i < n; ++i)
			w[i] = A[i][i];

		  // Calculate tr(A)^2 
		  sd = 0.0;
		  for (int i=0; i < n; ++i)
			sd += fabs(w[i]);
		  sd = sd * sd;
		 
		  // Main iteration loop
		  for (int nIter=0; nIter < 50; nIter++)
		  {
			// Test for convergence 
			so = 0.0;
			for (int p=0; p < n; p++)
			  for (int q=p+1; q < n; q++)
				so += fabs(A[p][q]);
			if (so == 0.0)
			  return 0;

			if (nIter < 4)
			  thresh = 0.2 * so / (n * n);
			else
			  thresh = 0.0;

			// Do sweep
			for (int p=0; p < n; p++)
			  for (int q=p+1; q < n; q++)
			  {
				g = 100.0 * fabs(A[p][q]);
				if (nIter > 4  &&  fabs(w[p]) + g == fabs(w[p])
							   &&  fabs(w[q]) + g == fabs(w[q]))
				{
				  A[p][q] = 0.0;
				}
				else if (fabs(A[p][q]) > thresh)
				{
				  // Calculate Jacobi transformation
				  h = w[q] - w[p];
				  if (fabs(h) + g == fabs(h))
				  {
					t = A[p][q] / h;
				  }
				  else
				  {
					theta = 0.5 * h / A[p][q];
					if (theta < 0.0)
					  t = -1.0 / (sqrt(1.0 + theta * theta) - theta);
					else
					  t = 1.0 / (sqrt(1.0 + theta * theta) + theta);
				  }
				  c = 1.0/sqrt(1.0 + t * t);
				  s = t * c;
				  z = t * A[p][q];

				  // Apply Jacobi transformation
				  A[p][q] = 0.0;
				  w[p] -= z;
				  w[q] += z;
				  for (int r=0; r < p; r++)
				  {
					t = A[r][p];
					A[r][p] = c*t - s*A[r][q];
					A[r][q] = s*t + c*A[r][q];
				  }
				  for (int r=p+1; r < q; r++)
				  {
					t = A[p][r];
					A[p][r] = c*t - s*A[r][q];
					A[r][q] = s*t + c*A[r][q];
				  }
				  for (int r=q+1; r < n; r++)
				  {
					t = A[p][r];
					A[p][r] = c*t - s*A[q][r];
					A[q][r] = s*t + c*A[q][r];
				  }

				  // Update eigenvectors
				  for (int r=0; r < n; r++)
				  {
					t = Q[r][p];
					Q[r][p] = c*t - s*Q[r][q];
					Q[r][q] = s*t + c*Q[r][q];
				  }
				}
			  }
		  }
		  return -1;
		}
};

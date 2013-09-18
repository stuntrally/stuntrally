#pragma once

#ifdef __BIG_ENDIAN__
	#define ENDIAN_SWAP_16(A)  ((((uint16_t)(A) & 0xff00) >> 8) | \
		(((uint16_t)(A) & 0x00ff) << 8))
	#define ENDIAN_SWAP_32(A)  ((((uint32_t)(A) & 0xff000000) >> 24) | \
		(((uint32_t)(A) & 0x00ff0000) >> 8)  | \
		(((uint32_t)(A) & 0x0000ff00) << 8)  | \
		(((uint32_t)(A) & 0x000000ff) << 24))
	#define ENDIAN_SWAP_64(x) (((_int64)(ntohl((int)((x << 32) >> 32))) << 32) | (unsigned int)ntohl(((int)(x >> 32))))
	#define ENDIAN_SWAP_FLOAT(A)  LoadLEFloat(&(A))
	inline float LoadLEFloat ( float *f )
	{
		#define __stwbrx( value, base, index ) \
			__asm__ ( "stwbrx %0, %1, %2" :  : "r" (value), "b%" (index), "r" (base) : "memory" )

		union
		{
			long            i;
			float           f;
		} transfer;

		//load the float into the integer unit
		unsigned int    temp = ( ( long* ) f ) [0];

		//store it to the transfer union, with byteswapping
		__stwbrx ( temp,  &transfer.i, 0 );

		//load it into the FPU and return it
		return transfer.f;
	}
#else
	#define ENDIAN_SWAP_16(A)  (A)
	#define ENDIAN_SWAP_32(A)  (A)
	#define ENDIAN_SWAP_64(A)  (A)
	#define ENDIAN_SWAP_FLOAT(A)  (A)
#endif

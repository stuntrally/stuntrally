#include "pch.h"
#include <math.h>

#include "vertexarray.h"

#define PI 3.14159265




double sinD(double degrees)
{
	return (double) sin(degrees * PI / 180.000f);
}

double cosD(double degrees)
{
	return (double) cos(degrees * PI / 180.000f);
}






void mesh_gen_tire(VERTEXARRAY *tire, float sectionWidth_mm, float aspectRatio, float rimDiameter_in)
{
	// configurable parameters - set to defaults
	unsigned int segmentsAround = 32;
	float innerRadius = 0.65f;
	float innerWidth = 0.60f;

	float sidewallRadius = 0.75f;
	float sidewallBulge = 0.05f;		// bulge from tread

	float shoulderRadius = 0.95f;
	float shoulderBulge = 0.05f;		// bulge from tread

	float treadRadius = 1.00f;
	float treadWidth = 0.60f;



	// use this to build a tire based on the tire code passed to this function
	// if the code under this if doesn't happen, then the parameters are meaningless
	if (true)
	{
		// use function parameters - comment out this section
		float sectionWidth_m = sectionWidth_mm / 1000.0f;
		float rimRadius_m = rimDiameter_in * 0.0254f;


		innerRadius = rimRadius_m;
		treadWidth = sectionWidth_m - shoulderBulge;


		// aspect ratio is a little strange, but i didn't make this stuff up
		// en.wikipedia.org/wiki/Tire_code
		// auto.howstuffworks.com/tire2.htm
		if (aspectRatio <= 0)	// aspect ratio ommitted default to 82%
		{
		    aspectRatio = 82.0f;
		}



		////////////////////////////////////////////////////////
		// this isn't iso standard, but it is sorta important
		// math says a percent should be less than 1
		// but the tire code has it as a whole number
		// could use a uint, but floats are always best for 3d
		if (aspectRatio > 1)    // then he gave
		{
		    aspectRatio = aspectRatio / 100.0f;
		}


		// if he gave us a large number specifically over 200 we need to thing that its in mm
		if (aspectRatio > 2)		// he gave us a mm diameter for the entire tire
		{
		    treadRadius = aspectRatio / 2000.0f;
		}
		else		// otherwise: he gave us the normal percent
		{
		    treadRadius = sectionWidth_m * aspectRatio + rimRadius_m;

		}


		// fudge some values by what looks good.
		innerWidth = treadWidth;

		sidewallBulge = innerWidth * 0.075f;
		shoulderBulge = treadWidth * 0.100f;

		float totalSidewallHeight = sectionWidth_m * aspectRatio;
		sidewallRadius = totalSidewallHeight/3.0f + rimRadius_m;
		shoulderRadius = totalSidewallHeight/1.1f + rimRadius_m;
	}








	// non-configurable parameters
	unsigned int vertexRings = 8;
	float angleIncrement = 360.0f / (float) segmentsAround;

	/////////////////////////////////////
	//
	// vertexes (temporary data)
	//
	unsigned int vertexCount = segmentsAround * vertexRings;
	unsigned int vertexFloatCount = vertexCount * 3;	// * 3 cause there are 3 floats in a vertex
	float *vertexData = new float [ vertexFloatCount ];

	// a re-used for loop variable
	unsigned int lv;

	// Right-side, Inner Ring
	for (lv=0 ; lv<segmentsAround ; lv++)
	{
		float *x = &vertexData[lv+segmentsAround * 0 + 0];
		float *y = &vertexData[lv+segmentsAround * 0 + 1];
		float *z = &vertexData[lv+segmentsAround * 0 + 2];

		*x = innerWidth / 2.0f;
		*y = innerRadius * cosD(angleIncrement * lv);
		*z = innerRadius * sinD(angleIncrement * lv);
	}
	// Right-side, Sidewall Ring
	for (lv=0 ; lv<segmentsAround ; lv++)
	{
		float *x = &vertexData[lv+segmentsAround * 1 + 0];
		float *y = &vertexData[lv+segmentsAround * 1 + 1];
		float *z = &vertexData[lv+segmentsAround * 1 + 2];

		*x = treadWidth / 2.0f + sidewallBulge;
		*y = sidewallRadius * cosD(angleIncrement * lv);
		*z = sidewallRadius * sinD(angleIncrement * lv);
	}
	// Right-side, Shoulder Ring
	for (lv=0 ; lv<segmentsAround ; lv++)
	{
		float *x = &vertexData[lv+segmentsAround * 2 + 0];
		float *y = &vertexData[lv+segmentsAround * 2 + 1];
		float *z = &vertexData[lv+segmentsAround * 2 + 2];

		*x = treadWidth / 2.0f + shoulderBulge;
		*y = shoulderRadius * cosD(angleIncrement * lv);
		*z = shoulderRadius * sinD(angleIncrement * lv);
	}
	// Right-side, Tread Ring
	for (lv=0 ; lv<segmentsAround ; lv++)
	{
		float *x = &vertexData[lv+segmentsAround * 3 + 0];
		float *y = &vertexData[lv+segmentsAround * 3 + 1];
		float *z = &vertexData[lv+segmentsAround * 3 + 2];

		*x = treadWidth / 2.0f;
		*y = treadRadius * cosD(angleIncrement * lv);
		*z = treadRadius * sinD(angleIncrement * lv);
	}


	// Left-side, Tread Ring
	for (lv=0 ; lv<segmentsAround ; lv++)
	{
		float *x = &vertexData[lv+segmentsAround * 4 + 0];
		float *y = &vertexData[lv+segmentsAround * 4 + 1];
		float *z = &vertexData[lv+segmentsAround * 4 + 2];

		*x = -1.0f * treadWidth / 2.0f;
		*y = treadRadius * cosD(angleIncrement * lv);
		*z = treadRadius * sinD(angleIncrement * lv);
	}
	// Left-side, Shoulder Ring
	for (lv=0 ; lv<segmentsAround ; lv++)
	{
		float *x = &vertexData[lv+segmentsAround * 5 + 0];
		float *y = &vertexData[lv+segmentsAround * 5 + 1];
		float *z = &vertexData[lv+segmentsAround * 5 + 2];

		*x = -1.0f * treadWidth / 2.0f + shoulderBulge;
		*y = shoulderRadius * cosD(angleIncrement * lv);
		*z = shoulderRadius * sinD(angleIncrement * lv);
	}
	// Left-side, Sidewall Ring
	for (lv=0 ; lv<segmentsAround ; lv++)
	{
		float *x = &vertexData[lv+segmentsAround * 6 + 0];
		float *y = &vertexData[lv+segmentsAround * 6 + 1];
		float *z = &vertexData[lv+segmentsAround * 6 + 2];

		*x = -1.0f * treadWidth / 2.0f + sidewallBulge;
		*y = sidewallRadius * cosD(angleIncrement * lv);
		*z = sidewallRadius * sinD(angleIncrement * lv);
	}
	// Left-side, Inner Ring
	for (lv=0 ; lv<segmentsAround ; lv++)
	{
		float *x = &vertexData[lv+segmentsAround * 7 + 0];
		float *y = &vertexData[lv+segmentsAround * 7 + 1];
		float *z = &vertexData[lv+segmentsAround * 7 + 2];

		*x = -1.0f * innerWidth / 2.0f;
		*y = innerRadius * cosD(angleIncrement * lv);
		*z = innerRadius * sinD(angleIncrement * lv);
	}





	/////////////////////////////////////
	//
	//  now lets build triangles
	//
	unsigned int triVIndexCount = 2 * segmentsAround * (vertexRings-1) * 3;	// 2 * triangles make a square,   * 3 indexes in a triangle
	unsigned int *triData = new unsigned int [ triVIndexCount ];

	unsigned int triIndex = 0;
	unsigned int circleSegment = 0;
	
	unsigned int *triVIndex0;
	unsigned int *triVIndex1;
	unsigned int *triVIndex2;

	for (circleSegment=0 ; circleSegment<segmentsAround; circleSegment++)
	{
		if (circleSegment == segmentsAround-1)
		{
			// 1st triangle (Right-side - Inner to Sidewall)
			triVIndex0 = &triData[ (triIndex+0)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+0)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+0)*3 + 2 ];
			*triVIndex0 = circleSegment;
			*triVIndex1 = circleSegment + segmentsAround;
			*triVIndex2 = 0;
			// 2nd triangle
			triVIndex0 = &triData[ (triIndex+1)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+1)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+1)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround;
			*triVIndex1 = 0+segmentsAround;
			*triVIndex2 = 0;

			// 3rd triangle (Right-side - Sidewall to shoulder)
			triVIndex0 = &triData[ (triIndex+2)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+2)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+2)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*1;
			*triVIndex1 = circleSegment+segmentsAround*2;
			*triVIndex2 = 0 + segmentsAround*1;
			// 4th triangle
			triVIndex0 = &triData[ (triIndex+3)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+3)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+3)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*2;
			*triVIndex1 = 0+segmentsAround*2;
			*triVIndex2 = 0+segmentsAround*1;


			// 5th triangle (Right-side - Shoulder to Tread)
			triVIndex0 = &triData[ (triIndex+4)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+4)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+4)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*2;
			*triVIndex1 = circleSegment+segmentsAround*3;
			*triVIndex2 = 0 + segmentsAround*2;
			// 6th triangle
			triVIndex0 = &triData[ (triIndex+5)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+5)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+5)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*3;
			*triVIndex1 = 0+segmentsAround*3;
			*triVIndex2 = 0+segmentsAround*2;

			//////////////////////////////////////////////////////////////
			// 7th triangle (Right-side Tread to Left-side Tread)
			triVIndex0 = &triData[ (triIndex+6)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+6)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+6)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*3;
			*triVIndex1 = circleSegment+segmentsAround*4;
			*triVIndex2 = 0 + segmentsAround*3;
			// 8th triangle
			triVIndex0 = &triData[ (triIndex+7)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+7)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+7)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*4;
			*triVIndex1 = 0+segmentsAround*4;
			*triVIndex2 = 0+segmentsAround*3;
			/////////////////////////////////////////////////////////////


			// 9th triangle (Left-side - Tread to Shoulder)
			triVIndex0 = &triData[ (triIndex+8)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+8)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+8)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*4;
			*triVIndex1 = circleSegment+segmentsAround*5;
			*triVIndex2 = 0 + segmentsAround*4;
			// 10th triangle
			triVIndex0 = &triData[ (triIndex+9)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+9)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+9)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*5;
			*triVIndex1 = 0+segmentsAround*5;
			*triVIndex2 = 0+segmentsAround*4;


			// 11th triangle (Left-side - Shoulder to Sidewall)
			triVIndex0 = &triData[ (triIndex+10)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+10)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+10)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*5;
			*triVIndex1 = circleSegment+segmentsAround*6;
			*triVIndex2 = 0 + segmentsAround*5;
			// 12th triangle
			triVIndex0 = &triData[ (triIndex+11)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+11)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+11)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*6;
			*triVIndex1 = 0+segmentsAround*6;
			*triVIndex2 = 0+segmentsAround*5;

			// 13th triangle (Left-side - Sidewall to Inner)
			triVIndex0 = &triData[ (triIndex+12)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+12)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+12)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*6;
			*triVIndex1 = circleSegment+segmentsAround*7;
			*triVIndex2 = 0 + segmentsAround*6;
			// 14th triangle
			triVIndex0 = &triData[ (triIndex+13)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+13)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+13)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*7;
			*triVIndex1 = 0+segmentsAround*7;
			*triVIndex2 = 0+segmentsAround*6;
		}
		else
		{
			// 1st triangle (Right-side - Inner to Sidewall)
			triVIndex0 = &triData[ (triIndex+0)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+0)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+0)*3 + 2 ];
			*triVIndex0 = circleSegment;
			*triVIndex1 = circleSegment + segmentsAround;
			*triVIndex2 = circleSegment +1;
			// 2nd triangle
			triVIndex0 = &triData[ (triIndex+1)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+1)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+1)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround;
			*triVIndex1 = circleSegment+segmentsAround+1;
			*triVIndex2 = circleSegment +1;

			// 3rd triangle (Right-side - Sidewall to shoulder)
			triVIndex0 = &triData[ (triIndex+2)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+2)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+2)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*1;
			*triVIndex1 = circleSegment+segmentsAround*2;
			*triVIndex2 = circleSegment+segmentsAround*1 +1;
			// 4th triangle
			triVIndex0 = &triData[ (triIndex+3)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+3)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+3)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*2;
			*triVIndex1 = circleSegment+segmentsAround*2 +1;
			*triVIndex2 = circleSegment+segmentsAround*1 +1;


			// 5th triangle (Right-side - Shoulder to Tread)
			triVIndex0 = &triData[ (triIndex+4)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+4)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+4)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*2;
			*triVIndex1 = circleSegment+segmentsAround*3;
			*triVIndex2 = circleSegment+segmentsAround*2 +1;
			// 6th triangle
			triVIndex0 = &triData[ (triIndex+5)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+5)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+5)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*3;
			*triVIndex1 = circleSegment+segmentsAround*3 +1;
			*triVIndex2 = circleSegment+segmentsAround*2 +1;

			//////////////////////////////////////////////////////////////
			// 7th triangle (Right-side Tread to Left-side Tread)
			triVIndex0 = &triData[ (triIndex+6)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+6)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+6)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*3;
			*triVIndex1 = circleSegment+segmentsAround*4;
			*triVIndex2 = circleSegment+segmentsAround*3 +1;
			// 8th triangle
			triVIndex0 = &triData[ (triIndex+7)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+7)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+7)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*4;
			*triVIndex1 = circleSegment+segmentsAround*4 +1;
			*triVIndex2 = circleSegment+segmentsAround*3 +1;
			/////////////////////////////////////////////////////////////


			// 9th triangle (Left-side - Tread to Shoulder)
			triVIndex0 = &triData[ (triIndex+8)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+8)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+8)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*4;
			*triVIndex1 = circleSegment+segmentsAround*5;
			*triVIndex2 = circleSegment+segmentsAround*4 +1;
			// 10th triangle
			triVIndex0 = &triData[ (triIndex+9)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+9)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+9)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*5;
			*triVIndex1 = circleSegment+segmentsAround*5 +1;
			*triVIndex2 = circleSegment+segmentsAround*4 +1;


			// 11th triangle (Left-side - Shoulder to Sidewall)
			triVIndex0 = &triData[ (triIndex+10)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+10)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+10)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*5;
			*triVIndex1 = circleSegment+segmentsAround*6;
			*triVIndex2 = circleSegment+segmentsAround*5 +1;
			// 12th triangle
			triVIndex0 = &triData[ (triIndex+11)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+11)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+11)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*6;
			*triVIndex1 = circleSegment+segmentsAround*6 +1;
			*triVIndex2 = circleSegment+segmentsAround*5 +1;

			// 13th triangle (Left-side - Sidewall to Inner)
			triVIndex0 = &triData[ (triIndex+12)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+12)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+12)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*6;
			*triVIndex1 = circleSegment+segmentsAround*7;
			*triVIndex2 = circleSegment+segmentsAround*6 +1;
			// 14th triangle
			triVIndex0 = &triData[ (triIndex+13)*3 + 0 ];
			triVIndex1 = &triData[ (triIndex+13)*3 + 1 ];
			triVIndex2 = &triData[ (triIndex+13)*3 + 2 ];
			*triVIndex0 = circleSegment+segmentsAround*7;
			*triVIndex1 = circleSegment+segmentsAround*7 +1;
			*triVIndex2 = circleSegment+segmentsAround*6 +1;
		}
		triIndex +=14;
	} 


	///////////////////////////////////////////////////////////
	//
	// UV Coordinates
	//
	// the tread pattern should always take up the middle 33%
	// of the texture
	// tried writting distance calculation support to get the
	// sides of the tire to always have even xyz <-> uv space
	// but it was sorta tedious.  and this hard coded approach
	// insures any tire texture will work ok on any size tire.
	// though wider tires will experience more tread stretching
	// unless they also have a texture with a more appropriate
	// resolution.
	unsigned int texCoordFloats = vertexCount * 2;
	float *texData = new float [ texCoordFloats ];


	for ( unsigned int uvl=0 ; uvl< texCoordFloats ; uvl+=2 )
	{
		// U coord
	
		float *u = &texData[uvl];
		*u = uvl % segmentsAround;
		*u = *u / segmentsAround;

		float *v = &texData[uvl+1];
		/*   *v = floor ( uvl+1 / segmentsAround );
		*v = *v / (vertexRings-1);*/
		
		if ( uvl < segmentsAround*1 )
			*v = 0.00f;
		else if ( uvl < segmentsAround*2 )
			*v = 0.10f;
		else if ( uvl < segmentsAround*3 )
			*v = 0.25f;
		else if ( uvl < segmentsAround*4 )
			*v = 0.333333333f;
		else if ( uvl < segmentsAround*5 )
			*v = 0.666666666f;
		else if ( uvl < segmentsAround*6 )
			*v = 0.75f;
		else if ( uvl < segmentsAround*7 )
			*v = 0.90f;
		else if ( uvl < segmentsAround*8 )
			*v = 1.00f;
		else
		{
			// shouldn't be able to get here
			// maybe put an error message here
			//  "tiregen error: accessing code that shouldn't be reachable"
		}


		// so for the very last segment it will need something a little special
		// otherwise it draws the entire texture in that tiny last segment space
		if ( (uvl % segmentsAround == 0) && (uvl > vertexRings * 8) )
		{
			*u = 1.0f;
		}

	}


	// assign VERTEXARRAY to this data?
	// or am i telling VERTEXARRAY to copy this data?
	tire->SetVertices(vertexData, vertexFloatCount);
	tire->SetFaces((int *)triData, triVIndexCount);
	tire->SetTexCoords(0, texData, texCoordFloats);



	// free up the temp data
	delete vertexData;
	delete triData;
	delete texData;

}

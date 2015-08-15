#include "pch.h"
#include "bezier.h"
#include "unittest.h"

#include <cmath>

#define M_PI  3.14159265358979323846

std::ostream & operator << (std::ostream &os, const BEZIER & b)
{
	os << "====" << std::endl;
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
			os << b[y*4+x] << std::endl;
		os << "----" << std::endl;
	}
	os << "====" << std::endl;
	return os;
}

BEZIER::BEZIER()
{
	next_patch = NULL;
	turn = 0;
	dist_from_start = 0.0;
	length = 0.0;
	have_racingline = false;
	radius = 1;
	track_radius = 100;
	track_curvature = 1;
}

BEZIER::~BEZIER()
{
	
}

AABB <float> BEZIER::GetAABB() const
{
	float maxv[3];
	float minv[3];
	bool havevals[6];
	for (int n = 0; n < 6; ++n)
		havevals[n] = false;

	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			MATHVECTOR<float,3> temp(points[x][y]);

			//cache for bbox stuff
			for ( int n = 0; n < 3; ++n )
			{
				if (!havevals[n])
				{
					maxv[n] = temp[n];
					havevals[n] = true;
				}
				else if (temp[n] > maxv[n])
					maxv[n] = temp[n];

				if (!havevals[n+3])
				{
					minv[n] = temp[n];
					havevals[n+3] = true;
				}
				else if (temp[n] < minv[n])
					minv[n] = temp[n];
			}
		}
	}

	MATHVECTOR<float,3> bboxmin(minv[0], minv[1], minv[2]);
	MATHVECTOR<float,3> bboxmax(maxv[0], maxv[1], maxv[2]);

	AABB <float> box;
	box.SetFromCorners(bboxmin, bboxmax);
	return box;
}

void BEZIER::SetFromCorners(const MATHVECTOR<float,3> & fl, const MATHVECTOR<float,3> & fr, const MATHVECTOR<float,3> & bl, const MATHVECTOR<float,3> & br)
{
	MATHVECTOR<float,3> temp;
	
	center = fl + fr + bl + br;
	center = center*0.25;
	radius = 0;
	if ((fl - center).Magnitude() > radius)
		radius = (fl - center).Magnitude();
	if ((fr - center).Magnitude() > radius)
		radius = (fr - center).Magnitude();
	if ((bl - center).Magnitude() > radius)
		radius = (bl - center).Magnitude();
	if ((br - center).Magnitude() > radius)
		radius = (br - center).Magnitude();
	
	//assign corners
	points[0][0] = fl;
	points[0][3] = fr;
	points[3][3] = br;
	points[3][0] = bl;
	
	//calculate intermediate front and back points
	temp = fr - fl;
	if (temp.Magnitude() < 0.0001)
	{
		points[0][1] = fl;
		points[0][2] = fl;
	}
	else
	{
		points[0][1] = fl + temp.Normalize()*(temp.Magnitude()/3.0);
		points[0][2] = fl + temp.Normalize()*(2.0*temp.Magnitude()/3.0);
	}
	
	temp = br - bl;
	if (temp.Magnitude() < 0.0001)
	{
		points[3][1] = bl;
		points[3][2] = bl;
	}
	else
	{
		points[3][1] = bl + temp.Normalize()*(temp.Magnitude()/3.0);
		points[3][2] = bl + temp.Normalize()*(2.0*temp.Magnitude()/3.0);
	}
	
	
	//calculate intermediate left and right points	
	int i;
	for (i = 0; i < 4; ++i)
	{
		temp = points[3][i] - points[0][i];
		if (temp.Magnitude() > 0.0001)
		{
			points[1][i] = points[0][i] + temp.Normalize()*(temp.Magnitude()/3.0);
			points[2][i] = points[0][i] + temp.Normalize()*(2.0*temp.Magnitude()/3.0);
		}
		else
		{
			points[1][i] = points[0][i];
			points[2][i] = points[0][i];
		}
	}
	
	//CheckForProblems();
}

void BEZIER::Attach(BEZIER & other, bool reverse)
{
	/*if (!reverse)
	{
		//move the other patch to the location of this patch and force its
		// intermediate points into a nice grid layout
		other.SetFromCorners(other.points[0][0], other.points[0][3], points[0][0], points[0][3]);
		
		for (int x = 0; x < 4; x++)
		{
			//slope points in the forward direction
			MATHVECTOR<float,3> slope = other.points[0][x] - points[3][x];
			if (slope.Magnitude() > 0.0001)
				slope = slope.Normalize();
			
			float otherlen = (other.points[0][x] - other.points[3][x]).Magnitude();
			float mylen = (points[0][x] - points[3][x]).Magnitude();
			
			float meanlen = (otherlen + mylen)/2.0;
			float leglen = meanlen / 3.0;
			
			if (slope.Magnitude() > 0.0001)
			{
				other.points[2][x] = other.points[3][x] + slope*leglen;
				points[1][x] = points[0][x] + slope*(-leglen);
			}
			else
			{
				other.points[2][x] = other.points[3][x];
				points[1][x] = points[0][x];
			}
		}
	}*/
	
	//CheckForProblems();
	
	//store the pointer to next patch
	next_patch = &other;

	//calculate the track radius at the connection of this patch and next patch
	MATHVECTOR<float,3> a = SurfCoord(0.5,0.0);
	MATHVECTOR<float,3> b = SurfCoord(0.5,1.0);
	MATHVECTOR<float,3> c = other.SurfCoord(0.5,1.0);
	
	if (reverse)
	{
		a = SurfCoord(0.5,1.0);
		b = SurfCoord(0.5,0.0);
		c = other.SurfCoord(0.5,0.0);
		
		//Reverse();
	}
	
	//racing_line = a;
	MATHVECTOR<float,3> d1 = a - b;
	MATHVECTOR<float,3> d2 = c - b;
	float diff = d2.Magnitude() - d1.Magnitude();
	double dd = ((d1.Magnitude() < 0.0001) || (d2.Magnitude() < 0.0001)) ? 0.0 : d1.Normalize().dot(d2.Normalize());
	float angle = acos((dd>=1.0L)?1.0L:(dd<=-1.0L)?-1.0L:dd);
	float d1d2mag = d1.Magnitude() + d2.Magnitude();
	float alpha = (d1d2mag < 0.0001) ? 0.0f : (M_PI * diff + 2.0 * d1.Magnitude() * angle) / d1d2mag / 2.0;
	if (fabs(alpha - M_PI/2.0) < 0.001) track_radius = 10000.0;
	else track_radius = d1.Magnitude() / 2.0 / cos(alpha);
	if (d1.Magnitude() < 0.0001)
		track_curvature = 0.0;
	else
		track_curvature = 2.0 * cos(alpha) / d1.Magnitude();

	//determine it's a left or right turn at the connection
	MATHVECTOR<float,3> d = d1.cross(d2);
	if (fabs(d[0]) < 0.1 && fabs(d[1]) < 0.1 && fabs(d[2]) < 0.1)
	{
		turn = 0; //straight ahead
	}
	else if (d[1] > 0.0) turn = -1; //left turn ahead
	else turn = 1; //right turn ahead

	//calculate distance from start of the road
	if (other.next_patch == NULL || reverse) other.dist_from_start = dist_from_start + d1.Magnitude();
	length = d1.Magnitude();
}

void BEZIER::Reverse()
{
	MATHVECTOR<float,3> oldpoints[4][4];
	
	for (int n = 0; n < 4; ++n)
		for (int i = 0; i < 4; ++i)
			oldpoints[n][i] = points[n][i];
		
	for (int n = 0; n < 4; ++n)
		for (int i = 0; i < 4; ++i)
			points[n][i] = oldpoints[3-n][3-i];
}

MATHVECTOR<float,3> BEZIER::Bernstein(float u, MATHVECTOR<float,3> *p) const
{
	float oneminusu(1.0f-u);
	
	MATHVECTOR<float,3> a = p[0] * (u*u*u);
	MATHVECTOR<float,3> b = p[1] * (3*u*u*oneminusu);
	MATHVECTOR<float,3> c = p[2] * (3*u*oneminusu*oneminusu);
	MATHVECTOR<float,3> d = p[3] * (oneminusu*oneminusu*oneminusu);

	//return a+b+c+d;
	return MATHVECTOR<float,3> (a[0]+b[0]+c[0]+d[0],a[1]+b[1]+c[1]+d[1],a[2]+b[2]+c[2]+d[2]);
}

MATHVECTOR<float,3> BEZIER::SurfCoord(float px, float py) const
{
	MATHVECTOR<float,3> temp[4];
	MATHVECTOR<float,3> temp2[4];
	int i, j;

	/*if (px == 0.0 && py == 0.0)
		return points[3][3];
	if (px == 1.0 && py == 1.0)
		return points[0][0];
	if (px == 1.0 && py == 0.0)
		return points[0][3];
	if (px == 0.0 && py == 1.0)
		return points[3][0];*/
	
	//get splines along x axis
	for (j = 0; j < 4; j++)
	{
		for (i = 0; i < 4; ++i)
			temp2[i] = points[j][i];
		temp[j] = Bernstein(px, temp2);
	}
	
	return Bernstein(py, temp);
}

MATHVECTOR<float,3> BEZIER::SurfNorm(float px, float py) const
{
	MATHVECTOR<float,3> temp[4];
	MATHVECTOR<float,3> temp2[4];
	MATHVECTOR<float,3> tempx[4];

	//get splines along x axis
	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 4; ++i)
			temp2[i] = points[j][i];
		temp[j] = Bernstein(px, temp2);
	}
	
	//get splines along y axis
	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 4; ++i)
			temp2[i] = points[i][j];
		tempx[j] = Bernstein(py, temp2);
	}
	
	return -(BernsteinTangent(px, tempx).cross(BernsteinTangent(py, temp)).Normalize());
}

MATHVECTOR<float,3> BEZIER::BernsteinTangent(float u, MATHVECTOR<float,3> *p) const
{
	MATHVECTOR<float,3> a = (p[1]-p[0])*(3*pow(u,2));
	MATHVECTOR<float,3> b = (p[2]-p[1])*(3*2*u*(1-u));
	MATHVECTOR<float,3> c = (p[3]-p[2])*(3*pow((1-u),2));

	return a+b+c;
}

BEZIER & BEZIER::CopyFrom(const BEZIER &other)
{
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			points[x][y] = other.points[x][y];
		}
	}
	
	center = other.center;
	radius = other.radius;
	length = other.length;
	dist_from_start = other.dist_from_start;
	next_patch = other.next_patch;
	track_radius = other.track_radius;
	turn = other.turn;
	track_curvature = other.track_curvature;
	racing_line = other.racing_line;
	have_racingline = other.have_racingline;

	return *this;
}

void BEZIER::ReadFrom(std::istream &openfile)
{
	assert(openfile);
	
	//MATHVECTOR<float,3> npoints[4][4];
	
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			openfile >> points[x][y][0];
			openfile >> points[x][y][1];
			openfile >> points[x][y][2];
		}
	}
}

void BEZIER::WriteTo(std::ostream &openfile) const
{
	assert(openfile);
	
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			openfile << points[x][y][0] << " ";
			openfile << points[x][y][1] << " ";
			openfile << points[x][y][2] << std::endl;
		}
	}
}

bool BEZIER::CollideSubDivQuadSimple(const MATHVECTOR<float,3> & origin, const MATHVECTOR<float,3> & direction, MATHVECTOR<float,3> &outtri) const
{
	MATHVECTOR<float,3> normal;
	return CollideSubDivQuadSimpleNorm(origin, direction, outtri, normal);
}

bool BEZIER::CollideSubDivQuadSimpleNorm(const MATHVECTOR<float,3> & origin, const MATHVECTOR<float,3> & direction, MATHVECTOR<float,3> &outtri, MATHVECTOR<float,3> & normal) const
{
	bool col = false;
	const int COLLISION_QUAD_DIVS = 6;
	const bool QUAD_DIV_FAST_DISCARD = true;
	
	float t, u, v;
	
	float su = 0;
	float sv = 0;
	
	float umin = 0;
	float umax = 1;
	float vmin = 0;
	float vmax = 1;
	
	//const float fuzziness = 0.13;
	
	MATHVECTOR<float,3> ul = points[3][3];
	MATHVECTOR<float,3> ur = points[3][0];
	MATHVECTOR<float,3> br = points[0][0];
	MATHVECTOR<float,3> bl = points[0][3];
	
	//int subdivnum = 0;

	bool loop = true;
	
	float areacut = 0.5;

	for (int i = 0; i < COLLISION_QUAD_DIVS && loop; ++i)
	{
		float tu[2];
		float tv[2];
		
		//speedup for i == 0
		//if (i != 0)
		{
			tu[0] = umin;
			if (tu[0] < 0)
				tu[0] = 0;
			tu[1] = umax;
			if (tu[1] > 1)
				tu[1] = 1;
			
			tv[0] = vmin;
			if (tv[0] < 0)
				tv[0] = 0;
			tv[1] = vmax;
			
			if (tv[1] > 1)
				tv[1] = 1;
			
			ul = SurfCoord(tu[0], tv[0]);
			ur = SurfCoord(tu[1], tv[0]);
			br = SurfCoord(tu[1], tv[1]);
			bl = SurfCoord(tu[0], tv[1]);
		}
		
		//u = v = 0.0;

		col = IntersectQuadrilateralF(origin, direction,
			ul, ur, br, bl,
			t, u, v);
		
		if (col)
		{
			//expand quad UV to surface UV
			//su = u * (umax - umin) + umin;
			//sv = v * (vmax - vmin) + vmin;
			
			su = u * (tu[1] - tu[0]) + tu[0];
			sv = v * (tv[1] - tv[0]) + tv[0];
			
			//place max and min according to area hit
			vmax = sv + (0.5*areacut)*(vmax-vmin);
			vmin = sv - (0.5*areacut)*(vmax-vmin);
			umax = su + (0.5*areacut)*(umax-umin);
			umin = su - (0.5*areacut)*(umax-umin);
		}
		else
		{
			if ((i == 0) && QUAD_DIV_FAST_DISCARD)
			//if (QUAD_DIV_FAST_DISCARD)
			{
				outtri = origin;
				return false;
			}
			else
			{
				/*if (verbose)
				{
					cout << "<" << i << ": nocol " << su << "," << sv << ">" << endl;
					cout << "<" << umin << "," << umax << ";" << vmin << "," << vmax << ">" << endl;
					
					ul.DebugPrint();
					ur.DebugPrint();
					bl.DebugPrint();
					br.DebugPrint();
					
					cout << endl;
				}*/
				
				loop = false;
			}
		}
	}
	
	if (col)// || QUAD_DIV_FAST_DISCARD)
	//if (col)
	{
		/*if (!col)
			cout << "blip " << su << "," << sv << ": " << u << "," << v << endl;*/
		outtri = SurfCoord(su, sv);
		normal = SurfNorm(su, sv);
		//if (verbose)
			//cout << "<" << i << ">" << endl;
		return true;
	}
	else
	{
		outtri = origin;
		return false;
	}
}

void BEZIER::DeCasteljauHalveCurve(MATHVECTOR<float,3> * points4, MATHVECTOR<float,3> * left4, MATHVECTOR<float,3> * right4) const
{
	left4[0] = points4[0];
	left4[1] = (points4[0]+points4[1])*0.5;
	MATHVECTOR<float,3> point23 = (points4[1]+points4[2])*0.5;
	left4[2] = (left4[1]+point23)*0.5;
	
	right4[3] = points4[3];
	right4[2] = (points4[3]+points4[2])*0.5;
	right4[1] = (right4[2]+point23)*0.5;
	
	left4[3] = right4[0] = (right4[1]+left4[2])*0.5;
}

bool BEZIER::CheckForProblems() const
{
	//MATHVECTOR<float,3> fl (points[0][0]), fr(points[0][3]), br(points[3][3]), bl(points[3][0]);
	
	MATHVECTOR<float,3> corners[4];
	corners[0] = points[0][0];
	corners[1] = points[0][3];
	corners[2] = points[3][3];
	corners[3] = points[3][0];
	
	bool problem = false;
	
	for (int i = 0; i < 4; ++i)
	{
		MATHVECTOR<float,3> leg1(corners[(i+1)%4] - corners[i]);
		MATHVECTOR<float,3> leg2(corners[(i+2)%4] - corners[i]);
		MATHVECTOR<float,3> leg3(corners[(i+3)%4] - corners[i]);
		
		MATHVECTOR<float,3> dir1 = leg1.cross(leg2);
		MATHVECTOR<float,3> dir2 = leg1.cross(leg3);
		MATHVECTOR<float,3> dir3 = leg2.cross(leg3);
		
		if (dir1.dot(dir2) < -0.0001)
			problem = true;
		if (dir1.dot(dir3) < -0.0001)
			problem = true;
		if (dir3.dot(dir2) < -0.0001)
			problem = true;
		
		/*if (problem)
		{
			std::cout << *this;
			std::cout << "i: " << i << ", " << (i+1)%4 << ", " << (i+2)%4 << std::endl;
			std::cout << corners[0] << std::endl;
			std::cout << corners[1] << std::endl;
			std::cout << corners[2] << std::endl;
			std::cout << corners[3] << std::endl;
			std::cout << leg1 << std::endl;
			std::cout << leg2 << std::endl;
			std::cout << dir1 << std::endl;
			std::cout << dir1.dot(dir2) << ", " <<dir1.dot(dir3) << ", " << dir3.dot(dir2) <<std::endl;
		}*/
	}
	
	//if (problem) cout << "Degenerate bezier patch detected" << endl;
	
	return problem;
}

QT_TEST(bezier_test)
{
	MATHVECTOR<float,3> p[4], l[4], r[4];
	p[0].Set(-1,0,0);
	p[1].Set(-1,1,0);
	p[2].Set(1,1,0);
	p[3].Set(1,0,0);
	BEZIER b;
	b.DeCasteljauHalveCurve(p,l,r);
	QT_CHECK_EQUAL(l[0],(MATHVECTOR<float,3>(-1,0,0)));
	QT_CHECK_EQUAL(l[1],(MATHVECTOR<float,3>(-1,0.5,0)));
	QT_CHECK_EQUAL(l[2],(MATHVECTOR<float,3>(-0.5,0.75,0)));
	QT_CHECK_EQUAL(l[3],(MATHVECTOR<float,3>(0,0.75,0)));
	
	QT_CHECK_EQUAL(r[3],(MATHVECTOR<float,3>(1,0,0)));
	QT_CHECK_EQUAL(r[2],(MATHVECTOR<float,3>(1,0.5,0)));
	QT_CHECK_EQUAL(r[1],(MATHVECTOR<float,3>(0.5,0.75,0)));
	QT_CHECK_EQUAL(r[0],(MATHVECTOR<float,3>(0,0.75,0)));
	
	b.SetFromCorners(MATHVECTOR<float,3>(1,0,1),MATHVECTOR<float,3>(-1,0,1),MATHVECTOR<float,3>(1,0,-1),MATHVECTOR<float,3>(-1,0,-1));
	QT_CHECK(!b.CheckForProblems());
}

bool BEZIER::IntersectQuadrilateralF(const MATHVECTOR<float,3> & orig, const MATHVECTOR<float,3> & dir,
				     const MATHVECTOR<float,3> & v_00, const MATHVECTOR<float,3> & v_10,
				     const MATHVECTOR<float,3> & v_11, const MATHVECTOR<float,3> & v_01,
				     float &t, float &u, float &v) const
{
	const float EPSILON = 0.000001;
	
	// Reject rays that are parallel to Q, and rays that intersect the plane
	// of Q either on the left of the line V00V01 or below the line V00V10.
	MATHVECTOR<float,3> E_01 = v_10 - v_00;
	MATHVECTOR<float,3> E_03 = v_01 - v_00;
	MATHVECTOR<float,3> P = dir.cross(E_03);
	float det = E_01.dot(P);
	
	if (std::abs(det) < EPSILON) return false;
	
	MATHVECTOR<float,3> T = orig - v_00;
	float alpha = T.dot(P) / det;
	
	if (alpha < 0.0) return false;
	
	MATHVECTOR<float,3> Q = T.cross(E_01);
	float beta = dir.dot(Q) / det;
	
	if (beta < 0.0) return false;
	
	if (alpha + beta > 1.0)
	{
		// Reject rays that that intersect the plane of Q either on
		// the right of the line V11V10 or above the line V11V00.
		MATHVECTOR<float,3> E_23 = v_01 - v_11;
		MATHVECTOR<float,3> E_21 = v_10 - v_11;
		MATHVECTOR<float,3> P_prime = dir.cross(E_21);
		float det_prime = E_23.dot(P_prime);
		
		if (std::abs(det_prime) < EPSILON) return false;
			
		MATHVECTOR<float,3> T_prime = orig - v_11;
		float alpha_prime = T_prime.dot(P_prime) / det_prime;
		
		if (alpha_prime < 0.0) return false;
			
		MATHVECTOR<float,3> Q_prime = T_prime.cross(E_23);
		float beta_prime = dir.dot(Q_prime) / det_prime;
		
		if (beta_prime < 0.0) return false;
	}
	
	// Compute the ray parameter of the intersection point, and
	// reject the ray if it does not hit Q.
	t = E_03.dot(Q) / det;
	
	if (t < 0.0) return false;
	
	// Compute the barycentric coordinates of the fourth vertex.
	// These do not depend on the ray, and can be precomputed
	// and stored with the quadrilateral.
	float alpha_11, beta_11;
	MATHVECTOR<float,3> E_02 = v_11 - v_00;
	MATHVECTOR<float,3> n = E_01.cross(E_03);
	
	if ((std::abs(n[0]) >= std::abs(n[1]))
		    && (std::abs(n[0]) >= std::abs(n[2])))
	{
		alpha_11 = ((E_02[1] * E_03[2]) - (E_02[2] * E_03[1])) / n[0];
		beta_11 = ((E_01[1] * E_02[2]) - (E_01[2]  * E_02[1])) / n[0];
	}
	else if ((std::abs(n[1]) >= std::abs(n[0]))
			 && (std::abs(n[1]) >= std::abs(n[2])))
	{
		alpha_11 = ((E_02[2] * E_03[0]) - (E_02[0] * E_03[2])) / n[1];
		beta_11 = ((E_01[2] * E_02[0]) - (E_01[0]  * E_02[2])) / n[1];
	}
	else
	{
		alpha_11 = ((E_02[0] * E_03[1]) - (E_02[1] * E_03[0])) / n[2];
		beta_11 = ((E_01[0] * E_02[1]) - (E_01[1]  * E_02[0])) / n[2];
	}
	
	// Compute the bilinear coordinates of the intersection point.
	if (std::abs(alpha_11 - (1.0)) < EPSILON)
	{
		// Q is a trapezium.
		u = alpha;
		if (std::abs(beta_11 - (1.0)) < EPSILON) v = beta; // Q is a parallelogram.
		else v = beta / ((u * (beta_11 - (1.0))) + (1.0)); // Q is a trapezium.
	}
	else if (std::abs(beta_11 - (1.0)) < EPSILON)
	{
		// Q is a trapezium.
		v = beta;
		if ( ((v * (alpha_11 - (1.0))) + (1.0)) == 0 )
		{
			return false;
		}
		u = alpha / ((v * (alpha_11 - (1.0))) + (1.0));
	}
	else
	{
		float A = (1.0) - beta_11;
		float B = (alpha * (beta_11 - (1.0)))
				- (beta * (alpha_11 - (1.0))) - (1.0);
		float C = alpha;
		float D = (B * B) - ((4.0) * A * C);
		if (D < 0) return false;
		float Q = (-0.5) * (B + ((B < (0.0) ? (-1.0) : (1.0))
				* std::sqrt(D)));
		u = Q / A;
		if ((u < (0.0)) || (u > (1.0))) u = C / Q;
		v = beta / ((u * (beta_11 - (1.0))) + (1.0));
	}
	
	return true;
}

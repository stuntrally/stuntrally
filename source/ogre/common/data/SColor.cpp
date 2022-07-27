#include "pch.h"
#include "../Def_Str.h"
#include "SColor.h"
using namespace std;
using namespace Ogre;


///  Color  . . . . . . . .
SColor::SColor()
	:h(0.f), s(0.f), v(0.f), a(0.f), n(0.f)
{	}
SColor::SColor(float h1, float s1, float v1, float a1, float n1)
	:h(h1), s(s1), v(v1), a(a1), n(n1)
{	}

//  tool check err
string SColor::Check(string t)
{
	string e;
	if (h > 1.f)  e += " h>1";  if (h < 0.f)  e += " h<0";
	if (s > 1.f)  e += " s>1";  if (s < 0.f)  e += " s<0";
	if (v > 3.f)  e += " v>3";  if (v < 0.f)  e += " v<0";
	if (a > 3.f)  e += " a>2";  if (a < 0.f)  e += " a<0";
	if (n > 1.f)  e += " n>1";  if (n < 0.f)  e += " n<0";
	if (!e.empty())  e += "  " + t + "  " + Save();
	return e;
}

//  load from old rgb
void SColor::LoadRGB(Vector3 rgb)
{
	Vector3 u = rgb;
    float vMin = std::min(u.x, std::min(u.y, u.z));
    n = vMin < 0.f ? -vMin : 0.f;  // neg = minimum  only for negative colors
    if (vMin < 0.f)  u += Vector3(n,n,n);  // cancel, normalize to 0

    float vMax = std::max(u.x, std::max(u.y, u.z));  // get max for above 1 colors
    v = vMax;
    if (vMax > 1.f)  u /= v;  // above, normalize to 1

    ColourValue cl(u.x, u.y, u.z);  // get hue and sat
    float vv;  // not important or 1
    cl.getHSB(&h, &s, &vv);
}	

//  get clr
Vector3 SColor::GetRGB1() const
{
	ColourValue cl;
	cl.setHSB(h, s, 1.f);
	float vv = std::min(1.f, v);  // * (1.f - n)
	return Vector3(
		cl.r * vv,
		cl.g * vv,
		cl.b * vv);
}
Vector3 SColor::GetRGB() const
{
	ColourValue cl;
	cl.setHSB(h, s, 1.f);
	return Vector3(
		cl.r * v -n,
		cl.g * v -n,
		cl.b * v -n);
}
ColourValue SColor::GetClr() const
{
	Vector3 c = SColor::GetRGB();
	return ColourValue(c.x, c.y, c.z);
}
Vector4 SColor::GetRGBA() const
{
	Vector3 c = GetRGB();
	return Vector4(c.x, c.y, c.z, a);
}

//  string
void SColor::Load(const char* ss)
{
	h=0.f; s=1.f; v=1.f; a=1.f; n=0.f;
	int i = sscanf(ss, "%f %f %f %f %f", &h,&s,&v,&a,&n);
#if 0  // test bad values
	if (h < 0.f || h > 1.f ||
		s < 0.f || s > 1.f ||
		v < 0.f || v > 4.f || // 3
		a < 0.f || a > 3.f || // 2
		n < 0.f || n > 1.f)
		LogO("!! BAD SColor: "+fToStr(h)+" "+fToStr(s)+" "+fToStr(v)+" "+fToStr(a)+" "+fToStr(n));
#endif
	if (i == 5)
		return;  // new

	if (i < 3 || i > 5)
	{	LogO("NOT 3..5 components color!");  }

	//  rgb old < 2.4
	//float r=h,g=s,b=v,u=a;
	LoadRGB(Vector3(h,s,v));

	//  test back
	/*Vector4 c = GetRGBA();
	float d = fabs(c.x-r) + fabs(c.y-g) + fabs(c.z-b) + fabs(c.w-u);
	LogO(String("CLR CHK ")+ss);
	LogO("CLR CHK r "+fToStr(c.x-r,2,5)+" g "+fToStr(c.y-g,2,5)+" b "+fToStr(c.z-b,2,5)+" a "+fToStr(c.w-u,2,5)
		+"  d "+fToStr(d,2,4) + (d > 0.01f ? " !!! BAD " : " ok"));/**/
}

string SColor::Save() const
{
	string ss = fToStr(h,3,5)+" "+fToStr(s,3,5)+" "+fToStr(v,3,5)+" "+fToStr(std::min(3.f,a),3,5)+" "+fToStr(n,3,5);
	return ss;
}

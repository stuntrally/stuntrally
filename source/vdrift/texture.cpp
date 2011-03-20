#include "stdafx.h"
#include "texture.h"


Uint8 ExtractComponent(Uint32 value, Uint32 mask, Uint32 shift, Uint32 loss)
{
	Uint32 temp = value & mask;
	temp = temp >> shift;
	temp = temp << loss;
	return (Uint8) temp;
}

//save the component to the value using the given mask, shift, loss, and return the resulting value
Uint32 InsertComponent(Uint32 value, Uint8 component, Uint32 mask, Uint32 shift, Uint32 loss)
{
	Uint32 temp = component;
	temp = temp >> loss;
	temp = temp << shift;
	return (value & ~mask) | temp;
}

struct COMPONENTINFO
{
	Uint32 mask;
	Uint32 shift;
	Uint32 loss;
};

void TEXTURE_GL::Unload()
{
	loaded = false;
}

bool TEXTURE_GL::IsEqualTo(const TEXTUREINFO & texinfo) const
{
	return (texinfo.GetName() == texture_info.GetName() && texinfo.GetMipMap() == texture_info.GetMipMap());
}

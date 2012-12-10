#ifndef _TRACK_OBJECT_H
#define _TRACK_OBJECT_H

class MODEL;
class TEXTURE_GL;
class TRACKSURFACE;

class TRACK_OBJECT
{
public:
	TRACK_OBJECT(
		MODEL * model,
		TEXTURE_GL * texture,
		bool surface);
	
	MODEL * GetModel() const
	{
		return model;
	}

	bool HasSurface() const
	{
		return surface;
	}

private:
	MODEL * model;
	TEXTURE_GL * texture;
	//const TRACKSURFACE * surface; //old
	bool surface;
};

#endif

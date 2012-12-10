#include "pch.h"
#include "track_object.h"
#include "assert.h"

TRACK_OBJECT::TRACK_OBJECT(MODEL * m, TEXTURE_GL * t, bool s)  //const TRACKSURFACE * s)
: model(m), texture(t), surface(s)
{
	assert(model);
	assert(texture);
}


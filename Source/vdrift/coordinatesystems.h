#ifndef _COORDINATESYSTEMS_H
#define _COORDINATESYSTEMS_H

#ifdef __APPLE__
//#include <GLExtensionWrangler/glew.h>
//#include <OpenGL/gl.h>
#else
//#include <GL/glew.h>
//#include <GL/gl.h>
#endif
//#include <string>
//#include <iostream>
//#include <cassert>

namespace COORDINATESYSTEMS
{
	void ConvertCarCoordinateSystemV2toV1(float & x, float & y, float & z);
}

#endif

#ifndef HWMOUSE_H
#define HWMOUSE_H

#include <cstring>
#include <string>

class HWMouse
{
public:
	HWMouse(size_t windowID, const int xhot, const int yhot, const std::string& filename);
	~HWMouse();
	
	void show();
	void hide();
	
protected:
	bool mVisible;
};

#endif

#pragma once
#include <OgrePlatform.h>

// force feedback is only supported on linux.
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	//#define ENABLE_FORCE_FEEDBACK
#endif

#ifdef ENABLE_FORCE_FEEDBACK

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <iostream>
#include <linux/input.h>

class FORCEFEEDBACK
{
public:
	FORCEFEEDBACK( std::string device, std::ostream & error_output, std::ostream & info_output);
	~FORCEFEEDBACK() {}
	void update( double force, double * position, double dt, std::ostream & error_output );
	void disable() { enabled = false; }
private:
	std::string device_name;
	bool enabled;
	bool stop_and_play;
	int device_handle;
	int axis_code, axis_min, axis_max;
	struct ff_effect effect;
	double lastforce;
};

#endif // ENABLE_FORCE_FEEDBACK

#include "pch.h"

#include "forcefeedback.h"

#ifdef ENABLE_FORCE_FEEDBACK

#include <cstring>
#include <string>

using std::string;
using std::endl;

//#define TEST_BIT(bit,bits) (((bits[bit>>5]>>(bit&0x1f))&1)!=0)

/* Number of bits for 1 unsigned char */
#define nBitsPerUchar          (sizeof(unsigned char) * 8)

/* Number of unsigned chars to contain a given number of bits */
#define nUcharsForNBits(nBits) ((((nBits)-1)/nBitsPerUchar)+1)

/* Index=Offset of given bit in 1 unsigned char */
#define bitOffsetInUchar(bit)  ((bit)%nBitsPerUchar)

/* Index=Offset of the unsigned char associated to the bit
   at the given index=offset */
#define ucharIndexForBit(bit)  ((bit)/nBitsPerUchar)

/* Value of an unsigned char with bit set at given index=offset */
#define ucharValueForBit(bit)  (((unsigned char)(1))<<bitOffsetInUchar(bit))

/* Test the bit with given index=offset in an unsigned char array */
#define testBit(bit, array)    ((array[ucharIndexForBit(bit)] >> bitOffsetInUchar(bit)) & 1)

FORCEFEEDBACK::FORCEFEEDBACK( string device, std::ostream & error_output, std::ostream & info_output )
	: device_name(device), enabled(true), stop_and_play(false), lastforce(0)
{
	unsigned char key_bits[1 + KEY_MAX/8/sizeof(unsigned char)];
	unsigned char abs_bits[1 + ABS_MAX/8/sizeof(unsigned char)];
	unsigned char ff_bits[1 + FF_MAX/8/sizeof(unsigned char)];

	struct input_event event;
	int valbuf[16];

	// Open event device with write permission
	device_handle = open(device_name.c_str(),O_RDWR|O_NONBLOCK);
	if (device_handle<0)
	{
		error_output << "force feedback: can not open " << device_name << " (" << strerror(errno) << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}

	// Which buttons has the device?
	memset(key_bits,0,sizeof(key_bits));
	if (ioctl(device_handle,EVIOCGBIT(EV_KEY,sizeof(key_bits)),key_bits)<0)
	{
		error_output << "force feedback: can not get key bits (" << strerror(errno) << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}

	// Which axes has the device?
	memset(abs_bits,0,sizeof(abs_bits));
	if (ioctl(device_handle,EVIOCGBIT(EV_ABS,sizeof(abs_bits)),abs_bits)<0)
	{
		error_output << "force feedback: can not get abs bits (" << strerror(errno) << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}

	// Now get some information about force feedback
	memset(ff_bits,0,sizeof(ff_bits));
	if (ioctl(device_handle,EVIOCGBIT(EV_FF ,sizeof(ff_bits)),ff_bits)<0)
	{
		error_output << "force feedback: can not get ff bits (" << strerror(errno) << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}

	// Which axis is the x-axis?
	if      (testBit(ABS_X    ,abs_bits)) axis_code=ABS_X;
	else if (testBit(ABS_RX   ,abs_bits)) axis_code=ABS_RX;
	else if (testBit(ABS_WHEEL,abs_bits)) axis_code=ABS_WHEEL;
	else
	{
		error_output << "force feedback: no suitable x-axis found [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}

	// get axis value range
	if (ioctl(device_handle,EVIOCGABS(axis_code),valbuf)<0)
	{
		error_output << "force feedback: can not get axis value range (" << strerror(errno) << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}
	axis_min=valbuf[1];
	axis_max=valbuf[2];
	if (axis_min>=axis_max)
	{
		error_output << "force feedback: bad axis value range (" << axis_min << "," << axis_max << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}

	// force feedback supported?
	if (!testBit(FF_CONSTANT,ff_bits))
	{
		error_output << "force feedback: device (or driver) has no force feedback support [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}

	// Switch off auto centering
	memset(&event,0,sizeof(event));
	event.type=EV_FF;
	event.code=FF_AUTOCENTER;
	event.value=0;
	if (write(device_handle,&event,sizeof(event))!=sizeof(event))
	{
		error_output << "force feedback: failed to disable auto centering (" << strerror(errno) << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}

	// Initialize constant force effect
	memset(&effect,0,sizeof(effect));
	effect.type=FF_CONSTANT;
	effect.id=-1;
	effect.trigger.button=0;
	effect.trigger.interval=0;
	effect.replay.length=0xffff;
	effect.replay.delay=0;
	effect.u.constant.level=0;
	effect.direction=0xC000;
	effect.u.constant.envelope.attack_length=0;
	effect.u.constant.envelope.attack_level=0;
	effect.u.constant.envelope.fade_length=0;
	effect.u.constant.envelope.fade_level=0;

	// Upload effect
	if (ioctl(device_handle,EVIOCSFF,&effect)==-1)
	{
		error_output << "force feedback: uploading effect failed (" << strerror(errno) << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}

	/* Start effect */
	memset(&event,0,sizeof(event));
	event.type=EV_FF;
	event.code=effect.id;
	event.value=1;
	if (write(device_handle,&event,sizeof(event))!=sizeof(event))
	{
		error_output << "force feedback: starting effect failed (" << strerror(errno) << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
		disable();
		return;
	}

	info_output << "Force feedback initialized successfully" << std::endl;
}

void FORCEFEEDBACK::update(double force, double * position, double dt, std::ostream & error_output)
{
	if ( !enabled )
		return;

	struct input_event event;

        // Delete effect
	if (stop_and_play && effect.id!=-1)
	{
		if (ioctl(device_handle,EVIOCRMFF,effect.id)==-1) {
			error_output << "force feedback: removing effect failed (" << strerror(errno) << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
			disable();
			return;
		}
		effect.id=-1;
	}

        // Set force
	if (force>1.0) force=1.0;
	if (force<-1.0) force=-1.0;
	effect.direction=0xC000;
	//force = -1.0;
	effect.u.constant.level=(short)(force*32767.0); /* only to be safe */
	effect.u.constant.envelope.attack_level=effect.u.constant.level;
	effect.u.constant.envelope.fade_level=effect.u.constant.level;
	/*effect.u.constant.envelope.attack_level = (short)(lastforce*32767.0);
	effect.u.constant.envelope.attack_length = (short)(dt*1000.0);
	effect.u.constant.envelope.fade_length = (short)(dt*1000.0);
	//effect.u.constant.envelope.attack_level=(short)(force*32767.0);*/

	lastforce = force;

        // Upload effect
	if (ioctl(device_handle,EVIOCSFF,&effect)==-1)
	{
              //error_output << "error uploading effect" << endl;
                /* We do not exit here. Indeed, too frequent updates may be
				* refused, but that is not a fatal error */
	}
	else if (effect.id!=-1)
        // Start effect
        //if (stop_and_play && effect.id!=-1)
	{
		memset(&event,0,sizeof(event));
		event.type=EV_FF;
		event.code=effect.id;
		event.value=1;
		if (write(device_handle,&event,sizeof(event))!=sizeof(event))
		{
			error_output << "force feedback: re-starting effect failed (" << strerror(errno) << ") [" << __FILE__ << ":" << __LINE__ << "]" << endl;
			disable();
			return;
		}
	}

        // Get events
	while (read(device_handle,&event,sizeof(event))==sizeof(event))
	{
		if (event.type==EV_ABS && event.code==axis_code)
		{
			*position=((double)(((short)event.value)-axis_min))*2.0/(axis_max-axis_min)-1.0;
			if (*position>1.0) *position=1.0;
			else if (*position<-1.0) *position=-1.0;
		}
	}
}

#endif // ENABLE_FORCE_FEEDBACK

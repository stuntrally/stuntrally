#ifndef _CRASHDETECTION_H
#define _CRASHDETECTION_H


class CRASHDETECTION
{
public:
	CRASHDETECTION();
	
	void Update(float vel, float dt);

	float GetMaxDecel() const
	{
		return maxdecel;
	}
	
//private:
	float lastvel;
	float curmaxdecel;
	float maxdecel;
	float deceltrigger;
};

#endif // _CRASHDETECTION_H

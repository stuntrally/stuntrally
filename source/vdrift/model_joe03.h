#pragma once
#include "model.h"

#include <string>
#include <ostream>

class JOEPACK;
struct JOEObject;

// This class handles all of the loading code
class MODEL_JOE03 : public MODEL
{
public:
	virtual ~MODEL_JOE03()
	{
		Clear();
	}

	virtual bool Load(const std::string & strFileName, std::ostream & error_output, bool genlist=true)
	{
		return Load(strFileName, error_output, genlist, 0);
	}
	
	virtual bool CanSave() const
	{
		return false;
	}
	
	bool Load(const std::string & strFileName, std::ostream & error_output, bool genlist, JOEPACK * pack);
	
	bool LoadFromHandle(FILE * f, JOEPACK * pack, std::ostream & error_output);

private:
	static const int JOE_MAX_FACES;
	static const int JOE_VERSION;
	static const float MODEL_SCALE;
	
	std::string modelpath;
	std::string modelname;
	
	// This reads in the data from the MD2 file and stores it in the member variable
	void ReadData(FILE * m_FilePointer, JOEPACK * pack, JOEObject & Object);
};

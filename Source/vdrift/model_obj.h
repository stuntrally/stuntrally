#ifndef _MODEL_OBJ_H
#define _MODEL_OBJ_H

//#include <string>
//#include <ostream>

#include "model.h"

class MODEL_OBJ : public MODEL
{
private:
	
public:
	///returns true on success
	virtual bool Load(const std::string & filepath, std::ostream & error_log);
	virtual bool CanSave() const {return true;}
	virtual bool Save(const std::string & strFileName, std::ostream & error_output) const;
};

#endif

#pragma once
#include "model.h"

#include <string>
#include <ostream>

class MODEL_OBJ : public MODEL
{
private:
	
public:
	MODEL_OBJ(const std::string & filepath, std::ostream & error_output) : MODEL(filepath, error_output) {}
	
	///returns true on success
	virtual bool Load(const std::string & filepath, std::ostream & error_log, bool genlist = true);
	virtual bool CanSave() const {return true;}
	virtual bool Save(const std::string & strFileName, std::ostream & error_output) const;
};

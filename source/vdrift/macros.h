#pragma once

#define _SERIALIZE_(ser,varname) if (!ser.Serialize(#varname,varname)) return false
#define _SERIALIZEENUM_(ser,varname,type) if (ser.GetIODirection() == joeserialize::Serializer::DIRECTION_INPUT) {int _enumint(0);if (!ser.Serialize(#varname,_enumint)) return false;varname=(type)_enumint;} else {int _enumint = varname;if (!ser.Serialize(#varname,_enumint)) return false;}

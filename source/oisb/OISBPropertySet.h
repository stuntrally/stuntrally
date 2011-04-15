/*
The zlib/libpng License

Copyright (c) 2009-2010 Martin Preisler

This software is provided 'as-is', without any express or implied warranty. In no event will
the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial 
applications, and to alter it and redistribute it freely, subject to the following
restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that 
		you wrote the original software. If you use this software in a product, 
		an acknowledgment in the product documentation would be appreciated but is 
		not required.

    2. Altered source versions must be plainly marked as such, and must not be 
		misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __OISB_PROPERTY_SET_H__
#define __OISB_PROPERTY_SET_H__

#include "OISBGlobal.h"
#include "OISBString.h"
#include <sstream>

namespace OISB
{
	/**
	 * @brief base abstract class for all classes allowing property set/get
	 */
	class _OISBExport PropertySet
	{
		public:
			/**
			 * @brief destructor
			 */
			virtual ~PropertySet();
            
            typedef std::vector<String> PropertyList;
            virtual void listProperties(PropertyList& list);

            /**
             * @brief sets property of given name to given value
             */
            template<typename T>
            inline void setProperty(const String& name, const T& value)
            {
                impl_setProperty(name, toString(value));
            }

            /**
             * @brief gets property of given name and the result is returned as given type
             */
            template<typename T>
            inline T getProperty(const String& name) const
            {
                return fromString<T>(impl_getProperty(name));
            }

            // stub method, compiler will pick this up if possible (it has priority over the templated variant)
            inline static const String& toString(const String& value)
            {
                return value;
            }

            /// converts given type to string, idea from boost::lexical_cast
            template<typename T>
            inline static String toString(const T& value)
            {
                std::ostringstream sstr;
                sstr << value;

                return sstr.str();
            }

            // stub method, compiler will pick this up if possible (it has priority over the templated variant)
            inline static const String& fromString(const String& value)
            {
                return value;
            }

            /// converts given string to type, idea from boost::lexical_cast
            template<typename T>
            inline static T fromString(const String& value)
            {
                T ret;

                std::istringstream sstr(value);
                sstr >> ret;

                return ret;
            }

        protected:
            /**
             * @brief implementation of property set
             */
            virtual void impl_setProperty(const String& name, const String& value);

            /**
             * @brief implementation of property get
             */
            virtual String impl_getProperty(const String& name) const;
	};
}

#endif

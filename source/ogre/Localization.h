#ifndef _Localization_h_
#define _Localization_h_

#include <string>
#include <locale.h>
#include <boost/algorithm/string.hpp>


std::string getSystemLanguage()
{
	const std::string default_lang = "en";

	setlocale(LC_ALL, "");
	
	char *loc = setlocale(LC_ALL, NULL);
	if (!loc)
		return default_lang;
	if (loc == "C")
		return "en";
	// TODO: Windows?

	// We parse here only the first part of two part codes (e.g.fi_FI).
	// We can revisit this if we get regional translations.
	std::string locstr(loc);
	if (locstr.size() > 2)
	{
		locstr = locstr.substr(0, 2);
		boost::to_lower(locstr);
	}
	return locstr;
}

#endif

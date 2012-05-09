#ifndef _Localization_h_
#define _Localization_h_

#include <string>
#include <locale.h>
#include <boost/algorithm/string.hpp>

#include <OgrePlatform.h>
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	#include <Winnls.h>  //<windows.h>
#endif


static std::string getSystemLanguage()
{
	const std::string default_lang = "en";

	setlocale(LC_ALL, "");
	
	char *loc = setlocale(LC_ALL, NULL);
	if (!loc)
		return default_lang;
	if (loc == "C")
		return "en";

	//  windows only
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		char buf[256];  // loc has same result?       // English name of language
		int res = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, buf, sizeof(buf));

			if (!strcmp(buf,"English"))    loc = "en";
		else if (!strcmp(buf,"Polish"))    loc = "pl";
		else if (!strcmp(buf,"German"))    loc = "de";
		else if (!strcmp(buf,"Finnish"))   loc = "fi";
		else if (!strcmp(buf,"Romanian"))  loc = "ro";
		else if (!strcmp(buf,"French"))    loc = "fr";
		else if (!strcmp(buf,"Russian"))   loc = "ru";
	#endif

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

#pragma once
#include <string>
#include <locale.h>

std::string getSystemLanguage() {
	const std::string default_lang = "en";

	setlocale(LC_ALL, "");
	
	char *loc = setlocale(LC_ALL, NULL);
	if (!loc) return default_lang;
	if (loc == "C") return "en";
	// TODO: Windows?

	std::string locstr(loc);
	// We parse here only the first part of two part codes (e.g.fi_FI).
	// We can revisit this if we get regional translations.
	if (locstr.size() > 2)
		locstr = locstr.substr(0, 2);
	return locstr;
}

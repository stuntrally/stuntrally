#pragma once
#include <string>

std::string getSystemLanguage() {
	const std::string default_lang = "en";

	char *loc = getenv("LC_ALL");
	if (!loc) loc = getenv("LANG");
	if (!loc) return default_lang;
	// TODO: Windows?

	std::string locstr(loc);
	// We parse here only the first part of two part codes (e.g.fi_FI).
	// We can revisit this if we get regional translations.
	locstr = locstr.substr(0, 2);
	return locstr;
}

#pragma once
#include <string>

std::string getSystemLanguage() {
	const std::string default_lang = "English";
	
	char *loc = getenv("LC_ALL");
	if (!loc) loc = getenv("LANG");
	if (!loc) return default_lang;
	// TODO: Windows?

	std::string locstr(loc);
	// Parse locale
	if (locstr.find("fi_") != std::string::npos) return "Finnish";
	else if (locstr.find("de_") != std::string::npos) return "German";

	return default_lang;
}

#pragma once

/**
 * @file
 * Internet address container and helpers.
 */

#include <iostream>
#include "types.hpp"

namespace net {

	/// Convert integer IPv4 address to dot-notation
	inline std::string IPv4(uint32_t i) {
		std::ostringstream oss;
		oss << (i & 0xFF) << "." << ((i >> 8) & 0xFF) << "." << ((i >> 16) & 0xFF) << "." << ((i >> 24) & 0xFF);
		return oss.str();
	}

	/// Internet address struct
	struct Address {
		uint32_t host; ///< IPv4 address
		uint16_t port; ///< Port number
		Address(uint32_t initHost = 0, uint16_t initPort = 0): host(initHost), port(initPort) { }
		std::string str() const { return IPv4(host)+":"+boost::lexical_cast<std::string>(port); }
		bool operator==(const Address& other) { return host == other.host && port == other.port; }
		bool operator!=(const Address& other) { return !(*this == other); }
		operator bool() { return port > 0; }
	};

	inline std::ostream& operator<< (std::ostream& out, const Address& addr) {
		out << addr.str(); return out;
	}

}

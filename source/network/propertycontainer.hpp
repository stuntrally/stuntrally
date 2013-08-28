#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>

/**
 * @brief Serialization container for key-value pairs.
 *
 * PropertyContainer supports primitive data types and std::strings for values.
 * Only supported key type is string, with a 255 byte length limit.
 * Maximum size of value strings is 65535.
 * The container is _not_ written execution speed in mind.
 */
class PropertyContainer
{
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;

public:

	/**
	 * Add a primitive type to or std::string (specialized implementation) the container.
	 * @param key the key to assign for the value
	 * @param value the value to associate with the key
	 * @returns reference to itself for chaining function calls
	 */
	template<typename T> // Specialized for std::string after class definition
	PropertyContainer& add(const std::string& key, const T& value) {
		std::ostringstream oss;
		oss << value;
		pushData(key, oss.str());
		return *this;
	}

	/**
	 * Get a primitive type or std::string (specialized implementation) from the container.
	 * @param key the key
	 * @param def optional value to return in case key is not found, defaults to zero or empty string
	 * @returns the value
	 */
	template<typename T> // Specialized for std::string after class definition
	T get(const std::string& key, const T& def = T()) const {
		props_t::const_iterator it = props.find(key);
		if (it == props.end()) return def;
		T value;
		std::istringstream iss(it->second);
		iss >> value;
		return value;
	}

	/**
	 * Returns the serialized byte buffer.
	 * @returns the data
	 */
	const char* serialize() const { return data.data(); }

	/**
	 * Populates the container with the provided data. Previous data is erased.
	 * If the data contains multiples of the same key, the last one prevails.
	 * @param buffer the data buffer
	 * @param n the size of the data buffer in bytes
	 * @returns reference to itself for chaining function calls
	 */
	PropertyContainer& deserialize(const char* buffer, size_t n) {
		clear();
		size_t pos = 0;
		while (pos < n) {
			const uint8_t keySize = buffer[pos++];
			std::string key(buffer + pos, keySize);
			pos += keySize;
			const uint8_t hi = buffer[pos];
			const uint8_t lo = buffer[pos + 1];
			const uint16_t valueSize = ((int)hi << 8) + lo;
			pos += 2;
			std::string value(buffer + pos, valueSize);
			pos += valueSize;
			props[key] = value;
		}
		return *this;
	}

	/**
	 * Clears the container.
	 */
	void clear() { data.clear(); props.clear(); }

	/**
	 * The size of the data buffer in bytes.
	 * @returns the size
	 */
	size_t size() const { return data.size(); }

	/**
	 * The number of properties currently contained.
	 * @returns the count
	 */
	size_t count() const { return props.size(); }

private:
	void pushData(const std::string& key, const std::string& value) {
		uint8_t keySize = key.size();
		data.push_back(keySize);
		data += key;
		uint16_t valueSize = value.size();
		data.push_back((valueSize & 0xff00) >> 8);
		data.push_back(valueSize & 0x00ff);
		data += value;
		props[key] = value;
	}

	std::string data;
	typedef std::map<std::string, std::string> props_t;
	props_t props;
};

template<>
PropertyContainer& PropertyContainer::add(const std::string& key, const std::string& value) {
	pushData(key, value);
	return *this;
}

template<>
std::string PropertyContainer::get(const std::string& key, const std::string& def) const {
	props_t::const_iterator it = props.find(key);
	if (it == props.end()) return def;
	return it->second;
}

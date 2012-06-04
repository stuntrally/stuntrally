#ifndef SH_PROPERTYSET_H
#define SH_PROPERTYSET_H

#include <string>

namespace sh
{
	class PropertyValue
	{
	public:
		inline virtual std::string serialize () = 0;
		inline virtual void deserialize (const std::string& in) = 0;
	};

	class StringValue : public PropertyValue
	{
	public:
		StringValue (const std::string& in);
		inline virtual std::string serialize ();
		inline virtual void deserialize (const std::string& in);

		std::string mValue;
	};

	class FloatValue : public PropertyValue
	{
	public:
		FloatValue (float in);
		inline virtual std::string serialize ();
		inline virtual void deserialize (const std::string& in);

		float mValue;
	};

	class IntValue : public PropertyValue
	{
		IntValue (int in);
		inline virtual std::string serialize ();
		inline virtual void deserialize (const std::string& in);

		int mValue;
	};

	/// \todo add more (vectors 2-4 components, ...)

	/// \brief base class that allows setting properties with any kind of value-type
	class PropertySet
	{
	public:
		void setProperty (const std::string& name, const PropertyValue& value);

	protected:
		virtual void setPropertyOverride (const std::string& name, const PropertyValue& value);
	};
}

#endif

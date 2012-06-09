#ifndef SH_PROPERTYBASE_H
#define SH_PROPERTYBASE_H

#include <string>
#include <map>

#include <boost/shared_ptr.hpp>

namespace sh
{
	class PropertyValue
	{
	public:
		inline virtual std::string serialize () = 0;
		inline virtual void deserialize (const std::string& in) = 0;
	};
	typedef boost::shared_ptr<PropertyValue> PropertyPtr;

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
		void setProperty (const std::string& name, PropertyPtr value);

	protected:
		virtual bool setPropertyOverride (const std::string& name, PropertyPtr value);
		///< @return \a true if the specified property was found, or false otherwise
	};

	typedef std::map<std::string, PropertyPtr> PropertyMap;

	/// \brief base class that allows setting properties with any kind of value-type and retrieving them
	class PropertySetGet
	{
	public:
		void setProperty (const std::string& name, PropertyPtr value);
		PropertyPtr getProperty (const std::string& name);

	protected:
		PropertyMap mProperties;
	};
}

#endif

#ifndef SH_PROPERTYBASE_H
#define SH_PROPERTYBASE_H

#include <string>
#include <map>

#include <boost/shared_ptr.hpp>

namespace sh
{
	class StringValue;

	class PropertyValue
	{
	public:
		PropertyValue() {}

		template <typename T>
		static T* retrieve (boost::shared_ptr<PropertyValue>& value)
		{
			if (typeid(T).name() == typeid(*value).name())
			{
				// requested type is the same as source type, only have to cast it
				return static_cast<T*>(value.get());
			}

			if ((typeid(T).name() == typeid(StringValue).name())
				&& typeid(*value).name() != typeid(StringValue).name())
			{
				// if string type is requested and value is not string, use serialize method to convert to string
				T* ptr = new T (value->serialize()); // note that T is always StringValue here, but we can't use it because it's not declared yet
				value = boost::shared_ptr<PropertyValue> (static_cast<PropertyValue*>(ptr));
				return ptr;
			}

			{
				// remaining case: deserialization from string by passing the string to constructor of class T
				T* ptr = new T(value->_getStringValue());
				boost::shared_ptr<PropertyValue> newVal (static_cast<PropertyValue*>(ptr));
				value = newVal;
				return ptr;
			}
		}
		///<
		/// @brief converts \a value to the type \a T, deserializing or serializing from/to strings if necessary \n
		/// example usage: int myNumber = PropertyValue::retrieve <IntValue> (val)->get();
		/// @note \a value is changed in-place to the converted object
		/// @return pointer to converted object \n

		std::string _getStringValue() { return mStringValue; }

		virtual std::string serialize() = 0;

	protected:
		std::string mStringValue; ///< this will possibly not contain anything in the specialised classes
	};
	typedef boost::shared_ptr<PropertyValue> PropertyValuePtr;


	template <typename T>
	inline PropertyValuePtr makeProperty (T* prop)
	{
		return PropertyValuePtr (static_cast<PropertyValue*> (prop));
	}

	class StringValue : public PropertyValue
	{
	public:
		StringValue (const std::string& in);
		std::string get() const { return mStringValue; }

		virtual std::string serialize();
	};

	class FloatValue : public PropertyValue
	{
	public:
		FloatValue (float in);
		FloatValue (const std::string& in);
		float get() const { return mValue; }

		virtual std::string serialize();
	private:
		float mValue;
	};

	class IntValue : public PropertyValue
	{
	public:
		IntValue (int in);
		IntValue (const std::string& in);
		int get() const { return mValue; }

		virtual std::string serialize();
	private:
		int mValue;
	};

	class BooleanValue : public PropertyValue
	{
	public:
		BooleanValue (bool in);
		BooleanValue (const std::string& in);
		bool get() const { return mValue; }

		virtual std::string serialize();
	private:
		bool mValue;
	};

	class Vector2 : public PropertyValue
	{
	public:
		Vector2 (float x, float y);
		Vector2 (const std::string& in);

		int mX, mY;

		virtual std::string serialize();
	};

	class Vector3 : public PropertyValue
	{
	public:
		Vector3 (float x, float y, float z);
		Vector3 (const std::string& in);

		int mX, mY, mZ;

		virtual std::string serialize();
	};

	class Vector4 : public PropertyValue
	{
	public:
		Vector4 (float x, float y, float z, float w);
		Vector4 (const std::string& in);

		int mX, mY, mZ, mW;

		virtual std::string serialize();
	};

	/// \brief base class that allows setting properties with any kind of value-type
	class PropertySet
	{
	public:
		void setProperty (const std::string& name, PropertyValuePtr& value);

	protected:
		virtual bool setPropertyOverride (const std::string& name, PropertyValuePtr& value);
		///< @return \a true if the specified property was found, or false otherwise
	};

	typedef std::map<std::string, PropertyValuePtr> PropertyMap;

	/// \brief base class that allows setting properties with any kind of value-type and retrieving them
	class PropertySetGet
	{
	public:
		PropertySetGet (PropertySetGet* parent);
		PropertySetGet ();

		void setParent (PropertySetGet* parent); ///< throws an exception if there is already a parent

		void setProperty (const std::string& name, PropertyValuePtr value);
		PropertyValuePtr& getProperty (const std::string& name);

	private:
		PropertyMap mProperties;

		PropertySetGet* mParent;
		///< the parent can provide properties as well (when they are retrieved via getProperty) \n
		/// multiple levels of inheritance are also supported
		/// children can override properties of their parents
	};
}

#endif

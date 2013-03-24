#ifndef SH_ACTIONS_H
#define SH_ACTIONS_H

#include <string>

namespace sh
{

	class Action
	{
	public:
		virtual void execute() = 0;
	};

	class ActionDeleteMaterial : public Action
	{
	public:
		ActionDeleteMaterial(const std::string& name)
			: mName(name) {}

		virtual void execute();
	private:
		std::string mName;
	};

	class ActionCloneMaterial : public Action
	{
	public:
		ActionCloneMaterial(const std::string& sourceName, const std::string& destName)
			: mSourceName(sourceName), mDestName(destName) {}

		virtual void execute();
	private:
		std::string mSourceName;
		std::string mDestName;
	};

	class ActionSaveAll : public Action
	{
	public:
		virtual void execute();
	};

	class ActionChangeGlobalSetting : public Action
	{
	public:
		ActionChangeGlobalSetting(const std::string& name, const std::string& newValue)
			: mName(name), mNewValue(newValue) {}

		virtual void execute();
	private:
		std::string mName;
		std::string mNewValue;
	};

	class ActionCreateConfiguration : public Action
	{
	public:
		ActionCreateConfiguration(const std::string& name)
			: mName(name) {}

		virtual void execute();
	private:
		std::string mName;

	};

	class ActionDeleteConfiguration : public Action
	{
	public:
		ActionDeleteConfiguration(const std::string& name)
			: mName(name) {}

		virtual void execute();
	private:
		std::string mName;

	};

	class ActionChangeConfiguration : public Action
	{
	public:
		ActionChangeConfiguration (const std::string& name, const std::string& key, const std::string& value)
			: mName(name), mKey(key), mValue(value) {}

		virtual void execute();
	private:
		std::string mName;
		std::string mKey;
		std::string mValue;
	};

	class ActionDeleteConfigurationProperty : public Action
	{
	public:
		ActionDeleteConfigurationProperty (const std::string& name, const std::string& key)
			: mName(name), mKey(key) {}

		virtual void execute();
	private:
		std::string mName;
		std::string mKey;
	};

}

#endif

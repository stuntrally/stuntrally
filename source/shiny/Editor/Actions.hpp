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

}

#endif

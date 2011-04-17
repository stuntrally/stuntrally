/*
The zlib/libpng License

Copyright (c) 2009-2010 Martin Preisler

This software is provided 'as-is', without any express or implied warranty. In no event will
the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial 
applications, and to alter it and redistribute it freely, subject to the following
restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that 
		you wrote the original software. If you use this software in a product, 
		an acknowledgment in the product documentation would be appreciated but is 
		not required.

    2. Altered source versions must be plainly marked as such, and must not be 
		misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#include "OISBSystem.h"
#include "OISBMouse.h"
#include "OISBKeyboard.h"
#include "OISBJoyStick.h"
#include "OISBActionSchema.h"
#include "OISBAction.h"
#include "OISBState.h"

#include "OISException.h"
#include "OISInputManager.h"
#include "OISKeyboard.h"
#include "OISMouse.h"
#include "OISJoyStick.h"

#include "OISBAnalogAxisAction.h"
#include "OISBSequenceAction.h"
#include "OISBTriggerAction.h"

#include <strstream>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace rapidxml;

namespace OISB
{
    // static member singleton pointer
    System* System::msSingleton = 0;

	System::System():
		mInitialized(false),

		mOIS(0),
		mOISMouse(0),
		mOISKeyboard(0),
		mOISJoysticks(),
		mMouse(0),
		mKeyboard(0),
		mJoysticks(),
        mDefaultActionSchema(0)
	{
		 msSingleton = this;
	}
 
	System::~System()
	{
		if (mActionSchemas.size() > 0)
		{
			while (mActionSchemas.size() > 0)
			{
				destroyActionSchema(mActionSchemas.begin()->first);
			}
		}

		msSingleton = 0;
	}
 
	void System::initialize(OIS::InputManager* ois)
	{
		if (mInitialized)
		{
			return;
		}

		mOIS = ois;
 
		// If possible create a buffered keyboard
		if (mOIS->getNumberOfDevices(OIS::OISKeyboard) > 0)
		{
			mOISKeyboard = static_cast<OIS::Keyboard*>(mOIS->createInputObject(OIS::OISKeyboard, true));
			mKeyboard = new Keyboard(mOISKeyboard);
			addDevice(mKeyboard);
		}
 
		// If possible create a buffered mouse
		if (mOIS->getNumberOfDevices(OIS::OISMouse) > 0)
		{
			mOISMouse = static_cast<OIS::Mouse*>(mOIS->createInputObject(OIS::OISMouse, true));
			mMouse = new Mouse(mOISMouse);
			addDevice(mMouse);
		}

		// get all joysticks
		int num_joy = mOIS->getNumberOfDevices(OIS::OISJoyStick);
		if (num_joy > 0)
		{
			mOISJoysticks.resize(num_joy);
			mJoysticks.resize(num_joy);
			for(int i=0; i < num_joy; i++)
			{
				mOISJoysticks[i] = static_cast<OIS::JoyStick*>(mOIS->createInputObject(OIS::OISJoyStick, true));
				mJoysticks[i]    = new JoyStick(mOISJoysticks[i]);
				addDevice(mJoysticks[i]);
			}
		}

		mInitialized = true;
	}

	void System::finalize()
	{
		if (!mInitialized)
		{
			return;
		}

		if (mOISMouse)
		{
			removeDevice(mMouse);
			delete mMouse;
			mMouse = 0;
			mOIS->destroyInputObject(mOISMouse);
			mOISMouse = 0;
		}
 
		if (mOISKeyboard)
		{
			removeDevice(mKeyboard);
			delete mKeyboard;
			mKeyboard = 0;
			mOIS->destroyInputObject(mOISKeyboard);
			mOISKeyboard = 0;
		}
 
		// remove all joysticks
		for(unsigned int i=0; i < mOISJoysticks.size(); i++)
		{
			removeDevice(mJoysticks[i]);
			delete(mJoysticks[i]);

			mOIS->destroyInputObject(mOISJoysticks[i]);
		}
		mJoysticks.clear();
		mOISJoysticks.clear();

		mOIS->destroyInputSystem(mOIS);
		mOIS = 0;

		mInitialized = false;
	}
 
	void System::process(Real delta)
	{
		mOISMouse->capture();
		mOISKeyboard->capture();
		for(unsigned int i=0; i < mOISJoysticks.size(); i++)
			mOISJoysticks[i]->capture();

		for (DeviceMap::const_iterator it = mDevices.begin(); it != mDevices.end(); ++it)
		{
			it->second->process(delta);
		}

		for (ActionSchemaMap::const_iterator it = mActionSchemas.begin(); it != mActionSchemas.end(); ++it)
        {
            it->second->process(delta);
        }
	}
	
	Device* System::getDevice(const String& name) const
	{
		DeviceMap::const_iterator it = mDevices.find(name);
		
		if (it == mDevices.end())
		{
			OIS_EXCEPT(OIS::E_General, String("Device '" + name + "' not found!").c_str());
		}
		
		return it->second;
	}

    bool System::hasDevice(const String& name) const
    {
        DeviceMap::const_iterator it = mDevices.find(name);
		
		return it != mDevices.end();
    }

    State* System::lookupState(const String& name) const
    {
        String::size_type i = name.find("/");
        const String deviceName = name.substr(0, i); // -1 because we want to copy just before the "/"

        if (hasDevice(deviceName))
        {
            Device* dev = getDevice(deviceName);
            const String stateName = name.substr(i + 1);

            if (dev->hasState(stateName))
            {
                return dev->getState(stateName);
            }
        }

        // nothing was found
        return 0;
    }

	int System::loadActionSchemaFromXMLFile(const String& filename)
	{
		// read the xml file
		std::string input_xml;
		std::string line;
		std::ifstream myfile (filename.c_str());
		if ( myfile.is_open() )
		{
			while (! myfile.eof() )
			{
				std::getline (myfile,line);
				input_xml += line;
			}
		} else
		{
			return 1;
		}
		myfile.close();
		std::vector<char> xml_copy(input_xml.begin(), input_xml.end());
		xml_copy.push_back('\0');
		return loadActionSchemaFromXML(&xml_copy[0]);
	}

	int System::loadActionSchemaFromXML(const char *xmlContent)
	{
		rapidxml::xml_document<> xmlActionSchema;
		xmlActionSchema.parse<rapidxml::parse_declaration_node | rapidxml::parse_no_data_nodes>(const_cast<char *>(xmlContent));

		/// walk the schemas
		rapidxml::xml_node<>* schemasNode = xmlActionSchema.first_node("schemas");
		if(schemasNode)
		{
			for (rapidxml::xml_node<> *child = schemasNode->first_node("schema"); child; child = child->next_sibling())
			{
				processSchemaXML(child);
			}
		}
		return 0;
	}

	int System::processSchemaXML(rapidxml::xml_node<>* schemaNode)
	{
		if(!schemaNode) return 1;
		ActionSchema* schema=0;

		// check for missing name
		if(!schemaNode->first_attribute("name")) return 2;

		std::string schema_name = schemaNode->first_attribute("name")->value();
		if (!hasActionSchema(schema_name))
			schema = createActionSchema(schema_name, false);
		else
			schema = getActionSchema(schema_name);
		
		// then process the actions
        for (rapidxml::xml_node<> *child = schemaNode->first_node("action"); child; child = child->next_sibling())
			processActionXML(child, schema);

		return 0;
	}

	int System::processActionXML(rapidxml::xml_node<>* actionNode, ActionSchema* schema)
	{
		if(!actionNode) return 1;

		std::string type="", name="";

		Action *tmpAction = 0;
		// process all attributes
		for (xml_attribute<> *attr = actionNode->first_attribute(); attr; attr = attr->next_attribute())
		{
			// beware, this assumes that the type and name comes first
			if     (!strcmp(attr->name(), "type")) type = std::string(attr->value());
			else if(!strcmp(attr->name(), "name")) name = std::string(attr->value());
			else if(tmpAction)
			{
				tmpAction->setProperty(std::string(attr->name()), std::string(attr->value()));
			}

			// check if we can create the object already
			if(!type.empty() && !name.empty() && !tmpAction)
			{
				if     (type == "AnalogAxis")
					tmpAction = schema->createAction<OISB::AnalogAxisAction>(name);
				else if(type == "Sequence")
					tmpAction = schema->createAction<OISB::SequenceAction>(name);
				else if(type == "Trigger")
					tmpAction = schema->createAction<OISB::TriggerAction>(name);
			}
		}

		// then process the child bindings
        for (rapidxml::xml_node<> *child = actionNode->first_node("bind"); child; child = child->next_sibling())
			processActionBindingXML(child, tmpAction);

		return 0;
	}

	int System::processActionBindingXML(rapidxml::xml_node<>* bindNode, Action *action)
	{
		if(!bindNode || !action) return 1;

		bool optional = false;
		if(bindNode->first_attribute("optional"))
			optional = true;

		try
		{
			action->bind(std::string(bindNode->value()));
		} catch(const OIS::Exception &ex)
		{
			// rethrow if this binding is not optional
			if(!optional)
				throw(ex);
		}
		return 0;
	}


    ActionSchema* System::createActionSchema(const String& name, bool setAsDefault)
	{
		ActionSchemaMap::const_iterator it = mActionSchemas.find(name);
		
		if (it != mActionSchemas.end())
		{
            OIS_EXCEPT(OIS::E_Duplicate, String("Action schema '" + name + "' already exists").c_str());
		}
		
		ActionSchema* ret = new ActionSchema(name);
		mActionSchemas.insert(std::make_pair(name, ret));

		if (setAsDefault)
		{
			setDefaultActionSchema(ret);
		}

		return ret;
	}

	void System::destroyActionSchema(const String& name)
	{
		ActionSchemaMap::iterator it = mActionSchemas.find(name);
		
		if (it == mActionSchemas.end())
		{
            OIS_EXCEPT(OIS::E_General, String("Action schema '" + name + "' not found").c_str());
		}

		ActionSchema* schema = it->second;
		mActionSchemas.erase(it);

        if (schema == mDefaultActionSchema)
        {
            setDefaultActionSchema(0);
        }

		delete schema;
	}
	
	void System::destroyActionSchema(ActionSchema* schema)
	{
		destroyActionSchema(schema->getName());
	}

	ActionSchema* System::getActionSchema(const String& name) const
	{
		ActionSchemaMap::const_iterator it = mActionSchemas.find(name);
		
		if (it == mActionSchemas.end())
		{
            OIS_EXCEPT(OIS::E_General, String("Action schema '" + name + "' not found").c_str());
		}

		return it->second;
	}

    bool System::hasActionSchema(const String& name) const
    {
        ActionSchemaMap::const_iterator it = mActionSchemas.find(name);
		
		return it != mActionSchemas.end();
    }

	void System::setDefaultActionSchema(ActionSchema* schema)
	{
		mDefaultActionSchema = schema;
	}

	void System::setDefaultActionSchema(const String& name)
	{
		setDefaultActionSchema(getActionSchema(name));
	}

	ActionSchema* System::getDefaultActionSchemaAutoCreate()
	{
		if (!mDefaultActionSchema)
		{
			createActionSchema("Default", true);
		}

		return getDefaultActionSchema();
	}

    Action* System::lookupAction(const String& name, bool throwOnMissing) const
    {
        String::size_type i = name.find("/");
        const String schemaName = name.substr(0, i); // -1 because we want to copy just before the "/"

        if (hasActionSchema(schemaName))
        {
            ActionSchema* schema = getActionSchema(schemaName);
            const String actionName = name.substr(i + 1);

            if (schema->hasAction(actionName))
            {
                return schema->getAction(actionName);
            }
            else
			{
				if (throwOnMissing)
				{
					String errorString = String("Action '") + actionName + String("' in schema '") + schemaName + String("' not found");
					OIS_EXCEPT(OIS::E_General, errorString.c_str());
				}
			}
        }
        else
		{
			if (throwOnMissing)
			{
				String errorString = String("Action schema '") + schemaName + String("' not found");
				OIS_EXCEPT(OIS::E_General, errorString.c_str());
			}
		}

        // nothing was found
        return 0;
    }

    Bindable* System::lookupBindable(const String& name) const
    {
        State* state = lookupState(name);
        if (state)
        {
            return state;
        }

        Action* action = lookupAction(name, false);
        if (action)
        {
            return action;
        }

        // nothing was found, return 0
        return 0;
    }

    void System::addListenerToAllStates(BindableListener* listener)
    {
        for (DeviceMap::const_iterator it = mDevices.begin(); it != mDevices.end(); ++it)
        {
            it->second->addListenerToAllStates(listener);
        }
    }

    void System::removeListenerFromAllStates(BindableListener* listener)
    {
        for (DeviceMap::const_iterator it = mDevices.begin(); it != mDevices.end(); ++it)
        {
            it->second->removeListenerFromAllStates(listener);
        }
    }

    void System::addListenerToAllActions(BindableListener* listener)
    {
        for (ActionSchemaMap::const_iterator it = mActionSchemas.begin(); it != mActionSchemas.end(); ++it)
        {
            it->second->addListenerToAllActions(listener);
        }
    }

    void System::removeListenerFromAllActions(BindableListener* listener)
    {
        for (ActionSchemaMap::const_iterator it = mActionSchemas.begin(); it != mActionSchemas.end(); ++it)
        {
            it->second->removeListenerFromAllActions(listener);
        }
    }

    void System::addListenerToAllBindables(BindableListener* listener)
    {
        addListenerToAllStates(listener);
        addListenerToAllActions(listener);
    }

    void System::removeListenerFromAllBindables(BindableListener* listener)
    {
        removeListenerFromAllActions(listener);
        removeListenerFromAllStates(listener);
    }

    void System::dumpDevices()
    {
        std::cout << "Dumping all registered OISB devices: " << std::endl;

        for (DeviceMap::const_iterator it = mDevices.begin(); it != mDevices.end(); ++it)
        {
            it->second->dump();
        }

        std::cout << "End of dump!" << std::endl;
    }

    void System::dumpActionSchemas()
    {
        std::cout << "Dumping all registered OISB action schemas: " << std::endl;

        for (ActionSchemaMap::const_iterator it = mActionSchemas.begin(); it != mActionSchemas.end(); ++it)
        {
            it->second->dump();
        }

        std::cout << "End of dump!" << std::endl;
    }
	
	void System::addDevice(Device* device)
	{
		DeviceMap::const_iterator it = mDevices.find(device->getName());
		
		if (it != mDevices.end())
		{
			OIS_EXCEPT(OIS::E_Duplicate, String("Device with name '" + device->getName() + "' already exists!").c_str());
		}
		
		mDevices.insert(std::make_pair(device->getName(), device));
	}
	
	void System::removeDevice(Device* device)
	{
		DeviceMap::iterator it = mDevices.find(device->getName());
		
		if (it == mDevices.end())
		{
			OIS_EXCEPT(OIS::E_General, String("Device '" + device->getName() + "' not found!").c_str());
		}
		
		mDevices.erase(it);
	}
}

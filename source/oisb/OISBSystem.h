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

#ifndef __OISB_SYSTEM_H__
#define __OISB_SYSTEM_H__

#include "OISBGlobal.h"
#include "OISBString.h"

#include "rapidxml.hpp"

#include <cassert>

namespace OISB
{
    /**
     * @brief this is the centerpoint of OISB
     */
	class _OISBExport System
	{
		public:
            /// @brief constructor
			System();

            /// @brief destructor
			~System();

            /**
             * @brief singleton retrieval method (reference)
             */
            inline static System& getSingleton()
            {
                assert(msSingleton);

                return *msSingleton;
            }

            /**
             * @brief singleton retrieval method (pointer)
             */
            inline static System* getSingletonPtr()
            {
                assert(msSingleton);

                return msSingleton;
            }
		 
            /**
             * @brief initializes the System with given OIS
             */
			void initialize(OIS::InputManager* ois);

            /**
             * @brief finalizes the system
             */
			void finalize();

            /**
             * @brief processes everything (call this every frame!)
             *
             * @param delta time delta since last processing
             */
			void process(Real delta);
		 
            /// @brief returns the OIS mouse
			inline OIS::Mouse* getOISMouse()
			{
				return mOISMouse;
			}

            /// @brief returns the OIS keyboard
			inline OIS::Keyboard* getOISKeyboard()
			{
				return mOISKeyboard;
			}

			/**
			 * @brief retrieves input device
			 * 
			 * @param name name to lookup
			 * @return pointer to resulting input device
			 */
			Device* getDevice(const String& name) const;

            /**
             * @brief checks whether given device exists
             *
             * @param name name of the device to check
             */
            bool hasDevice(const String& name) const;
            
            /**
             * @brief this method tries to lookup a state by it's full name
             *
             * @param name name of the action (e.g. Keyboard/A)
             * @return pointer to the discovered state (0 if none is found)
             */
            State* lookupState(const String& name) const;

            /**
			 * @brief creates a new action schema
			 * 
			 * @param name name of the schema
             * @param setAsDefault if true the new schema is automatically set as the default oner
			 * @return pointer to the newly created schema
			 */
			ActionSchema* createActionSchema(const String& name, bool setAsDefault = true);
			
			/**
			 * @brief destroys previously created action schema
			 * 
			 * @param name name to lookup
			 */
			void destroyActionSchema(const String& name);
			
			/**
			 * @brief destroys previously created action schema
			 * 
			 * @param schema schema to destroy
			 */
			void destroyActionSchema(ActionSchema* schema);

            /**
             * @brief retrieves action schema with given name
             */
			ActionSchema* getActionSchema(const String& name) const;

            /**
             * @brief checks whether action schema with given name exists
             */
            bool hasActionSchema(const String& name) const;

            /**
             * @brief sets given action schema as the default one
             */
			void setDefaultActionSchema(ActionSchema* schema);
	
            /**
             * @brief sets action schema with given name as the default one
             */
			void setDefaultActionSchema(const String& name);
			
            /**
             * @brief retrieves default action schema
             *
             * @note this can return 0 if no default action schema is set!
             */
			inline ActionSchema* getDefaultActionSchema() const
			{
				return mDefaultActionSchema;
			}

            /**
             * @brief retrieves default action schema, if it doesn't exist, it creates one
             *
             * @note this never returns 0
             */
			ActionSchema* getDefaultActionSchemaAutoCreate();

            /**
             * @brief this method tries to lookup an action by it's full name
             *
             * @param name name of the action (e.g. Ingame/Shoot)
             * @return pointer to the discovered action (0 if none is found)
             */
            Action* lookupAction(const String& name, bool throwOnMissing = true) const;

            /**
             * @brief this method tries to lookup a bindable by it's full name (works for states and actions)
             *
             * @param name name of the bindable (e.g. Keyboard/A or Ingame/Shoot)
             * @return pointer to the discovered bindable (0 if none is found)
             */
            Bindable* lookupBindable(const String& name) const;

            /**
             * @brief adds a listener to all input states in all devices
             *
             * @note this is especially useful when you want to do ingame controls setup or similar,
             *       you just listen to all states and pick up the ones activated
             */
            void addListenerToAllStates(BindableListener* listener);

            /**
             * @brief removes previously added listener from all input states in all devices
             */
            void removeListenerFromAllStates(BindableListener* listener);

            /**
             * @brief adds a listener to all actions in all schemas
             */
            void addListenerToAllActions(BindableListener* listener);

            /**
             * @brief removes previously added listener from all actions in all schemas
             */
            void removeListenerFromAllActions(BindableListener* listener);

            /**
             * @brief adds a listener to all bindables (states & actions)
             */
            void addListenerToAllBindables(BindableListener* listener);

            /**
             * @brief removes previously added listener from all bindables (states & actions)
             */
            void removeListenerFromAllBindables(BindableListener* listener);

			/**
			 * @brief loads schemas and actions from a char string
			 */
			int loadActionSchemaFromXML(const char *xmlContent);

			/**
			 * @brief loads schemas and actions from an xml file
			 */
			int loadActionSchemaFromXMLFile(const String& filename);

			/**
             * @brief a method to ease debugging, dumps all actions schemas to stdout
             */
            void dumpDevices();

			/**
			 * @brief a method to ease debugging, dumps all actions schemas to stdout
			 */
			void dumpActionSchemas();

		private:
            /// singleton implementation pointer
            static System* msSingleton;

            /// true if the system was initialized
			bool mInitialized;

            /// OIS input manager
			OIS::InputManager* mOIS;

            /// OIS mouse, OISB::Mouse wraps this
			OIS::Mouse* mOISMouse;
            /// OIS keyboard, OISB::Keyboard wraps this
			OIS::Keyboard* mOISKeyboard;
            /// OIS joystick, OISB::Joystick wraps this
			std::vector<OIS::JoyStick*> mOISJoysticks;

            /// our wrap mouse device
			Mouse* mMouse;
            /// our wrap keyboard device
			Keyboard* mKeyboard;
            /// our wrap keyboard device
			std::vector<JoyStick*> mJoysticks;

			/**
			 * @brief adds input device
			 * 
			 * @param device device to add
			 */
			void addDevice(Device* device);
			
			/**
			 * @brief removes previously added input device
			 * 
			 * @param device device to remove
			 */
			void removeDevice(Device* device);
			
			typedef std::map<String, Device*> DeviceMap;
			/// stores input devices of this input system
			DeviceMap mDevices;

            /// stores pointer to the default action schema
			ActionSchema* mDefaultActionSchema;

			typedef std::map<String, ActionSchema*> ActionSchemaMap;
			/// stores all action schemas
			ActionSchemaMap mActionSchemas;

			// xml processing things, should be moved to separate class later on
			int processSchemaXML(rapidxml::xml_node<>* schemaNode);
			int processActionXML(rapidxml::xml_node<>* actionNode, ActionSchema* schema);
			int processActionBindingXML(rapidxml::xml_node<>* bindNode, Action *action);

	};
}

#endif

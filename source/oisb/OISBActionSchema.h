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

#ifndef __OISB_ACTION_SCHEMA_H__
#define __OISB_ACTION_SCHEMA_H__

#include "OISBGlobal.h"
#include "OISBAction.h"

namespace OISB
{
	/**
	 * @brief class that manages actions
	 */
	class _OISBExport ActionSchema
	{
		public:
			/**
			 * @brief constructor
			 * 
			 * @param name name / identifier
			 */
			ActionSchema(const String& name);
			
			/**
			 * @brief destructor
			 */
			~ActionSchema();
			
			/**
			 * @brief retrieves name of the action schema
			 * 
			 * @return name / identifier of this action schema
			 */
			inline const String& getName() const
			{
				return mName;
			}
			
			/**
			 * @brief sets whether this action schema is enabled or not
			 * 
			 * @param enabled if true, this action schema is enabled
			 */
			void setEnabled(bool enabled);
			
			/**
			 * @brief checks whether this action schema is enabled or not
			 * 
			 * @return true if this action schema is enabled
			 */
			inline bool isEnabled() const
			{
				return mIsEnabled;
			}
			
			/**
			 * @brief creates a new action
			 * 
             * @param type type of action
			 * @param name name of the new action
			 * @return pointer to the newly created action
			 */
			Action* createAction(ActionType type, const String& name);

             /**
             * @brief creates a new action and casts it to the specified action type.
             * 
             * @tparam T type of action
             * @param name name of the new action
             * @return pointer to the newly created action
             */
            template<class T>
            inline T* createAction(const String& name)
            {
                return static_cast<T*>(
                    createAction(T::ACTION_TYPE, name)
                );
            }
			
			/**
			 * @brief destroys previously created action
			 * 
			 * @param name name to lookup
			 */
			void destroyAction(const String& name);
			
			/**
			 * @brief destroys previously created action
			 * 
			 * @param action action to lookup
			 */
			void destroyAction(Action* action);

            /**
             * @brief retrieves action by given name
             */
            Action* getAction(const String& name) const;

            /**
             * @brief checks whether action with given name is contained in this schema
             */
            bool hasAction(const String& name) const;

            /**
             * @brief adds listener to all actions within this schema
             */
            void addListenerToAllActions(BindableListener* listener);

            /**
             * @brief removes previously added listener from all actions within this schema
             */
            void removeListenerFromAllActions(BindableListener* listener);
			
			/**
			 * @brief inactivate all actions of this schema
			 */
			void process(Real delta);
			
			/**
			 * @brief a method to ease debugging, dumps this action schema into stdout
			 */
			void dump();

		private:
			/// stores name / identifier
			const String mName;
			/// if true, this action schema is enabled
			bool mIsEnabled;
			
			typedef std::map<String, Action*> ActionMap;
			/// stores actions inside this schema
			ActionMap mActions;
	};
}

#endif

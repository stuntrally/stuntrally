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

#ifndef __OISB_ACTION_H__
#define __OISB_ACTION_H__

#include "OISBGlobal.h"
#include "OISBBindable.h"

namespace OISB
{
    /// @brief enumerates possible action types, serves as a simple RTTI
    enum ActionType
    {
        /// simplest form of action, all active states in at least one binding activate this
        AT_TRIGGER,
        /// "iddqd" type of action, you have to activate and deactivate all the states in the binding in a sequence
        AT_SEQUENCE,

        /// analog action, contains one floating point number (e.g. steering)
        AT_ANALOG_AXIS
    };

	/**
	 * @brief abstract element that bindables can be bound to
     *
     * @par
     * By using actions instead of states for your input, you can allow users to
     * remap keys or even map analog input devices to your actions!
	 */
	class _OISBExport Action : public Bindable
	{
		public:
			/**
			 * @brief constructor
			 * 
			 * @param parent parent action schema
			 * @param name name / identifier
			 */
			Action(ActionSchema* parent, const String& name);
			
			/**
			 * @brief destructor
			 */
			virtual ~Action();

            /// @copydoc Bindable::getBindableType
            virtual BindableType getBindableType() const;

            /// @copydoc Bindable::getBindableName
            virtual String getBindableName() const;

            /// @copydoc Bindable::isActive
			virtual bool isActive() const;

            /// @copydoc Bindable::hasChanged
            virtual bool hasChanged() const;
			
            /**
             * @brief gets the parent action schema
             */
			inline ActionSchema* getParent() const
			{
				return mParent;
			}

			/**
			 * @brief retrieves name of this action
			 * 
			 * @return name / identifier of this action
			 */
			inline const String& getName() const
			{
				return mName;
			}

			/**
			 * @brief retrieves full name of this action (including parent schema)
			 * 
			 * @return full name of this action
			 */
			String getFullName() const;

            /**
             * @brief actions are divided into types, this returns the type of this action
             */
            virtual ActionType getActionType() const = 0;

            /**
             * @brief creates and adds binding to this action
             *
             * @see Binding
             */
            virtual Binding* createBinding();

            /**
             * @brief removes and destroys binding of this action
             */
            virtual void destroyBinding(Binding* binding);

            /**
             * @brief convenience method, creates one binding and binds one given bindable to it
             */
            void bind(Bindable* bindable);

            /**
             * @brief convenience method, creates one binding and binds two given bindables to it
             */
            void bind(Bindable* bindable1, Bindable* bindable2);

            /**
             * @brief convenience method, creates one binding and binds three given bindables to it
             */
            void bind(Bindable* bindable1, Bindable* bindable2, Bindable* bindable3);

            /**
             * @brief convenience method, creates one binding and binds one given bindable to it
             */
            void bind(const String& bindable);

            /**
             * @brief convenience method, creates one binding and binds two given bindables to it
             */
            void bind(const String& bindable1, const String& bindable2);

            /**
             * @brief convenience method, creates one binding and binds three given bindables to it
             */
            void bind(const String& bindable, const String& bindable2, const String& bindable3);
			
			/**
			 * @brief processes given input state and sets active status accordingly
			 * 
			 * @param delta time delta from last processing
			 */
			void process(Real delta);

            /**
             * @brief nasty but handy, this destroys the action using parent action schema
             */
			void destroyItself();

            /// @copydoc PropertySet::listProperties
            virtual void listProperties(PropertyList& list);
        
        protected:
            /// @copydoc PropertySet::setProperty
            virtual void impl_setProperty(const String& name, const String& value);

            /// @copydoc PropertySet::getProperty
            virtual String impl_getProperty(const String& name) const;
			
            /// internal method that activates this action
            void activate();
            /// internal method that deactivates this action
            void deactivate();
            
            /// internal implementation process method, every action has to override this
            virtual bool impl_process(Real delta) = 0;

			/// stores parent schema
			ActionSchema* mParent;
			/// name / identifier
			const String mName;
			
			/// stores whether this action is active or not, true if active
			bool mIsActive;
            /// stores whether the active state has changed or not
            bool mChanged;

            typedef std::vector<Binding*> BindingList;
            /// alternative bindings of this action
            BindingList mBindings;
	};
}

#endif

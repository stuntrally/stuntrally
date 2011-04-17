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

#ifndef __OISB_BINDING_H__
#define __OISB_BINDING_H__

#include "OISBGlobal.h"
#include "OISBString.h"

#include <list>

namespace OISB
{
    /**
     * @brief stores list of states
     *
     * Binding is just a simple class that contains states, whether the parent
     * action uses them as combination or sequence or anything else is completely
     * in the hands of it
     *
     * The main feature and reason why this is needed and not contained in the
     * Action class is to not repeat code and allow multiple alternatives to
     * be bound to a single Action.
     */
	class _OISBExport Binding
	{
		public:
			/**
			 * @brief constructor
			 * 
			 * @param parent parent action schema
			 * @param name name / identifier
			 * @param type type of binding
			 */
			Binding(Action* parent);
			
			/**
			 * @brief destructor
			 */
			~Binding();

            /**
             * @brief internal method to set this binding as active or not
             *
             * @note do not use directly! Used by Actions
             */
            void _setActive(bool active);

            /**
             * @brief checks whether this binding is active
             *
             * @note the meaning of active is dependant on used action
             */
            inline bool isActive() const
            {
                return mIsActive;
            }
			
            /**
             * @brief binds another bindable to this binding
             *
             * @param bindable The Bindable (Action/State) to bind
             * @param role role of the new bindable (optional), usage depends on Action
             */
			void bind(Bindable* bindable, const String& role = "");

            /**
             * @brief binds another bindable to this binding
             *
             * @param bindable The Bindable's name (Action/State) to bind
             * @param role role of the new bindable (optional), usage depends on Action
             */
            void bind(const String& bindable, const String& role = "");

            /**
             * @brief unbinds bound bindable
             *
             * @param bindable The Bindable to unbind
             */
            void unbind(Bindable* bindable);

            /**
             * @brief unbinds bound bindable by name
             *
             * @param bindable The Bindable to unbind
             */
            void unbind(const String& bindable);

            /**
             * @brief checks whether given bindable is bound to this binding
             */
            bool isBound(Bindable* bindable) const;

            /**
             * @brief checks whether at least one bindable wit given name is bound to this binding
             */
            bool isBound(const String& role) const;

            /**
             * @brief retrieves number of bindables bound to this binding
             */
            inline size_t getNumBindables() const
            {
                return mBindables.size();
            }

            /**
             * @brief retrieves bindable at given index
             */
            Bindable* getBindable(size_t idx) const;

            /**
             * @brief retrieves bound state at given index
             *
             * @note this will throw exception if bindable at given index is not a state!
             */
            State* getState(size_t idx) const;

            /**
             * @brief retrieves bound action at given index
             *
             * @note this will throw exception if bindable at given index is not an action!
             */
            Action* getAction(size_t idx) const;

            /**
             * @brief returns the first bindable with given role
             */
            Bindable* getBindable(const String& role) const;

            /**
             * @brief retrieves the first bindable with given role as a state
             *
             * @note this will throw exception if bindable is not a state!
             */
            State* getState(const String& role) const;

            /**
             * @brief retrieves the first bindable with given role as an action
             *
             * @note this will throw exception if bindable is not an action!
             */
            Action* getAction(const String& role) const;

            /**
             * @brief puts all bindables of given role to target
             */
            void getBindables(const String& role, std::list<Bindable*>& target);

            typedef std::vector<std::pair<String, Bindable*> > BindableList;

            /// @brief returns bindables as a vector
            inline BindableList& getBindables()
            {
                return mBindables;
            }

            /// @brief returns bindables as a vector
            inline const BindableList& getBindables() const
            {
                return mBindables;
            }

            /**
             * @brief checks whether at least one bindable is active
             */
            bool isAnyBindableActive() const;

            /**
             * @brief checks whether all bindables are active
             */
            bool areAllBindablesActive() const;
			
		private:
			/// stores parent action
			Action* mParent;
            /// true if this binding is active (means that it activated the parent action)
            bool mIsActive;

            /// stores bounds states/actions of this binding
            BindableList mBindables;
	};
}

#endif

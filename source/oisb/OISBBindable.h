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

#ifndef __OISB_BINDABLE_H__
#define __OISB_BINDABLE_H__

#include "OISBGlobal.h"
#include "OISBPropertySet.h"

#include <vector>

namespace OISB
{
    /**
     * @brief type of the bindable
     */
    enum BindableType
    {
        /// bindable is an input state
        BT_STATE,
        /// bindable is an action
        BT_ACTION
    };

    /**
     * @brief lightweight and simple listener base class
     */
    class _OISBExport BindableListener
    {
        public:
            /// @brief destructor
            virtual ~BindableListener();

            /// called when any bindable this is attached to is activated
            virtual void bindableActivated(Bindable* bindable);

            /// called when any bindable this is attached to is deactivated
            virtual void bindableDeactivated(Bindable* bindable);

            /// called when any bindable this is attached to is updated
            virtual void bindableProcessed(Bindable* bindable);
    };

    /**
     * @brief helper little debug listener, reports every (de)activation of everything it catches
     *
     * @par
     * Call OISB::System::addListenerToAllStates(new OISB::DebugBindableListener()) to debug states and
     * OISB::System::addListenerToAllActions(new OISB::DebugBindableListener()) to debug actions
     */
    class _OISBExport DebugBindableListener : public BindableListener
    {
        public:
            /// @brief destructor
            virtual ~DebugBindableListener();

            /// @copydoc BindableListener::bindableActivated
            virtual void bindableActivated(Bindable* bindable);

            /// @copydoc BindableListener::bindableDeactivated
            virtual void bindableDeactivated(Bindable* bindable);
    };

    /**
     * @brief represents a class that can be added to a binding
     *
     * This is the parent class of State and Action
     */
	class _OISBExport Bindable : public PropertySet
	{
		public:
			/**
			 * @brief destructor
			 */
            virtual ~Bindable();

            /**
             * @brief returns the type of this bindable
             */
            virtual BindableType getBindableType() const = 0;

            /**
             * @brief returns full name of this bindable
             */
            virtual String getBindableName() const = 0;

            /**
             * @brief checks whether this bindable is active
             */
            virtual bool isActive() const = 0;

            /**
             * @brief checks whether the active state has changed since last update
             */
            virtual bool hasChanged() const = 0;

            /**
             * @brief adds a listener to this bindable
             *
             * @see BindableListener
             */
            void addListener(BindableListener* listener);

            /**
             * @brief removes a listener from this bindable
             *
             * @see BindableListener
             */
            void removeListener(BindableListener* listener);

            /// @copydoc PropertySet::listProperties
            virtual void listProperties(PropertyList& list);

        protected:
            /// @copydoc PropertySet::impl_setProperty
            virtual void impl_setProperty(const String& name, const String& value);

            /// @copydoc PropertySet::impl_getProperty
            virtual String impl_getProperty(const String& name) const;

            /// internal method, notifies attached listeners that this bindable has been activated
            void notifyActivated();
            /// internal method, notifies attached listeners that this bindable has been deactivated
            void notifyDeactivated();
            /// internal method, notifies attached listeners that this bindable has been processed
            void notifyProcessed();

        private:
            typedef std::vector<BindableListener*> ListenerList;
            /// stores listeners attached to this bindable
            ListenerList mListeners;
	};
}

#endif

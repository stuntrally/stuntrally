/*
 * This source file is part of MyGUI. For the latest info, see http://mygui.info/
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#ifndef MYGUI_CONTROLLER_ITEM_H_
#define MYGUI_CONTROLLER_ITEM_H_

#include "MyGUI_Prerequest.h"
#include "MyGUI_WidgetDefines.h"
#include "MyGUI_IObject.h"
#include "MyGUI_Delegate.h"
#include "MyGUI_EventPair.h"

namespace MyGUI
{

	class ControllerItem;

	using EventHandle_WidgetPtr = delegates::CMultiDelegate1<Widget*>;
	typedef delegates::CMultiDelegate2<Widget*, ControllerItem*> EventHandle_WidgetPtrControllerItemPtr;

	/** Base interface for controllers */
	class MYGUI_EXPORT ControllerItem :
		public IObject
	{
		MYGUI_RTTI_DERIVED( ControllerItem )

	public:
		~ControllerItem() override = default;

		virtual void prepareItem(Widget* _widget) = 0;
		virtual bool addTime(Widget* _widget, float _time) = 0;

		virtual void setProperty(const std::string& /*_key*/, const std::string& /*_value*/) { }

		/** Event : Before controller started working.\n
			signature : void method(MyGUI::Widget* _sender, MyGUI::ControllerItem* _controller)\n
			@param _sender widget under control
		*/
		EventPairAddParameter<EventHandle_WidgetPtr, EventHandle_WidgetPtrControllerItemPtr> eventPreAction;

		/** Event : Controller updated (called every frame).\n
			signature : void method(MyGUI::Widget* _sender, MyGUI::ControllerItem* _controller)\n
			@param _sender widget under control
		*/
		EventPairAddParameter<EventHandle_WidgetPtr, EventHandle_WidgetPtrControllerItemPtr> eventUpdateAction;

		/** Event : After controller finished working.\n
			signature : void method(MyGUI::Widget* _sender, MyGUI::ControllerItem* _controller)\n
			@param _sender widget under control
		*/
		EventPairAddParameter<EventHandle_WidgetPtr, EventHandle_WidgetPtrControllerItemPtr> eventPostAction;

	};

} // namespace MyGUI

#endif // MYGUI_CONTROLLER_ITEM_H_

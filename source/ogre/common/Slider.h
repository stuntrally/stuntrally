#pragma once
#include "MyGUI_Prerequest.h"
#include "MyGUI_Widget.h"

namespace MyGUI
{
	class Slider;
	typedef delegates::CMultiDelegate2<Slider*, float> EventHandle_ValueChanged;

	class Slider :
		public Widget,
		public MemberObsolete<Slider>
	{
		MYGUI_RTTI_DERIVED( Slider )

	public:
		Slider();

		//  Slider value  0.f .. 1.f
		void setValue(float val);
		float getValue() const;

		//  default value
		float mfDefault;

		/*events:*/
		/** Event : Slider position changed.\n
			signature : void method(MyGUI::Slider* sender, float value)\n
			@param sender - widget that called this event
			@param value - new slider value
		*/
		EventHandle_ValueChanged eventValueChanged;

		virtual void setSize(const IntSize& _value);
		virtual void setCoord(const IntCoord& _value);
		void setSize(int _width, int _height);
		void setCoord(int _left, int _top, int _width, int _height);

	protected:
		virtual void initialiseOverride();
		virtual void shutdownOverride();
		virtual void setPropertyOverride(const std::string& _key, const std::string& _value);

		void notifyMouse(Widget* _sender, int _left, int _top, MouseButton _id);
		void notifyMouseWheel(Widget* _sender, int _rel);
		void onMouseWheel(int _rel);

		void updateTrack();

	protected:
		//  <--[]----->
		Button* mWidgetStart;  // <
		Button* mWidgetEnd;    // >
		ImageBox* mWidgetTrack;  // [] knob

		float mfValue;
	};

}

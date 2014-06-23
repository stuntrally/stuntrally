#include "pch.h"
#include "MyGUI_Precompiled.h"
#include "Slider.h"
#include "MyGUI_InputManager.h"
#include "MyGUI_Button.h"
#include "MyGUI_ResourceSkin.h"
#include "MyGUI_ImageBox.h"

namespace MyGUI
{

	Slider::Slider() :
		mWidgetStart(nullptr),
		mWidgetEnd(nullptr),
		mWidgetTrack(nullptr),
		mfValue(0.f), mfDefault(0.5f)
	{
	}

	//  Init
	//-----------------------------------------------------------------------------------------------
	void Slider::initialiseOverride()
	{
		Base::initialiseOverride();
		
		eventMouseButtonPressed += newDelegate(this, &Slider::notifyMouse);
		eventMouseDrag += newDelegate(this, &Slider::notifyMouse);
		eventMouseWheel += newDelegate(this, &Slider::notifyMouseWheel);

		assignWidget(mWidgetTrack, "Track");
		if (mWidgetTrack != nullptr)
		{
			mWidgetTrack->eventMouseDrag += newDelegate(this, &Slider::notifyMouse);
			mWidgetTrack->eventMouseButtonPressed += newDelegate(this, &Slider::notifyMouse);
			mWidgetTrack->eventMouseWheel += newDelegate(this, &Slider::notifyMouseWheel);
			mWidgetTrack->setVisible(false);
		}

		/*
		assignWidget(mWidgetStart, "Start");
		if (mWidgetStart != nullptr)
		{
			mWidgetStart->eventMouseButtonPressed += newDelegate(this, &Slider::notifyMouse);
			mWidgetStart->eventMouseWheel += newDelegate(this, &Slider::notifyMouseWheel);
		}
		assignWidget(mWidgetEnd, "End");
		if (mWidgetEnd != nullptr)
		{
			mWidgetEnd->eventMouseButtonPressed += newDelegate(this, &Slider::notifyMouse);
			mWidgetEnd->eventMouseWheel += newDelegate(this, &Slider::notifyMouseWheel);
		}*/
	}

	void Slider::shutdownOverride()
	{
		mWidgetStart = nullptr;
		mWidgetEnd = nullptr;
		mWidgetTrack = nullptr;

		Base::shutdownOverride();
	}

	void Slider::updateTrack()
	{
		if (mWidgetTrack == nullptr)
			return;

		_forcePick(mWidgetTrack);

		if (!mWidgetTrack->getVisible())
			mWidgetTrack->setVisible(true);

		int iTrack = mWidgetTrack->getSize().width;
		int iSize = mWidgetTrack->getParent()->getSize().width - iTrack;

		int pos = mfValue * iSize;

		mWidgetTrack->setPosition(pos, mWidgetTrack->getTop());
	}

	///  mouse button
	//-----------------------------------------------------------------------------------------------
	void Slider::notifyMouse(Widget* _sender, int _left, int _top, MouseButton _id)
	{
		//eventMouseButtonPressed(this, _left, _top, _id);
		const IntPoint& p = mWidgetTrack->getParent()->getAbsolutePosition();
		const IntSize& s = mWidgetTrack->getParent()->getSize();
		int iTrack = mWidgetTrack->getSize().width;

		//  LMB set (in slider range)
		if (_id == MouseButton::Left)
		{
			float fx = (_left-iTrack/2 - p.left) / float(s.width - iTrack);
			//float fy = (_top - p.top) / float(s.height);
			fx = std::max(0.f, std::min(1.f, fx));
			
			mfValue = fx;
			eventValueChanged(this, mfValue);
			updateTrack();
		}
		else  /// RMB reset to default value
		if (_id == MouseButton::Right)
		{
			mfValue = mfDefault;
			eventValueChanged(this, mfValue);
			updateTrack();
		}
	}


	float Slider::getValue() const
	{
		return mfValue;
	}

	void Slider::setValue(float val)
	{
		//if (mValue == val)
		//	return;

		mfValue = val;  //std::max(0.f, std::min(1.f, val));
		updateTrack();
	}


	//  mouse wheel
	//--------------------------------------------------------------------------------
	void Slider::onMouseWheel(int _rel)
	{
		notifyMouseWheel(nullptr, _rel);

		Base::onMouseWheel(_rel);
	}

	void Slider::notifyMouseWheel(Widget* _sender, int _rel)
	{
		mfValue = std::max(0.f, std::min(1.f, mfValue - _rel/120.f*0.5f * 0.125f ));
		eventValueChanged(this, mfValue);
		updateTrack();
	}

	void Slider::setPropertyOverride(const std::string& _key, const std::string& _value)
	{
		if (_key == "Value")
			setValue(utility::parseValue<float>(_value));
		else
		{
			Base::setPropertyOverride(_key, _value);
			return;
		}
		eventChangeProperty(this, _key, _value);
	}


	//  update track on resize
	void Slider::setSize(int _width, int _height)
	{
		setSize(IntSize(_width, _height));
	}

	void Slider::setCoord(int _left, int _top, int _width, int _height)
	{
		setCoord(IntCoord(_left, _top, _width, _height));
	}

	void Slider::setSize(const IntSize& _size)
	{
		Base::setSize(_size);
		updateTrack();
	}

	void Slider::setCoord(const IntCoord& _coord)
	{
		Base::setCoord(_coord);
		updateTrack();
	}

}

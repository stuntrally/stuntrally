/*
 * This source file is part of MyGUI. For the latest info, see http://mygui.info/
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#ifndef MYGUI_I_ITEM_CONTAINER_H_
#define MYGUI_I_ITEM_CONTAINER_H_

#include "MyGUI_Prerequest.h"
#include "MyGUI_Constants.h"

namespace MyGUI
{

	class IItem;

	class MYGUI_EXPORT IItemContainer
	{
	public:
		virtual ~IItemContainer() { }

		virtual size_t _getItemCount() const
		{
			return 0;
		}

		virtual void _addItem(const MyGUI::UString& /*_name*/) { }

		virtual void _removeItemAt(size_t /*_index*/) { }

		virtual Widget* _getItemAt(size_t _index) const
		{
			return nullptr;
		}

		virtual void _setItemNameAt(size_t /*_index*/, const UString& /*_name*/) { }
		virtual const UString& _getItemNameAt(size_t _index) const
		{
			return Constants::getEmptyUString();
		}

		virtual void _setItemSelected(IItem* /*_item*/) { }
	};

} // namespace MyGUI

#endif // MYGUI_I_ITEM_CONTAINER_H_

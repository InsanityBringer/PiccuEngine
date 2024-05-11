/* 
* Descent 3 
* Copyright (C) 2024 Parallax Software
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CTLCFGELEM_H
#define CTLCFGELEM_H

#include "newui.h"
#include "Controller.h"
#include "controls.h"
#include "stringtable.h"


//////////////////////////////////////////////////////////////////////////////
// Data types
#define N_CFGELEM_SLOTS			CTLBINDS_PER_FUNC		// number of controller slots per function
#define CFGELEM_SLOT_CLEAR		2


//////////////////////////////////////////////////////////////////////////////
//
class cfg_element: public UIGadget 
{
	static UIBitmapItem *m_btn_bmp_lit, *m_btn_bmp, *m_xbtn_bmp_lit, *m_xbtn_bmp;
	static short m_count;
	const char *m_title;
	sbyte m_fnid, m_flags;						// fnflags : 1 if invert btn visible 
	sbyte m_slot, m_curslot;					// what slow is in focus and number of slots.
	ubyte m_slot_alpha;
	sbyte m_blink_state;

public:
	void Create(UIWindow *wnd, int str_i, int x, int y, int fnid, int id);
	sbyte GetActiveSlot() const { return m_slot; };
	bool Configure(ct_type *elem_type, ubyte *controller, ubyte *new_elem, sbyte *slot);	// calls configuration routines (returns true if approved)

protected:
	virtual void OnDraw();
	virtual void OnKeyDown(int key);
	virtual void OnMouseBtnDown(int btn);
	virtual void OnMouseBtnUp(int btn);
	virtual void OnLostFocus();
	virtual void OnGainFocus();
	virtual void OnDestroy();
	virtual void OnFormat();
};


struct tCfgDataParts
{
	ubyte bind_0, bind_1, ctrl_0, ctrl_1;
};

inline void parse_config_data(tCfgDataParts *parts, ct_type type0, ct_type type1, ct_config_data cfgdata)
{
	switch (type0)
	{
	case ctKey:	 parts->bind_0 = CONTROLLER_KEY1_VALUE(CONTROLLER_VALUE(cfgdata)); break;
	case ctAxis:
	case ctMouseAxis:
	case ctButton:
	case ctMouseButton:
	case ctPOV: 
	case ctPOV2:
	case ctPOV3:
	case ctPOV4: parts->bind_0 = CONTROLLER_CTL1_VALUE(CONTROLLER_VALUE(cfgdata)); break;
	default: parts->bind_0 = 0;
	}

	switch (type1)
	{
	case ctKey: parts->bind_1 = CONTROLLER_KEY2_VALUE(CONTROLLER_VALUE(cfgdata)); break;
	case ctAxis:
	case ctMouseAxis:
	case ctButton:
	case ctMouseButton:
	case ctPOV: 
	case ctPOV2:
	case ctPOV3:
	case ctPOV4: parts->bind_1 = CONTROLLER_CTL2_VALUE(CONTROLLER_VALUE(cfgdata)); break;
	default: parts->bind_1 = 0;
	}

	parts->ctrl_0 = CONTROLLER_CTL1_INFO(CONTROLLER_INFO(cfgdata));
	parts->ctrl_1 = CONTROLLER_CTL2_INFO(CONTROLLER_INFO(cfgdata));
}


inline ct_config_data unify_config_data(tCfgDataParts *parts)
{
	return MAKE_CONFIG_DATA(CONTROLLER_CTL_INFO(parts->ctrl_0, parts->ctrl_1), CONTROLLER_CTL_VALUE(parts->bind_0, parts->bind_1));
}

#endif

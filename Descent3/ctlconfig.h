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

#ifndef CTLCONFIG_H
#define CTLCONFIG_H

#define CTLCONFIG_KEYBOARD			0
#define CTLCONFIG_CONTROLLER		1
#define CTLCONFIG_WPNSEL			2

struct t_cfg_element
{
	short fn_id;									// -1 = group start
	short text;										// text string id.
	short x;
	short y;											// location (for groups only)
};

extern t_cfg_element Cfg_key_elements[];
extern t_cfg_element Cfg_joy_elements[];

int CtlFindBinding(int controlid,bool keyboard);
//	configures controllers.
void CtlConfig(int mode);

// opens the settings dialogs.
void joystick_settings_dialog();
void key_settings_dialog();

#endif

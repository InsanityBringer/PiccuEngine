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

#ifndef WINCONTROLLER_H
#define WINCONTROLLER_H

#include "Controller.h"
#include "joystick.h"

#define NULL_SDLCONTROLLER ((sbyte)NULL_CONTROLLER)

const int	CTF_POV = 64,				// POV control
			CTF_POV2 = 128,			// POV 2
			CTF_POV3 = 256,			// POV 3
			CTF_POV4 = 512;			// POV 4



const unsigned CTF_X_AXIS = (1<<(CT_X_AXIS-1)),		// AXIS constants for ctAxis
			CTF_Y_AXIS = (1<<(CT_Y_AXIS-1)),
			CTF_Z_AXIS = (1<<(CT_Z_AXIS-1)),
			CTF_R_AXIS = (1<<(CT_R_AXIS-1)),
			CTF_U_AXIS = (1<<(CT_U_AXIS-1)),
			CTF_V_AXIS = (1<<(CT_V_AXIS-1));

const unsigned CT_MAX_CONTROLLERS = 32,
			CT_MAX_ELEMENTS = 255,
			CT_MAX_EXTCTLS = 16,
			CT_MAX_BUTTONS = 32;

//	rules for adding controllers
//		any nonstandard special controllers should be added to the below list starting at 
//		CTID_MOUSE-1 (meaning a value of -3, -4 and so on)

const int CTID_KEYBOARD = -1,			// always -1 for keyboards
		CTID_MOUSE = -2,					// always -2 for mice
		CTID_INVALID = -3;				// invalid controller

//[ISB] temp solution for getting Descent3Heh code working. 
#define CONTROLLER_KEYBOARD 0
#define CONTROLLER_MOUSE 1
#define NUM_FIXED_CONTROLLERS 2

//	External controls
//		these are standard controllers handled through DDIO interface
//		like joysticks, etc.

const int CTID_EXTCONTROL0 = 0;

//[ISB] Can someone tell me again why the controller class is platform-dependent?
class gameSDLController: public gameController
{
public:
//	initialization of controller with needs
	gameSDLController(int num_funcs, ct_function *funcs, char *remote_adr=NULL);
	virtual ~gameSDLController();

	virtual void suspend();
	virtual void resume();

//	this functions polls the controllers if needed.  some systems may not need to implement
//	this function.
	virtual void poll(bool force = false);

//	flushes all controller information
	virtual void flush();

//	returns the value of a requested controller type.  
	virtual ct_config_data get_controller_value(ct_type type_req);

//	sets the configuration of a function
	virtual void set_controller_function(int id, const ct_type *type, ct_config_data value, const ubyte *flags);

//	returns information about a requested function
	virtual void get_controller_function(int id, ct_type *type, ct_config_data *value, ubyte *flags);

//	temporarily enables or disables a function
	virtual void enable_function(int id, bool enable); 

	virtual bool get_packet(int id, ct_packet *packet, ct_format alt_format=ctNoFormat);

// gets sensitivity of axis item
	virtual float get_axis_sensitivity(ct_type axis_type, ubyte axis);

// sets sensitivity of axis item
	virtual void set_axis_sensitivity(ct_type axis_type, ubyte axis, float val);

// assigns an individual function
	virtual int assign_function(ct_function *fn);

// activates or deactivates mouse and or controller
	virtual void mask_controllers(bool joystick, bool mouse);

// retrieves binding text for desired function, binding, etc.
	virtual const char *get_binding_text(ct_type type, ubyte ctrl, ubyte bind);

// get raw values for the controllers
	virtual int get_mouse_raw_values(int *x, int *y);
	virtual unsigned get_joy_raw_values(int *x, int *y);

//	toggles use of deadzone for controllers. ctl can be 0 to ???
// dead zone is from 0.0 to 0.5
	void set_controller_deadzone(int ctl, float deadzone);

	//[ISB] Commits the current state of all axises.
	//These values will later be comopared by get_controller_value to determine if an axis has moved.
	void commit_axis_state() override;


private:
	int m_NumControls;						// number of controllers available
	int m_Suspended;							// is controller polling suspended?
	bool m_JoyActive, m_MouseActive;		// enables or disables mouse, joystick control

	struct t_controller {
		int id;
		ushort flags;
		ushort buttons;
		unsigned btnmask;
		float normalizer[CT_NUM_AXES];
		float sens[CT_NUM_AXES];
		float sensmod[CT_NUM_AXES];
		float deadzone;
		float commit_state[CT_NUM_AXES];
	} m_ControlList[CT_MAX_CONTROLLERS];	// the control list.

	struct ct_element {
		ct_format format;
		sbyte ctl[CTLBINDS_PER_FUNC];
		ubyte value[CTLBINDS_PER_FUNC];
		ct_type	ctype[CTLBINDS_PER_FUNC];
		ubyte flags[2];
		bool enabled;
	} m_ElementList[CT_MAX_ELEMENTS];

	bool enum_controllers(char *remote_adr);

//	sets up an elements information structure
	void assign_element(int id, ct_element *elem);

//	this returns an index into the control list.
	sbyte get_axis_controller(ubyte axis);

//	returns controller with specified button
	sbyte get_button_controller(ubyte btn);

//	returns the controller with a pov hat
	sbyte get_pov_controller(ubyte pov);

//	note controller is index into ControlList.
	float get_axis_value(sbyte controller, ubyte axis, ct_format format,bool invert = false);

//	get value of button in  seconds, presses, etc.
	float get_button_value(sbyte controller, ct_format format, ubyte button);

//	get value of pov (using JOYPOV values)
	float get_pov_value(sbyte controller, ct_format format, ubyte pov_number, ubyte pov);

//	get keyboard info
	float get_key_value(int key, ct_format format);

// okay, now search for a '****.ctl' file in the current directory.
	void parse_ctl_file(int devnum, const char *ctlname);

private:
	struct t_msestate
	{
		int          m_deltaX, m_deltaY, m_deltaZ;
		int          m_absX, m_absY;
		unsigned int m_buttonMask;
	}m_MseState;

	struct t_extctlstate {
		int x,y,z,r,u,v;
		int pov[JOYPOV_NUM];
		int last_pov[JOYPOV_NUM];
		float povstarts[JOYPOV_NUM][JOYPOV_DIR];
		float povtimes[JOYPOV_NUM][JOYPOV_DIR];
		ubyte povpresses[JOYPOV_NUM][JOYPOV_DIR];
		unsigned buttons;
		ubyte btnpresses[CT_MAX_BUTTONS];
		float btnstarts[CT_MAX_BUTTONS];
		float btntimes[CT_MAX_BUTTONS];
	} m_ExtCtlStates[CT_MAX_EXTCTLS];

//	thread info.
	longlong m_frame_timer_ms;
	float m_frame_time;
	float m_frame_timer;

//	note id is id value from controller in control list.
	void extctl_getpos(int id);
	void extctl_geteval(int id);

//	this gets timings for mouse buttons
	void mouse_geteval();
};

#endif

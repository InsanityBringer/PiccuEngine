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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "pstypes.h"
#include "Macros.h"

enum ct_format 
{
	ctNoFormat,
	ctAnalog,						// analog information (-1.0 to 1.0)
	ctDigital,						// digital information (0 or 1) 
	ctDownCount,					//	special information for key presses and button/mouse presses.
	ctTime							// time in seconds format
};

enum ct_type 
{
	ctNone,
	ctAxis,							// axis element of controller. 
	ctPOV,							// function value (hiword = +axis, loword = -axis)
	ctButton,						// fn value: controller button #
	ctKey,							// fn value: lobyte(key constant), hibyte(alternate key constant)
	ctMouseAxis,
	ctMouseButton,
	ctPOV2,
	ctPOV3,
	ctPOV4							// auxillary POV values.
};


struct ct_function 
{
	int id;							// identifier for the function (like forward thrust)
	ct_format format;				// what format should the return value be for this function
	ct_type ctype[2];				// type of controller requested for this id. (1 for each value packed.)
	ubyte value[2];					// corresponding value to ctype
	ubyte flags[2];					// flags.
};


struct ct_packet 
{
	ct_format format;				// format of value.
	float value;					// time value for buttons, absolute value for axis values
	unsigned flags;					// additional information (see below)
};

typedef unsigned ct_config_data;	// passed by controller system to the outside, and back to controller system

//	values for ct_packet.flags
#define CTPK_ELEMENTACTIVE		0x1	// indicates element was activated but no time/analog information is available.
#define CTPK_MOUSE				0x2	// this is coming from a mouse device. default is joystick/keyboard.

//	element values
const ubyte CT_X_AXIS = 1,			// AXIS constants for ctAxis
			CT_Y_AXIS = 2,
			CT_Z_AXIS = 3,
			CT_R_AXIS = 4,
			CT_U_AXIS = 5,
			CT_V_AXIS = 6,
			CT_NUM_AXES = 6;		// number of axes

// ct_function flags
#define CTFNF_INVERT				0x1	// invert values returned via get_packet.


#define NULL_BINDING				0x00
#define NULL_CONTROLLER			0xff
#define INVALID_CONTROLLER_INFO	0xffff

#define CONTROLLER_KEY1_VALUE(_b) lobyte(_b)
#define CONTROLLER_KEY2_VALUE(_b) hibyte(_b)
#define CONTROLLER_KEY_VALUE(_k1, _k2) makeshort(_k2, _k1)

#define CONTROLLER_CTL1_VALUE(_b) CONTROLLER_KEY1_VALUE(_b)
#define CONTROLLER_CTL2_VALUE(_b) CONTROLLER_KEY2_VALUE(_b)
#define CONTROLLER_CTL_VALUE(_l, _h) makeshort(_h, _l)

#define CONTROLLER_CTL1_INFO(_b) ((sbyte)lobyte(_b))
#define CONTROLLER_CTL2_INFO(_b) ((sbyte)hibyte(_b))
#define CONTROLLER_CTL_INFO(_l, _h) makeshort(_h, _l)

#define CONTROLLER_VALUE(_l) ((ushort)loword(_l))
#define CONTROLLER_INFO(_l) ((short)hiword(_l))
#define MAKE_CONFIG_DATA(_c, _v) makeword(_c,_v)

#define CTLBINDS_PER_FUNC	2

class gameController
{
public:
	gameController(int num_funcs, ct_function *funcs) {};
	virtual ~gameController() {};

//	these functions suspend or resume any controller reading.  this is really only useful for
//	preemptive controller polling, but they should be used to activate and deactivate controller
//	reading.
	virtual void suspend() {};
	virtual void resume() {};

//	this functions polls the controllers if needed.  some systems may not need to implement
//	this function.
	virtual void poll() {};

//	flushes all controller information
	virtual void flush() = 0;

//	returns the value of a requested controller type. make sure you flush the controller before polling.  
	virtual ct_config_data get_controller_value(ct_type type_req) = 0;

//	sets the configuration of a function (type must be of an array == CTLBINDS_PER_FUNC)
	virtual void set_controller_function(int id, const ct_type *type, ct_config_data value, const ubyte *flags) = 0;

//	returns information about a requested function (type must be of an array == CTLBINDS_PER_FUNC)
	virtual void get_controller_function(int id, ct_type *type, ct_config_data *value, ubyte *flags) = 0;

//	temporarily enables or disables a function
	virtual void enable_function(int id, bool enable) = 0; 

//	all systems need to implement this function.  this returns information about the controller
	virtual bool get_packet(int id, ct_packet *packet, ct_format alt_format=ctNoFormat) = 0;

// gets sensitivity of axis item
	virtual float get_axis_sensitivity(ct_type axis_type, ubyte axis)=0;

// sets sensitivity of axis item
	virtual void set_axis_sensitivity(ct_type axis_type, ubyte axis, float val)=0;

// assigns an individual function
	virtual int assign_function(ct_function *fn) = 0;

// activates or deactivates mouse and or controller
	virtual void mask_controllers(bool joystick, bool mouse) = 0;

// retrieves binding text for desired function, binding, etc.
	virtual const char *get_binding_text(ct_type type, ubyte ctrl, ubyte bind) = 0;

// get raw values for the controllers
	virtual int get_mouse_raw_values(int *x, int *y) = 0;
	virtual unsigned get_joy_raw_values(int *x, int *y) = 0;

//	toggles use of deadzone for controllers. ctl can be 0 to ???
// dead zone is from 0.0 to 0.5
	virtual void  set_controller_deadzone(int ctl, float deadzone) {};
	virtual float get_controller_deadzone(int ctl) { return 0; };

// toggles use of axis on controllers. ctl can be 0 to ???
//	axis is a CT_?_AXIS value
	void toggle_controller_axis(int ctl, int axis, bool toggle) {};

};

gameController *CreateController(int num_funcs, ct_function *funcs, char *remote_ip);
void DestroyController(gameController *ctl);


#if defined(WIN32)
	#include "win\WinController.h"
#elif defined(__LINUX__)
	#include "linux/lnxcontroller.h"
#endif

#endif

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

#ifndef CONTROLS_H
#define CONTROLS_H

#include "object.h"
#include "Controller.h"

class gameController;
class pilot;

#define READF_MOUSE	0x2
#define READF_JOY		0x1
#define JOY_AXIS_SENS_RANGE	4.0f
#define MSE_AXIS_SENS_RANGE	4.0f

// used to set or get axis sensitivity
#define HEADING_AXIS		0
#define PITCH_AXIS		1
#define BANK_AXIS			2
#define HORIZONTAL_AXIS	3
#define VERTICAL_AXIS	4
#define THROTTLE_AXIS	5


//	Controller functions
const int NUM_CTLFUNCS_DEMOv1_0 = 63;
const int NUM_CONTROLLER_FUNCTIONS = 73;

const int ctfFORWARD_THRUSTAXIS = 0,
		ctfFORWARD_THRUSTKEY = 1,
		ctfREVERSE_THRUSTKEY = 2,
		ctfUP_THRUSTAXIS = 3,
		ctfUP_THRUSTKEY = 4,
		ctfDOWN_THRUSTKEY = 5,
		ctfRIGHT_THRUSTAXIS = 6,
		ctfRIGHT_THRUSTKEY = 7,
		ctfLEFT_THRUSTKEY = 8,
		ctfPITCH_DOWNAXIS = 9,
		ctfPITCH_DOWNKEY = 10,
		ctfPITCH_UPKEY = 11,
		ctfBANK_RIGHTAXIS = 12,
		ctfBANK_RIGHTKEY = 13,
		ctfBANK_LEFTKEY = 14,
		ctfHEADING_RIGHTAXIS = 15,
		ctfHEADING_RIGHTKEY = 16,
		ctfHEADING_LEFTKEY = 17,
		ctfFIREPRIMARY_BUTTON = 18,
		ctfFIREPRIMARY_KEY = 19,
		ctfFIREPRIMARY_KEY2 = 20,
		ctfFIRESECONDARY_BUTTON = 21,
		ctfFIRESECONDARY_KEY = 22,
		ctfTOGGLE_SLIDEKEY = 23,
		ctfTOGGLE_SLIDEBUTTON = 24,
		ctfFIREFLARE_KEY = 25,
		ctfFIREFLARE_BUTTON = 26,
		ctfUP_BUTTON = 27,
		ctfDOWN_BUTTON = 28,
		ctfLEFT_BUTTON = 29,
		ctfRIGHT_BUTTON = 30,
		ctfAFTERBURN_KEY = 31,
		ctfAFTERBURN_BUTTON = 32,
		ctfFORWARD_BUTTON = 33,
		ctfREVERSE_BUTTON = 34,
		ctfTOGGLE_BANKKEY = 35,
		ctfTOGGLE_BANKBUTTON = 36,
		ctfHEADING_LEFTBUTTON = 37,
		ctfHEADING_RIGHTBUTTON = 38,
		ctfPITCH_UPBUTTON = 39,
		ctfPITCH_DOWNBUTTON = 40,
		ctfBANK_LEFTBUTTON = 41,
		ctfBANK_RIGHTBUTTON = 42,
		ctfAUTOMAP_KEY = 43,
		ctfPREV_INVKEY = 44,
		ctfNEXT_INVKEY = 45,
		ctfINV_USEKEY = 46,
		ctfPREV_CNTMSKEY = 47,
		ctfNEXT_CNTMSKEY = 48,
		ctfCNTMS_USEKEY = 49,
		ctfHEADLIGHT_KEY = 50,
		ctfHEADLIGHT_BUTTON = 51,
		ctfAUTOMAP_BUTTON = 52,
		ctfPREV_INVBTN = 53,
		ctfNEXT_INVBTN = 54,
		ctfINV_USEBTN = 55,
		ctfPREV_CNTMSBTN = 56,
		ctfNEXT_CNTMSBTN = 57,
		ctfCNTMS_USEBTN = 58,
		ctfWPNSEL_PCYCLEKEY = 59,
		ctfWPNSEL_PCYCLEBTN = 60,
		ctfWPNSEL_SCYCLEKEY = 61,
		ctfWPNSEL_SCYCLEBTN = 62,
		ctfREARVIEW_KEY = 63,
		ctfREARVIEW_BTN = 64,
		ctfAUDIOTAUNT1_KEY = 65,
		ctfAUDIOTAUNT1_BTN = 66,
		ctfAUDIOTAUNT2_KEY = 67,
		ctfAUDIOTAUNT2_BTN = 68,
		ctfAUDIOTAUNT3_KEY = 69,
		ctfAUDIOTAUNT3_BTN = 70,
		ctfAUDIOTAUNT4_KEY = 71,
		ctfAUDIOTAUNT4_BTN = 72;


typedef struct game_controls {
//	movement values
//	these values are from -1.0 to 1.0.-
	float pitch_thrust;
	float heading_thrust;
	float bank_thrust;
	float vertical_thrust;
	float sideways_thrust;
	float forward_thrust;
	float	afterburn_thrust;

// these values modify thrust
	bool toggle_slide;
	bool toggle_bank;

//	this is for weapon control
	int	fire_primary_down_count;
	bool	fire_primary_down_state;
	float	fire_primary_down_time;
	int	fire_secondary_down_count;
	bool	fire_secondary_down_state;
	float	fire_secondary_down_time;

	//The flare
	int fire_flare_down_count;
	int rearview_down_count;
	bool rearview_down_state;
} game_controls;


//	This value should be set at initialization time.  Use for remote controlling.
extern char *Controller_ip;

//	Controller object.
extern gameController *Controller;

// determines if controller is being polled.
extern bool Control_poll_flag;

//
extern ct_function Controller_needs[NUM_CONTROLLER_FUNCTIONS];

//	initializes control system
void InitControls();
void CloseControls();

//	polls controller at a certain framerate.
void PollControls();

//	suspends and or resumes the control system.  this deactivates or reactivates any 
//	preemptive controller polling we setup.
void SuspendControls();
void ResumeControls();

//Read the keyboard & other controllers.  Fills in the specified structure.
void ReadPlayerControls(game_controls *controls);

//	reinits the controller, hence restoring default controller configurations
void RestoreDefaultControls();

//	load in controller configurations
void LoadControlConfig(pilot *plt=NULL);

//	save controller configurations.
void SaveControlConfig(pilot *plt=NULL);

// Reads movement only
void DoMovement(game_controls *controls);

#endif
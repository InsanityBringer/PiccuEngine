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

#ifndef JOYSTICK_H
#define JOYSTICK_H  

//	joystick ids.  used to initialize a stick and get its position
#define MAX_JOYSTICKS	8
#define JOYPOV_NUM		4

//	these flags tell what axes these controllers control.
#define JOYFLAG_XVALID			1
#define JOYFLAG_YVALID			2
#define JOYFLAG_ZVALID			4
#define JOYFLAG_RVALID			8
#define JOYFLAG_UVALID			16
#define JOYFLAG_VVALID			32
#define JOYFLAG_POVVALID		64
#define JOYFLAG_POV2VALID		128
#define JOYFLAG_POV3VALID		256
#define JOYFLAG_POV4VALID		512

//	set in joystate.pov
#define JOYPOV_DIR				8
#define JOYPOV_MAXVAL			0x100
#define JOYPOV_UP				0
#define JOYPOV_RIGHT			0x40
#define JOYPOV_DOWN				0x80
#define JOYPOV_LEFT				0xc0
#define JOYPOV_CENTER			0xff

#define JOYAXIS_RANGE			256

typedef int tJoystick;

#define JOYSTICK_1				0
#define JOYSTICK_2				1
#define JOYSTICK_3				2
#define JOYSTICK_4				3
#define JOYSTICK_5				4
#define JOYSTICK_6				5
#define JOYSTICK_7				6
#define JOYSTICK_8				7


struct tJoyInfo
{
	char name[128];
	unsigned axes_mask;
	unsigned num_btns;
	int minx, maxx;
	int miny, maxy;
	int minz, maxz;
	int minr, maxr;
	int minu, maxu;
	int minv, maxv;
};

//	shared between joystick remote server and local client.
#define JOY_PORT		3192
#define JOY_REQTERM	"RTRM"
#define JOY_TERM		"TERM"
#define JOY_POS		"POSI"
#define JOY_INFO		"INFO"
#define JOY_POLL		"POLL"

struct tJoyPacket
{
	char coda[4];								// used to identify packet
	char buf[128];
};

struct tJoyPos
{
	int x;
	int y;
	int z;
	int r;
	int u;
	int v;
	unsigned buttons;
	unsigned btn;
	unsigned pov[JOYPOV_NUM];
};

//	joystick system initialization
bool joy_Init(bool emulation);
void joy_Close();

//	retreive information about joystick.
void joy_GetJoyInfo(tJoystick joy, tJoyInfo *info);

//	retreive position of joystick
void joy_GetPos(tJoystick joy, tJoyPos *pos);

//	retreive uncalibrated position of joystick
void joy_GetRawPos(tJoystick joy, tJoyPos *pos);

//	returns true if joystick valid
bool joy_IsValid(tJoystick joy);

// run by ddio_Frame
void ddio_InternalJoyFrame();

#endif

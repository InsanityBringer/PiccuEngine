/*
* Descent 3: Piccu Engine
* Copyright (C) 2024 SaladBadger
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

// ----------------------------------------------------------------------------
//      Mouse Interface
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <math.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_rect.h>

#include "DDAccess.h"
#include "pserror.h"
#include "mono.h"
#include "ddio.h"
#include "Application.h"
#include "psclass.h"
#include "Macros.h"

struct t_mse_button_info
{
    bool is_down[N_MSEBTNS];
    ubyte down_count[N_MSEBTNS];
    ubyte up_count[N_MSEBTNS];
    float time_down[N_MSEBTNS]; // in seconds from main timer
    float time_up[N_MSEBTNS];
};

struct t_mse_event
{
    short btn;
    short state;
};

#define MOUSE_ZMIN 0 // mouse wheel z min and max (increments of 120 = 10 units)
#define MOUSE_ZMAX 1200
#define N_DIMSEBTNS 4                     // # of REAL mouse buttons
#define MSEBTN_WHL_UP (N_DIMSEBTNS)       // button index for mouse wheel up
#define MSEBTN_WHL_DOWN (N_DIMSEBTNS + 1) // button index for mouse wheel down

// taken from winuser.h
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x20a
#endif

//SDL's rect stores a point and a size.. this is easier
struct DDIORect
{
    int left, top, right, bottom;
    void Set(int newleft, int newtop, int newright, int newbottom)
    {
        left = newleft;
        top = newtop;
        right = newright;
        bottom = newbottom;
    }
};

// ----------------------------------------------------------------------------

static bool DDIO_mouse_init = false;

static struct mses_state
{
    DDIORect brect;              // limit rectangle of absolute mouse coords
    short x, y, z;           // current x,y,z in absolute mouse coords
    short cx, cy, cz;        // prior values of x,y,z from last mouse frame
    short zmin, zmax;        // 3 dimensional mouse devices use this
    int btn_mask, btn_flags; // btn_flags are the avaiable buttons on this device in mask form.
    float timer;             // done to keep track of mouse polling. [ISB] this is in InjectD3 but not here?
    bool emulated;           // are we emulating direct input?
    bool acquired;
    bool suspended;
    sbyte cursor_count;
    float x_aspect, y_aspect; // used in calculating coordinates returned from ddio_MouseGetState
    HANDLE hmseevt;           // signaled if mouse input is awaiting.
    short dx, dy, dz, imm_dz;
    short mode;         // mode of mouse operation.
    short nbtns, naxis; // device caps.
    float last_read_time; // [ISB] This is the time that the last WM_INPUT came in at
    float expire_time; // [ISB] How long a delta should be held for, to smooth out low poll devices at high framerates
    bool polled;            // [ISB] When a latched delta is read, this should be set to true. This will zero the deltas next read
} DDIO_mouse_state;

static t_mse_button_info DIM_buttons;
static tQueue<t_mse_event, 16> MB_queue;

//Used for fetching window and SDL state
static SDLApplication* Mouse_app;
void ddio_SDLMouseLinkApp(SDLApplication* app)
{
    Mouse_app = app;
}

void DDIOShowCursor(BOOL show)
{
    if (show)
    {
        //if (DDIO_mouse_state.cursor_count == -1) 
        //    ShowCursor(TRUE);
        DDIO_mouse_state.cursor_count = 0;
    }
    else
    {
        //if (DDIO_mouse_state.cursor_count == 0) 
        //    ShowCursor(FALSE);
        DDIO_mouse_state.cursor_count = -1;
    }
}

void ddio_MouseMode(int mode)
{
    mprintf((0, "mouse mode set to %d\n", mode));
    if (mode == MOUSE_EXCLUSIVE_MODE)
    {
        DDIOShowCursor(FALSE);
        SDL_SetWindowRelativeMouseMode(Mouse_app->GetWindow(), true);
    }
    else if (mode == MOUSE_STANDARD_MODE)
    {
        DDIOShowCursor(TRUE);
        SDL_SetWindowRelativeMouseMode(Mouse_app->GetWindow(), false);
        //ClipCursor(nullptr);
    }
    else 
    {
        Int3();
        return;
    }

    DDIO_mouse_state.mode = mode;
}

void ddio_MouseQueueFlush()
{
    memset(&DIM_buttons, 0, sizeof(DIM_buttons));
    MB_queue.flush();
}

void ddio_MouseReset() 
{
    //SetRect(&DDIO_mouse_state.brect, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    //[ISB] idk need better defaults
    DDIO_mouse_state.brect.Set(0, 0, 640, 480);
    DDIO_mouse_state.zmin = MOUSE_ZMIN;
    DDIO_mouse_state.zmax = MOUSE_ZMAX;

    //    set up new coordinates for mouse pointer.
    DDIO_mouse_state.btn_mask = 0;
    DDIO_mouse_state.dx = 0;
    DDIO_mouse_state.dy = 0;
    DDIO_mouse_state.dz = 0;
    DDIO_mouse_state.imm_dz = 0;
    DDIO_mouse_state.x = (DDIO_mouse_state.brect.right - DDIO_mouse_state.brect.left) / 2;
    DDIO_mouse_state.y = (DDIO_mouse_state.brect.bottom - DDIO_mouse_state.brect.top) / 2;
    DDIO_mouse_state.z = (DDIO_mouse_state.zmax = DDIO_mouse_state.zmin) / 2;
    DDIO_mouse_state.cz = 0;
    DDIO_mouse_state.x_aspect = 1.0f;
    DDIO_mouse_state.y_aspect = 1.0f;

    // reset button states
    ddio_MouseQueueFlush();
}

// return mouse button down time.
// This function has been hacked to use timer_GetTime which can be much more accurate.
float ddio_MouseBtnDownTime(int btn)
{
    float time, curtime = timer_GetTime();

    // ASSERT(btn >= 0 && btn < N_MSEBTNS);

    if (DIM_buttons.is_down[btn])
    {
        time = curtime - DIM_buttons.time_down[btn];
        DIM_buttons.time_down[btn] = curtime;
    }
    else
    {
        time = DIM_buttons.time_up[btn] - DIM_buttons.time_down[btn];
        DIM_buttons.time_down[btn] = DIM_buttons.time_up[btn] = 0;
    }

    DIM_buttons.is_down[MSEBTN_WHL_UP] = false;
    DIM_buttons.is_down[MSEBTN_WHL_DOWN] = false;

    return time;
}

int ddio_MouseGetState(int* x, int* y, int* dx, int* dy, int* z, int* dz)
{
    //    update mouse timer.
    int btn_mask = DDIO_mouse_state.btn_mask;

    DDIO_mouse_state.timer = timer_GetTime();

    //    get return values.
    if (x)
        *x = DDIO_mouse_state.x;
    if (y)
        *y = DDIO_mouse_state.y;
    if (z)
        *z = DDIO_mouse_state.z;
    if (dx)
        *dx = DDIO_mouse_state.dx;
    if (dy)
        *dy = DDIO_mouse_state.dy;
    if (dz)
        *dz = DDIO_mouse_state.dz;

    if (timer_GetTime() > DDIO_mouse_state.expire_time)
    {
        DDIO_mouse_state.dx = 0;
        DDIO_mouse_state.dy = 0;
        DDIO_mouse_state.dz = 0;
    }
    DDIO_mouse_state.btn_mask = 0;
    DDIO_mouse_state.polled = true;

    DIM_buttons.is_down[MSEBTN_WHL_UP] = false;
    DIM_buttons.is_down[MSEBTN_WHL_DOWN] = false;

    return btn_mask;
}

DDIO_ShouldCaptureMouse_fp DDIO_ShouldCaptureMouse;
void ddio_MouseSetCallbackFn(DDIO_ShouldCaptureMouse_fp fn)
{
    DDIO_ShouldCaptureMouse = fn;
}

void ddio_InternalMouseFrame() 
{
    int btn_mask = 0;

    // These need to be continually maintained, since a small number of inputs rely on it being set every frame.
    if (DIM_buttons.is_down[0])
        btn_mask |= MOUSE_LB;
    if (DIM_buttons.is_down[1])
        btn_mask |= MOUSE_RB;
    if (DIM_buttons.is_down[2])
        btn_mask |= MOUSE_CB;
    if (DIM_buttons.is_down[3])
        btn_mask |= MOUSE_B4;
    if (DIM_buttons.is_down[4])
        btn_mask |= MOUSE_B5;
    if (DIM_buttons.is_down[5])
        btn_mask |= MOUSE_B6;
    if (DIM_buttons.is_down[6])
        btn_mask |= MOUSE_B7;
    if (DIM_buttons.is_down[7])
        btn_mask |= MOUSE_B8;

    DDIO_mouse_state.btn_mask = btn_mask;

    if (DDIO_ShouldCaptureMouse)
    {
        bool capture = DDIO_ShouldCaptureMouse();
        if (DDIO_mouse_state.mode != MOUSE_EXCLUSIVE_MODE && capture)
            ddio_MouseMode(MOUSE_EXCLUSIVE_MODE);
        else if (DDIO_mouse_state.mode != MOUSE_STANDARD_MODE && !capture)
            ddio_MouseMode(MOUSE_STANDARD_MODE);
    }
}

// used to prevent mouse input from being registered
void ddio_InternalMouseSuspend() 
{
    if (!DDIO_mouse_init)
        return;

    DDIO_mouse_state.suspended = true;
}

void ddio_InternalMouseResume() 
{
    if (!DDIO_mouse_init)
        return;

    DDIO_mouse_state.suspended = false;
}

// return mouse button down time
int ddio_MouseBtnDownCount(int btn) 
{
    if (btn < 0 || btn >= N_MSEBTNS)
        return 0;
    int n_downs = DIM_buttons.down_count[btn];

    if (n_downs) {

        DIM_buttons.down_count[btn] = 0;
    }

    return n_downs;
}

// return mouse button up count
int ddio_MouseBtnUpCount(int btn) 
{
    if (btn < 0 || btn >= N_MSEBTNS)
        return 0;
    int n_ups = DIM_buttons.up_count[btn];
    DIM_buttons.up_count[btn] = 0;
    return n_ups;
}

// get device caps
int ddio_MouseGetCaps(int* btn, int* axis) 
{
    *btn = (int)DDIO_mouse_state.nbtns;
    *axis = (int)DDIO_mouse_state.naxis;

    return DDIO_mouse_state.btn_flags;
}

//      gets limits on the position of the mouse cursor (or atleast what's returned from GetState)
void ddio_MouseGetLimits(int* left, int* top, int* right, int* bottom, int* zmin, int* zmax) 
{
    *left = DDIO_mouse_state.brect.left;
    *top = DDIO_mouse_state.brect.top;
    *right = DDIO_mouse_state.brect.right;
    *bottom = DDIO_mouse_state.brect.bottom;

    if (zmin)
        *zmin = DDIO_mouse_state.zmin;
    if (zmax)
        *zmax = DDIO_mouse_state.zmax;
}

//      sets limits on the position of the mouse cursor (or atleast what's returned from GetState)
void ddio_MouseSetLimits(int left, int top, int right, int bottom, int zmin, int zmax) 
{
    bool zaxis = (DDIO_mouse_state.naxis >= 3);
    DDIO_mouse_state.brect.Set(left, top, right, bottom);
    DDIO_mouse_state.zmin = (!zmin && zaxis) ? MOUSE_ZMIN : zmin;
    DDIO_mouse_state.zmax = (!zmax && zaxis) ? MOUSE_ZMAX : zmax;
    DDIO_mouse_state.cx = left + (right - left) / 2;
    DDIO_mouse_state.cy = top + (bottom - top) / 2;
}

// virtual coordinate system for mouse (match to video resolution set for optimal mouse usage.
void ddio_MouseSetVCoords(int width, int height) 
{ 
    ddio_MouseSetLimits(0, 0, width, height); 
}

// gets a mouse button event, returns false if none.
bool ddio_MouseGetEvent(int* btn, bool* state) 
{
    t_mse_event evt;

    if (MB_queue.recv(&evt)) 
    {
        *btn = (int)evt.btn;
        *state = evt.state ? true : false;
        return true;
    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////
// [ISB] These must be 32-char strings
char Ctltext_MseBtnBindings[N_MSEBTNS][32] = { "mse-l\0\0\0\0\0\0\0\0\0\0\0\0",
                                              "mse-r\0\0\0\0\0\0\0\0\0\0\0\0",
                                              "mse-c\0\0\0\0\0\0\0\0\0\0\0\0",
                                              "mse-b4\0\0\0\0\0\0\0\0\0\0\0",
                                              "msew-u\0\0\0\0\0\0\0\0\0\0\0",
                                              "msew-d\0\0\0\0\0\0\0\0\0\0\0",
                                              "mse-b5",
                                              "" };

char Ctltext_MseAxisBindings[][32] = { "mse-X\0\0\0\0\0\0\0\0\0\0\0\0",
                                        "mse-Y\0\0\0\0\0\0\0\0\0\0\0\0",
                                        "msewheel\0\0\0\0\0\0\0\0\0\0" };

// returns string to binding.
const char* ddio_MouseGetBtnText(int btn)
{
    if (btn >= N_MSEBTNS || btn < 0)
        return ("");
    return Ctltext_MseBtnBindings[btn];
}

const char* ddio_MouseGetAxisText(int axis)
{
    if (axis >= (sizeof(Ctltext_MseAxisBindings) / sizeof(char[32])) || axis < 0)
        return ("");
    return Ctltext_MseAxisBindings[axis];
}

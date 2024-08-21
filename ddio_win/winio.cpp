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

// ----------------------------------------------------------------------------
// Win32 IO System Main Library Interface
// ----------------------------------------------------------------------------
#include "DDAccess.h"

#include <stdlib.h>
#include <stdarg.h>

#include "pserror.h"
#include "pstring.h"
#include "Application.h"
#include "ddio_win.h"
#include "ddio.h"
#include "dinput.h"

#include "forcefeedback.h"

bool 				DDIO_init = 0;
dinput_data			DInputData;



// ----------------------------------------------------------------------------
//	Initialization and destruction functions
// ----------------------------------------------------------------------------

bool ddio_InternalInit(ddio_init_info *init_info)
{
	oeWin32Application *obj = (oeWin32Application *)init_info->obj;
	LPDIRECTINPUT lpdi;
	HRESULT	dires;

	ASSERT(!DDIO_init);

//	Initialize DirectInput subsystem
	mprintf((0, "DI system initializing.\n"));

	//Try to open DirectX 5.00
	dires = DirectInputCreate((HINSTANCE)obj->m_hInstance, DIRECTINPUT_VERSION, &lpdi, NULL);

	//Deal with error opening DX5
	if (dires != DI_OK) {

		//If running NT, try DirectX 3.0
		if(obj->NT()){

			dires = DirectInputCreate((HINSTANCE)obj->m_hInstance, 0x0300, &lpdi, NULL);
			if (dires != DI_OK) {
				Error("Unable to DirectInput system (Requires at least DirectX 3.0 For NT) [DirectInput:%x]\n", dires);
			}
		}
		else {	//Not running NT, so error out

			//Couldn't open DirectX, so print error
			Error("Unable to DirectInput system (Requires at least DirectX 5.0) [DirectInput:%x]\n", dires);
		}
	}

	DInputData.app = obj;
	DInputData.lpdi = lpdi;
	DInputData.hwnd = (HWND)obj->m_hWnd;

	DDIO_init = 1;

	return 1;
}


void ddio_InternalClose()
{
	ASSERT(DDIO_init);

//	//Close down forcefeedback
//	ddio_ff_DetachForce();

	DInputData.lpdi->Release();
	DInputData.lpdi = NULL;
	DDIO_init = 0;

	mprintf((0, "DI system closed.\n"));
}


#ifdef _DEBUG
void ddio_DebugMessage(unsigned err, char *fmt, ...)
{
	char buf[128];
	va_list arglist;

	va_start(arglist,fmt);
	Pvsprintf(buf,128,fmt,arglist);
	va_end(arglist);

	mprintf((0, "DDIO: %s\n", buf));
	if (err) {
		mprintf((1, "DIERR %x.\n", err));
	}
}
#endif



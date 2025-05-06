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
// SDL IO initialization
// but it's already initialized heh
// ----------------------------------------------------------------------------
#include "DDAccess.h"

#include <stdlib.h>
#include <stdarg.h>

#include "pserror.h"
#include "pstring.h"
#include "Application.h"
#include "ddio.h"
#include "dinput.h"

#include "forcefeedback.h"

bool 				DDIO_init = false;



// ----------------------------------------------------------------------------
//	Initialization and destruction functions
// ----------------------------------------------------------------------------

bool ddio_InternalInit(ddio_init_info* init_info)
{
	//hey, how'd I get here?
	mprintf((0, "DDIO started up. Isn't that great?\n"));
	DDIO_init = true;

	return 1;
}


void ddio_InternalClose()
{
	DDIO_init = false;
	mprintf((0, "DDIO system closed.\n"));
}


#ifdef _DEBUG
void ddio_DebugMessage(unsigned err, char* fmt, ...)
{
	char buf[128];
	va_list arglist;

	va_start(arglist, fmt);
	Pvsprintf(buf, 128, fmt, arglist);
	va_end(arglist);

	mprintf((0, "DDIO: %s\n", buf));
	if (err) {
		mprintf((1, "DIERR %x.\n", err));
	}
}
#endif

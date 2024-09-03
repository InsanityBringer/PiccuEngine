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

#include "DDAccess.h"
#include "application.h"
#include "AppConsole.h"
#include "mono.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

//put some data up on the screen
void con_null_Puts(int window,const char *str);

void con_null_Printf(const char *fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args,fmt);
	vsprintf(buffer,fmt,args);
	va_end(args);
	con_null_Puts(0,buffer);
}

bool con_null_Input(char *buf, int buflen)
{
	return false;
}

void con_null_Defer(void)
{
}

bool con_null_Create(void)
{
	printf("Descent 3 Dedicated Server\n");
	printf("Running in quiet mode.\n");
	printf("To Administer, you must telnet in to the dedicated server.\n");
	return true;
}

void con_null_Destroy(void)
{
}

//put some data up on the screen
void con_null_Puts(int window,const char *str)
{
	mprintf((0,(char *)str));
}

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

//Windows console routines for driving an actual console window.
//Currently used for the dedicated server when compiled with SDL
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <varargs.h>
#include <string>
#include "pserror.h"
#include "enginebrand.h"

static HANDLE inhandle;

static std::string coninputbuf;
static std::string coninputcommand;
char conlinebuf[2048];
void con_Printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(conlinebuf, sizeof(conlinebuf), fmt, args);

	//To avoid overwriting the input line, I store the cursor position at the end of each print,
	//and restore it at the beginning of the next.
	printf("\u001b8"); //restore cursor position

	//Clear the input line in full
	printf("\u001b[%dX", (int)coninputbuf.size() + 1);

	printf("%s", conlinebuf); //print message
	printf("\u001b7"); //save new cursor position
	printf("%%%s", coninputbuf.c_str()); //print current input line
}

bool con_Input(char* buf, int buflen)
{
	if (coninputcommand.empty())
		return false;

	strncpy(buf, coninputcommand.c_str(), buflen);
	buf[buflen - 1] = 0;

	coninputcommand.clear();

	return true;
}

void con_init()
{
	inhandle = GetStdHandle(STD_INPUT_HANDLE);
	if (!inhandle)
		Error("con_init: Failed to get standard input handle!");

	if (!SetConsoleMode(inhandle, ENABLE_INSERT_MODE))
		Error("con_init: Changing console input mode failed!");

	SetConsoleTitle(TEXT(ENGINE_NAME" dedicated server"));

	//Save the cursor position, which will be used to position messages printed to console. 
	printf("\u001b7");

	printf("%"); //Add a prompt marker
}

void con_shutdown()
{
}

void con_defer()
{
	if (!inhandle)
		return;

	DWORD numreadevents;
	INPUT_RECORD inputbuf[256];
	//Must read the amount of console events before calling ReadConsoleInput since it is impossible to make ReadConsoleInput never block. 
	//This should ensure it will never block by always having something to read. 
	if (GetNumberOfConsoleInputEvents(inhandle, &numreadevents) && numreadevents > 0)
	{
		if (ReadConsoleInput(inhandle, inputbuf, 256, &numreadevents))
		{
			for (int i = 0; i < numreadevents; i++)
			{
				if (inputbuf[i].EventType == KEY_EVENT && inputbuf[i].Event.KeyEvent.bKeyDown)
				{
					char c = inputbuf[i].Event.KeyEvent.uChar.AsciiChar;
					if (c == '\u0008')
					{
						if (!coninputbuf.empty())
						{
							coninputbuf.resize(coninputbuf.size() - 1);
							printf("\u0008 \u0008"); //heh
						}
					}
					else if (c == '\n') //do we ever get a \n? It doesn't seem like it. 
					{
						DebugBreak();
					}
					else if (c == '\r')
					{
						coninputcommand = coninputbuf;
						coninputbuf.clear();
						con_Printf("\n"); //needs to update memorized cursor position
					}
					else if (c != '\0')
					{
						printf("%c", c);
						coninputbuf.push_back(c);
					}
				}
			}
		}
	}
}

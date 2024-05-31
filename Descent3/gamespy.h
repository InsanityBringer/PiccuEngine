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

#ifndef GAMESPY_H
#define GAMESPY_H

void gspy_StartGame(char *name);
void gspy_EndGame();
int gspy_Init(void);
void gspy_DoFrame();

int gspy_ParseReq(char *buffer,SOCKADDR_IN *addr);

int gspy_DoEcho(SOCKADDR_IN *addr,char *msg);
int gspy_DoBasic(SOCKADDR_IN *addr);
int gspy_DoStatus(SOCKADDR_IN *addr);
int gspy_DoRules(SOCKADDR_IN *addr);
int gspy_DoPlayers(SOCKADDR_IN *addr);
int gspy_DoGameInfo(SOCKADDR_IN *addr);
int gspy_DoHeartbeat(SOCKADDR_IN *addr);

#endif

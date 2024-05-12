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

#pragma once

#define MAX_MISSION_URL_LEN		300
#define MAX_MISSION_URL_COUNT	5

#define URL_ASK_POLL_TIME		2

extern int Got_url;

struct msn_urls
{
	char msnname[_MAX_PATH];
	char URL[MAX_MISSION_URL_COUNT][MAX_MISSION_URL_LEN];
	//Possibly some quality of service flags
};


//Function prototypes
void msn_DoAskForURL(ubyte *indata,network_address *net_addr);
void msn_DoCurrMsnURLs(ubyte *data,network_address *net_addr);
int msn_CheckGetMission(network_address *net_addr,char *filename);
int msn_ShowDownloadChoices(msn_urls *urls);
int msn_DownloadWithStatus(char *url,char *filename);
void msn_ClipURLToWidth(int width,char *string);
char * msn_SecondsToString(int time_sec);

int ModDownloadWithStatus(char *url,char *filename);
int CheckGetD3M(char *d3m);

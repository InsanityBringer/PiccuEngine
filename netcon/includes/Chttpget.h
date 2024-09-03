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

#ifndef _CHTTPGET_HEADER_
#define _CHTTPGET_HEADER_

#define HTTP_STATE_INTERNAL_ERROR		0
#define HTTP_STATE_SOCKET_ERROR			1
#define HTTP_STATE_URL_PARSING_ERROR	2
#define HTTP_STATE_CONNECTING			3
#define HTTP_STATE_HOST_NOT_FOUND		4
#define HTTP_STATE_CANT_CONNECT			5
#define HTTP_STATE_CONNECTED			6
#define HTTP_STATE_FILE_NOT_FOUND		10
#define HTTP_STATE_RECEIVING			11
#define HTTP_STATE_FILE_RECEIVED		12
#define HTTP_STATE_UNKNOWN_ERROR		13
#define HTTP_STATE_RECV_FAILED			14
#define HTTP_STATE_CANT_WRITE_FILE		15
#define HTTP_STATE_STARTUP				16

#define MAX_URL_LEN	300

class ChttpGet
{
public:
	ChttpGet(const char *URL,const char *localfile);
	ChttpGet(const char *URL,const char *localfile,char *proxyip,unsigned short proxyport);
	void PrepSocket(const char *URL);
	~ChttpGet();
	void GetFile(const char *URL,const char *localfile);
	int GetStatus();
	unsigned int GetBytesIn();
	unsigned int GetTotalBytes();
	void AbortGet();
	void WorkerThread();
	bool m_Aborted;

protected:
	int ConnectSocket();
	char *GetHTTPLine();
	unsigned int ReadDataChannel();
	unsigned int m_iBytesIn;
	unsigned int m_iBytesTotal;
	unsigned int m_State;
	bool m_ProxyEnabled;
	char *m_ProxyIP;
	char m_URL[MAX_URL_LEN];
	unsigned short m_ProxyPort;

	char m_szUserName[100];
	char m_szPassword[100];
	char m_szHost[_MAX_PATH];
	char m_szDir[_MAX_PATH];
	char m_szFilename[_MAX_PATH];
	
	bool m_Aborting;


	SOCKET m_DataSock;
	
	FILE *LOCALFILE;
	char recv_buffer[1000];

};

#endif

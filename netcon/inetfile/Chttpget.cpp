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

#ifdef WIN32
#include <windows.h>
#include <process.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <wininet.h>
#endif

#include "inetgetfile.h"
#include "Chttpget.h"
#include "mono.h"

#ifndef WIN32
#include "mem.h"
#else
#define mem_malloc(a) malloc(a)
#define mem_free(a) free(a)
#endif

#ifdef __LINUX__

#include "SDL_thread.h"

inline void Sleep(int millis)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = millis*1000;
	select(0,NULL,NULL,NULL,&tv);
}
#endif

#define NW_AGHBN_CANCEL		1
#define NW_AGHBN_LOOKUP		2
#define NW_AGHBN_READ		3

#ifndef __LINUX__
void __cdecl http_gethostbynameworker(void *parm);
#else
int http_gethostbynameworker(void *parm);
#endif

int http_Asyncgethostbyname(unsigned int *ip,int command, char *hostname);

#ifndef __LINUX__
void HTTPObjThread( void * obj )
#else
int HTTPObjThread( void * obj )
#endif
{
	((ChttpGet *)obj)->WorkerThread();
	((ChttpGet *)obj)->m_Aborted = true;
	//OutputDebugString("http transfer exiting....\n");

	#ifdef __LINUX__
	return 0;
	#endif
}

void ChttpGet::AbortGet()
{
#ifdef WIN32
	OutputDebugString("Aborting....\n");
#endif
	m_Aborting = true;
	while(!m_Aborted) Sleep(50); //Wait for the thread to end
#ifdef WIN32
	OutputDebugString("Aborted....\n");
#endif
}

ChttpGet::ChttpGet(const char *URL, const char *localfile, char *proxyip, unsigned short proxyport)
{
	m_ProxyEnabled = true;
	m_ProxyIP = proxyip;
	m_ProxyPort = proxyport;
	GetFile(URL,localfile);
}

ChttpGet::ChttpGet(const char *URL, const char *localfile)
{
	m_ProxyEnabled = false;
	GetFile(URL,localfile);
}

void ChttpGet::PrepSocket(const char *URL)
{

	m_DataSock = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_DataSock)
	{
		m_State = HTTP_STATE_SOCKET_ERROR;
		return;
	}
	unsigned long arg;

	arg = true;
#if defined(WIN32)
	ioctlsocket( m_DataSock, FIONBIO, &arg );
#elif defined(__LINUX__)
	ioctl( m_DataSock, FIONBIO, &arg );
#endif

	const char *pURL = URL;
	if(strnicmp(URL,"http:",5) == 0)
	{
		pURL +=5;
		while(*pURL == '/')
		{
			pURL++;
		}
	}
	else if (strnicmp(URL, "https:", 6) == 0)
	{
		pURL += 6;
		while (*pURL == '/')
		{
			pURL++;
		}
	}

	//There shouldn't be any : in this string
	if(strchr(pURL,':'))
	{
		m_State = HTTP_STATE_URL_PARSING_ERROR;
		return;
	}
	//read the filename by searching backwards for a /
	//then keep reading until you find the first /
	//when you found it, you have the host and dir
	const char *filestart = NULL;
	const char *dirstart;
	for(int i = strlen(pURL);i>=0;i--)
	{
		if(pURL[i]== '/')
		{
			if(!filestart)
			{
				filestart = pURL+i+1;
				dirstart = pURL+i+1;
				strcpy(m_szFilename,filestart);
			}
			else
			{
				dirstart = pURL+i+1;
			}
		}
	}
	if((dirstart==NULL) || (filestart==NULL))
	{
		m_State = HTTP_STATE_URL_PARSING_ERROR;
		return;
	}
	else
	{
		strcpy(m_szDir,dirstart);//,(filestart-dirstart));
		//m_szDir[(filestart-dirstart)] = NULL;
		strncpy(m_szHost,pURL,(dirstart-pURL));
		m_szHost[(dirstart-pURL)-1] = '\0';
	}

}


void ChttpGet::GetFile(const char *URL,const char *localfile)
{
	m_DataSock = INVALID_SOCKET;
	m_iBytesIn = 0;
	m_iBytesTotal = 0;
	m_State = HTTP_STATE_STARTUP;;
	m_Aborting = false;
	m_Aborted = false;

	strncpy(m_URL,URL,MAX_URL_LEN-1);
	m_URL[MAX_URL_LEN-1] = 0;

	LOCALFILE = fopen(localfile,"wb");
	if(NULL == LOCALFILE)
	{
		m_State = HTTP_STATE_CANT_WRITE_FILE;
		return;
	}
	
	PrepSocket(URL);

#ifdef WIN32
	if(NULL==_beginthread(HTTPObjThread,0,this))
	{
		m_State = HTTP_STATE_INTERNAL_ERROR;
		return;
	}
#elif defined(__LINUX__)
//	pthread_t thread;
    SDL_Thread *thread;

	if(!inet_LoadThreadLib())
	{
		m_State = HTTP_STATE_INTERNAL_ERROR;
		return;
	}

//	if(df_pthread_create(&thread,NULL,HTTPObjThread,this)!=0)
    thread = SDL_CreateThread(HTTPObjThread, this);
    if (thread == NULL)
	{
		m_State = HTTP_STATE_INTERNAL_ERROR;
		return;
	}
#endif
}


ChttpGet::~ChttpGet()
{
	if(m_DataSock != INVALID_SOCKET)
	{
		shutdown(m_DataSock,2);
#ifndef __LINUX__
		closesocket(m_DataSock);
#else
		close(m_DataSock);
#endif
	}
}

int ChttpGet::GetStatus()
{
	return m_State;
}

unsigned int ChttpGet::GetBytesIn()
{
	return m_iBytesIn;
}

unsigned int ChttpGet::GetTotalBytes()
{
	return m_iBytesTotal;
}

void ChttpGet::WorkerThread()
{
#ifdef WIN32
	HINTERNET hInternetSession;
	HINTERNET hURL;
	BOOL bResult;
	DWORD dwBytesRead = 1;
	char buf[1024];

	hInternetSession = InternetOpen("Descent3", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hInternetSession)
	{
		m_State = HTTP_STATE_UNKNOWN_ERROR;
		fclose(LOCALFILE);
		return;
	}

	hURL = InternetOpenUrl(hInternetSession, m_URL, NULL, 0, 0, 0);
	if (!hURL)
	{
		DWORD err = GetLastError();
		m_State = err == ERROR_INTERNET_NAME_NOT_RESOLVED ? HTTP_STATE_HOST_NOT_FOUND :
			err == ERROR_INTERNET_CANNOT_CONNECT ? HTTP_STATE_CANT_CONNECT :
			err == ERROR_FILE_NOT_FOUND ? HTTP_STATE_FILE_NOT_FOUND : HTTP_STATE_UNKNOWN_ERROR;
		InternetCloseHandle(hInternetSession);
		fclose(LOCALFILE);
		return;
	}


	DWORD dwStatusCode;
	DWORD dwStatusCodeSize = sizeof(dwStatusCode);
	if (!HttpQueryInfo(hURL, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwStatusCodeSize, NULL))
	{
		DWORD err = GetLastError();
		m_State = HTTP_STATE_UNKNOWN_ERROR;
		InternetCloseHandle(hURL);
		InternetCloseHandle(hInternetSession);
		fclose(LOCALFILE);
		return;
	}

	if (dwStatusCode != 200)
	{
		m_State = dwStatusCode == 404 ? HTTP_STATE_FILE_NOT_FOUND : HTTP_STATE_UNKNOWN_ERROR;
		InternetCloseHandle(hURL);
		InternetCloseHandle(hInternetSession);
		fclose(LOCALFILE);
		return;
	}

	DWORD dwContentLength;
	DWORD dwContentLengthSize = sizeof(dwContentLength);
	if (HttpQueryInfo(hURL, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
		&dwContentLength, &dwContentLengthSize, NULL))
	{
		m_iBytesTotal = dwContentLength;
	}

	for (; dwBytesRead > 0;)
	{
		if (!InternetReadFile(hURL, buf, (DWORD)sizeof(buf), &dwBytesRead))
		{
			DWORD err = GetLastError();
			mprintf((0,"InternetReadFile error %d\n", err));
			m_State = HTTP_STATE_RECV_FAILED;
			InternetCloseHandle(hURL);
			InternetCloseHandle(hInternetSession);
			fclose(LOCALFILE);
			return;
		}
		if (fwrite(buf, 1, dwBytesRead, LOCALFILE) != dwBytesRead)
		{
			m_State = HTTP_STATE_CANT_WRITE_FILE;
			InternetCloseHandle(hURL);
			InternetCloseHandle(hInternetSession);
			fclose(LOCALFILE);
			return;
		}
		m_iBytesIn += dwBytesRead;
	}

	InternetCloseHandle(hURL);
	InternetCloseHandle(hInternetSession);

	m_State = HTTP_STATE_FILE_RECEIVED;
	fclose(LOCALFILE);
#else
	char szCommand[1000];
	char *p;
	int irsp = 0;
	ConnectSocket();
	if(m_Aborting)
	{
		fclose(LOCALFILE);
		return;
	}
	if(m_State != HTTP_STATE_CONNECTED)
	{
		fclose(LOCALFILE);
		return;
	}
	sprintf(szCommand,"GET %s%s HTTP/1.1\nAccept: */*\nAccept-Encoding: deflate\nHost: %s\n\n\n",m_ProxyEnabled?"":"/",m_ProxyEnabled?m_URL:m_szDir,m_szHost);
	send(m_DataSock,szCommand,strlen(szCommand),0);
	p = GetHTTPLine();
	if(p && strnicmp("HTTP/",p,5)==0)
	{
		char *pcode;
		pcode = strchr(p,' ')+1;
		if(!pcode)
		{
			m_State = HTTP_STATE_UNKNOWN_ERROR;	
			fclose(LOCALFILE);
			return;

		}
		pcode[3] = '\0';
		irsp = atoi(pcode);

		if(irsp == 0)
		{
			m_State = HTTP_STATE_UNKNOWN_ERROR;	
			fclose(LOCALFILE);
			return;
		}
		if(irsp == 301)
		{

			//This is a redirection! woohoo, what fun we are going to have.
			//Next thing we need to do is find where it's redirected to.
			//We do that by looking for a "Location: xxxx" line.
			
			int idataready=0;
			do
			{
				p = GetHTTPLine();
				if(p==NULL)
				{
					m_State = HTTP_STATE_UNKNOWN_ERROR;	
					fclose(LOCALFILE);
					return;
				}
				if(*p=='\0')
				{
					idataready = 1;
					break;
				}
				if(strnicmp(p,"Location:",strlen("Location:"))==0)
				{
					char *s = strchr(p,' ')+1;
					
					
					//Then, once we've found that, we close the sockets & reissue the whole freakin request.
					shutdown(m_DataSock,2);

					#ifdef WIN32
					closesocket(m_DataSock);
					#else
					close(m_DataSock);
					#endif

					m_DataSock = INVALID_SOCKET;

					//New location to look at is in 's'
					PrepSocket(s);
					WorkerThread();
					return;
				}
			}while(!idataready);


		}
		if(irsp==200)
		{
			int idataready=0;
			do
			{
				p = GetHTTPLine();
				if(p==NULL)
				{
					m_State = HTTP_STATE_UNKNOWN_ERROR;	
					fclose(LOCALFILE);
					return;
				}
				if(*p=='\0')
				{
					idataready = 1;
					break;
				}
				if(strnicmp(p,"Content-Length:",strlen("Content-Length:"))==0)
				{
					char *s = strchr(p,' ')+1;
					p = s;
					if(s)
					{
						while(*s)
						{
							if(!isdigit(*s))
							{
								*s='\0';
							}
							s++;
						};
						m_iBytesTotal = atoi(p);
					}

				}
			}while(!idataready);
		ReadDataChannel();
		return;
		}
		m_State = HTTP_STATE_FILE_NOT_FOUND;
		fclose(LOCALFILE);
		return;
	}
	else
	{
		m_State = HTTP_STATE_UNKNOWN_ERROR;
		fclose(LOCALFILE);
		return;
	}
#endif
}

int ChttpGet::ConnectSocket()
{
	//HOSTENT *he;
	unsigned int ip;
	SERVENT *se;
	SOCKADDR_IN hostaddr;

	int rcode = 0;

	if(m_Aborting)
		return 0;
	
	ip = inet_addr((const char *)m_szHost);

	if(ip==INADDR_NONE)
	{
		http_Asyncgethostbyname(&ip,NW_AGHBN_LOOKUP,m_szHost);
		rcode = 0;
		do
		{	
			if(m_Aborting)
			{
				http_Asyncgethostbyname(&ip,NW_AGHBN_CANCEL,m_szHost);
				return 0;
			}
			rcode = http_Asyncgethostbyname(&ip,NW_AGHBN_READ,m_szHost);
		}while(rcode==0);
	}
	
	if(rcode == -1)
	{
		m_State = HTTP_STATE_HOST_NOT_FOUND;
		return 0;
	}
	//m_ControlSock
	if(m_Aborting)
		return 0;
	se = getservbyname("http", NULL);
	if(m_Aborting)
		return 0;
	if(se == NULL)
	{
		hostaddr.sin_port = htons(80);
	}
	else
	{
		hostaddr.sin_port = se->s_port;
	}
	hostaddr.sin_family = AF_INET;		
	//ip = htonl(ip);
	memcpy(&hostaddr.sin_addr,&ip,4);

	if(m_ProxyEnabled)
	{
		//This is on a proxy, so we need to make sure to connect to the proxy machine
		ip = inet_addr((const char *)m_ProxyIP);
				
		if(ip==INADDR_NONE)
		{
			http_Asyncgethostbyname(&ip,NW_AGHBN_LOOKUP,m_ProxyIP);
			rcode = 0;
			do
			{	
				if(m_Aborting)
				{
					http_Asyncgethostbyname(&ip,NW_AGHBN_CANCEL,m_ProxyIP);
					return 0;
				}
				rcode = http_Asyncgethostbyname(&ip,NW_AGHBN_READ,m_ProxyIP);
			}while(rcode==0);
			
			
			if(rcode == -1)
			{
				m_State = HTTP_STATE_HOST_NOT_FOUND;
				return 0;
			}

		}
		//Use either the proxy port or 80 if none specified
		hostaddr.sin_port = htons(m_ProxyPort?m_ProxyPort:80);
		//Copy the proxy address...
		memcpy(&hostaddr.sin_addr,&ip,4);

	}
	//Now we will connect to the host					
	fd_set	wfds;

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int serr = connect(m_DataSock, (SOCKADDR *)&hostaddr, sizeof(SOCKADDR));
	int cerr = WSAGetLastError();
	if(serr)
	{
		#ifdef __LINUX__
		while((cerr==WSAEALREADY)||(cerr==WSAEINVAL)||(cerr==WSAEWOULDBLOCK)||(cerr==EINPROGRESS))
		#else
		while((cerr==WSAEALREADY)||(cerr==WSAEINVAL)||(cerr==WSAEWOULDBLOCK))
		#endif
		{
			FD_ZERO(&wfds);
			FD_SET( m_DataSock, &wfds );
			if(select(m_DataSock+1,NULL,&wfds,NULL,&timeout))
			{
				serr = 0;
				break;
			}
			if(m_Aborting)
				return 0;
			serr = connect(m_DataSock, (SOCKADDR *)&hostaddr, sizeof(SOCKADDR));
			if(serr == 0)
				break;
			cerr = WSAGetLastError();
			if(cerr==WSAEISCONN)
			{
				serr = 0;
				break;
			}
		};
	}
	if(serr)
	{
		m_State = HTTP_STATE_CANT_CONNECT;
		return 0;
	}
	m_State = HTTP_STATE_CONNECTED;
	return 1;
}

char *ChttpGet::GetHTTPLine()
{
	unsigned int iBytesRead;
	char chunk[2];
	unsigned int igotcrlf = 0;
	memset(recv_buffer,0,1000);
	do
	{
		chunk[0]='\0';
		bool gotdata = false;
		do
		{
			iBytesRead = recv(m_DataSock,chunk,1,0);

			if(SOCKET_ERROR == iBytesRead)
			{	
				int error = WSAGetLastError();
				#ifdef __LINUX__
				if(WSAEWOULDBLOCK==error || 0==error)
				#else
				if(WSAEWOULDBLOCK==error)
				#endif
				{
					gotdata = false;
					continue;
				}
				else
				{
					return NULL;
				}
			}
			else
			{
				gotdata = true;
			}
		}while(!gotdata);
		
		if(chunk[0]==0x0d)
		{
			//This should always read a 0x0a
			do
			{
				iBytesRead = recv(m_DataSock,chunk,1,0);

				if(SOCKET_ERROR == iBytesRead)
				{	
					int error = WSAGetLastError();
					#ifdef __LINUX__
					if(WSAEWOULDBLOCK==error || 0==error)
					#else
					if(WSAEWOULDBLOCK==error)
					#endif
					{
						gotdata = false;
						continue;
					}
					else
					{
						return NULL;
					}
				}
				else
				{
					gotdata = true;
				}
			}while(!gotdata);
			igotcrlf = 1;	
		}
		else
		{	chunk[1] = '\0';
			strcat(recv_buffer,chunk);
		}
		
		
	}while(igotcrlf==0);
	return recv_buffer;	
}

unsigned int ChttpGet::ReadDataChannel()
{
	char sDataBuffer[4096];		// Data-storage buffer for the data channel
	int nBytesRecv;						// Bytes received from the data channel

	fd_set	wfds;

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500;

	m_State = HTTP_STATE_RECEIVING;			
   do	
   {
		FD_ZERO(&wfds);
		FD_SET( m_DataSock, &wfds );

		if((m_iBytesTotal)&&(m_iBytesIn==m_iBytesTotal))
		{
			break;
		}

		select(m_DataSock+1,&wfds,NULL,NULL,&timeout);

    		if(m_Aborting)
		{
			fclose(LOCALFILE);
			return 0;		
		}

		nBytesRecv = recv(m_DataSock, (char *)&sDataBuffer,sizeof(sDataBuffer), 0);

    		if(m_Aborting)
		{
			fclose(LOCALFILE);
			return 0;
		}

		if(SOCKET_ERROR == nBytesRecv)
		{	
			int error = WSAGetLastError();
			#ifdef __LINUX__
			if(WSAEWOULDBLOCK==error || 0==error)
			#else
			if(WSAEWOULDBLOCK==error)
			#endif
			{
				nBytesRecv = 1;
				continue;
			}
		}
		m_iBytesIn += nBytesRecv;
		if (nBytesRecv > 0 )
		{
			fwrite(sDataBuffer,nBytesRecv,1,LOCALFILE);
			//Write sDataBuffer, nBytesRecv
    		}
		

	}while (nBytesRecv > 0);

	fclose(LOCALFILE);							

	// Close the file and check for error returns.
	if (nBytesRecv == SOCKET_ERROR)
	{ 
		//Ok, we got a socket error -- xfer aborted?
		m_State = HTTP_STATE_RECV_FAILED;
		return 0;
	}
	else
	{
		//OutputDebugString("HTTP File complete!\n");
		//done!
		m_State = HTTP_STATE_FILE_RECEIVED;
		return 1;
	}
}	


typedef struct _async_dns_lookup
{
	unsigned int ip;	//resolved host. Write only to worker thread.
	char * host;//host name to resolve. read only to worker thread
	bool done;	//write only to the worker thread. Signals that the operation is complete
	bool error; //write only to worker thread. Thread sets this if the name doesn't resolve
	bool abort;	//read only to worker thread. If this is set, don't fill in the struct.

    #ifdef __LINUX__
    SDL_Thread *threadId;
    #endif
}async_dns_lookup;

async_dns_lookup httpaslu;
async_dns_lookup *http_lastaslu = NULL;

#ifndef __LINUX__
void __cdecl http_gethostbynameworker(void *parm);
#else
int http_gethostbynameworker(void *parm);
#endif

int http_Asyncgethostbyname(unsigned int *ip,int command, char *hostname)
{
	
	if(command==NW_AGHBN_LOOKUP)
	{
		if(http_lastaslu)
			http_lastaslu->abort = true;

		async_dns_lookup *newaslu;
		newaslu = (async_dns_lookup *)mem_malloc(sizeof(async_dns_lookup));
		memset(&newaslu->ip,0,sizeof(unsigned int));
		newaslu->host = hostname;
		newaslu->done = false;
		newaslu->error = false;
		newaslu->abort = false;
		http_lastaslu = newaslu;
		httpaslu.done = false;

#ifdef WIN32
		_beginthread(http_gethostbynameworker,0,newaslu);
#elif defined(__LINUX__)
//		pthread_t thread;
		if(!inet_LoadThreadLib())
		{
			return 0;
		}

//		df_pthread_create(&thread,NULL,http_gethostbynameworker,newaslu);
        newaslu->threadId = SDL_CreateThread(http_gethostbynameworker,newaslu);
#endif
		return 1;
	}
	else if(command==NW_AGHBN_CANCEL)
	{
		if(http_lastaslu)
			http_lastaslu->abort = true;

        #ifdef __LINUX__
            SDL_WaitThread(http_lastaslu->threadId, NULL);
        #endif

		http_lastaslu = NULL;
	}
	else if(command==NW_AGHBN_READ)
	{
		if(!http_lastaslu)
			return -1;
		if(httpaslu.done)
		{
			//free(http_lastaslu);
            #ifdef __LINUX__
                SDL_WaitThread(http_lastaslu->threadId, NULL);
            #endif

			http_lastaslu = NULL;
			memcpy(ip,&httpaslu.ip,sizeof(unsigned int));
			return 1;
		}
		else if(httpaslu.error)
		{
            #ifdef __LINUX__
                SDL_WaitThread(http_lastaslu->threadId, NULL);
            #endif

			mem_free(http_lastaslu);
			http_lastaslu = NULL;
			return -1;
		}
		else return 0;
	}
	return -2;

}

// This is the worker thread which does the lookup.
#ifndef __LINUX__
void __cdecl http_gethostbynameworker(void *parm)
#else
int http_gethostbynameworker(void *parm)
#endif
{
#ifdef __LINUX__
	//df_pthread_detach(df_pthread_self());
#endif
	async_dns_lookup *lookup = (async_dns_lookup *)parm;
	HOSTENT *he = gethostbyname(lookup->host);
	if(he==NULL)
	{
		lookup->error = true;
		#ifdef __LINUX__
		return NULL;
		#else
		return;
		#endif
	}
	else if(!lookup->abort)
	{
		memcpy(&lookup->ip,he->h_addr_list[0],sizeof(unsigned int));
		lookup->done = true;
		memcpy(&httpaslu,lookup,sizeof(async_dns_lookup));
	}
	mem_free(lookup);

#ifdef __LINUX__
	return NULL;
#endif
}

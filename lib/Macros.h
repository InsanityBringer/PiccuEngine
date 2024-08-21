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

#ifndef _MACROS_H
#define _MACROS_H
#define SWAP(a,b) do { int _swap_var_=(a); (a)=(b); (b)=_swap_var_; } while (0)
#define SET_MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define CHECK_FLAG(_var, _flag) ((_var) & (_flag))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define makeword(_h,_l) (((_h)<<16) + ((_l)&0xffff))
#define hiword(_v) ((_v)>>16)
#define loword(_v) ((_v)& 0x0000ffff)
#define makeshort(_h, _l) (((_h)<<8) + ((_l)&0x00ff))
#define hibyte(_w) ((_w)>> 8)
#define lobyte(_w) ((_w)& 0x00ff)
#define kb_to_bytes(_kb) ((_kb) * 1024)
#define _min(a,b) min(a,b)
#define _max(a,b) max(a,b)
#define ABS(a) ((a<0)?(-a):(a))

#ifndef WIN32
#define __min(a,b) min(a,b)
#define __max(a,b) max(a,b)
#endif

#ifdef LINUX
#define stricmp(a,b) strcasecmp(a,b)
#define _fstat32 fstat
#define _stat stat
#endif

#endif

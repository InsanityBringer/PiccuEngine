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

#ifndef __MANAGE_EXTERNAL_H_
#define __MANAGE_EXTERNAL_H_

#define PAGENAME_LEN	35

//Use this macro around your parameter in a table-file lookup, etc. to have the
//table file parser ignore this instance of the function (because you might be
//using one of the $$TABLE_ defines in a comment to force add items
#define IGNORE_TABLE(s)	s

//Use these macros around literal strings that are in arrays.
//NOTE that these strings must be inside curly braces
#define TBL_SOUND(s) s
#define TBL_GENERIC(s) s
#define TBL_GAMEFILE(s) s

#endif

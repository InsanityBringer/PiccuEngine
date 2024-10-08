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

#ifndef __AUDIO_ENCODE_H_
#define __AUDIO_ENCODE_H_

//input_levels (default 7 or for 2k total)
//input_samples (default 16 or for 2k total)
//input_rate (default 22K)
//input_channels (default 1)
//input_factor (compression factor) (default 4 for 22K, 8 for 44K)
//input_volscale (Volume scaling) (slightly <= 1.0, default ,97)
int aenc_Compress(char *input_filename,char *output_filename,int *input_levels=NULL,int *input_samples=NULL,int *input_rate=NULL,int *input_channels=NULL,float *input_factor=NULL,float *input_volscale=NULL);

#endif

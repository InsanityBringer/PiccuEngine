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

#include "CFILE.H"
#include "pserror.h"
#include "pstypes.h"
#include "bitmap.h"
#include "mono.h"
#include "grdefs.h"
#include "texture.h"
#include <string.h>
#include "mem.h"

#include <stdlib.h>

ubyte* Tga_file_data = NULL;
int Fake_pos = 0;
int Bad_tga = 0;
int Fake_file_size = 0;

inline char tga_read_byte()
{
	// Check for bad file
	if (Fake_pos + 1 > Fake_file_size)
	{
		Bad_tga = 1;
		return 0;
	}

	return Tga_file_data[Fake_pos++];
}

inline int tga_read_int()
{
	// Check for bad file
	if (Fake_pos + 4 > Fake_file_size)
	{
		Bad_tga = 1;
		return 0;
	}

	int i = Tga_file_data[Fake_pos] |
		(Tga_file_data[Fake_pos + 1] << 8) |
		(Tga_file_data[Fake_pos + 2] << 16) |
		(Tga_file_data[Fake_pos + 3] << 24);

	Fake_pos += 4;
	return i;
}

inline short tga_read_short()
{
	// Check for bad file
	if (Fake_pos + 2 > Fake_file_size)
	{
		Bad_tga = 1;
		return 0;
	}

	short i = Tga_file_data[Fake_pos] |
		(Tga_file_data[Fake_pos + 1] << 8);

	Fake_pos += 2;
	return i;
}

ushort bm_tga_translate_pixel(int pixel, int format)
{
	int red = ((pixel >> 16) & 0xFF);
	int green = ((pixel >> 8) & 0xFF);
	int blue = ((pixel) & 0xFF);
	int alpha = ((pixel >> 24) & 0xFF);
	ushort newpix;

	if (format == BITMAP_FORMAT_4444)
	{
		int newred = red >> 4;
		int newgreen = green >> 4;
		int newblue = blue >> 4;
		int newalpha = alpha >> 4;
		newpix = (newalpha << 12) | (newred << 8) | (newgreen << 4) | (newblue);
	}
	else
	{
		int newred = red >> 3;
		int newgreen = green >> 3;
		int newblue = blue >> 3;

		newpix = OPAQUE_FLAG | (newred << 10) | (newgreen << 5) | (newblue);

		if (alpha == 0)
			newpix = NEW_TRANSPARENT_COLOR;
	}

	return newpix;
}

int bm_tga_read_outrage_compressed16(CFILE* infile, int n, int num_mips, int type)
{
	ushort* dest_data;
	ushort pixel;
	int width, height;
	int m;

	for (m = 0; m < num_mips; m++)
	{
		width = bm_w(n, m);
		height = bm_h(n, m);

		int total = height * width;
		int count = 0;
		bool mipwarning = false;

		dest_data = (ushort*)bm_data(n, m);

		while (count != total)
		{
			if (count >= total && !mipwarning)
			{
				//This condition doesn't abort, but it will stop reading data. This is so that the correct amount of data is read. 
				mprintf((1, "Bad pixel data in image %s: count %d exceeds total %d!\n", infile->name, total, count));
				mipwarning = true;
			}

			ubyte command = tga_read_byte();

			if (Bad_tga)
				return 0;

			if (command == 0)	// next pixel is raw
			{
				pixel = tga_read_short();

				if (Bad_tga)
					return 0;

				if (type != OUTRAGE_1555_COMPRESSED_MIPPED && type != OUTRAGE_4444_COMPRESSED_MIPPED)
				{
					if (pixel == 0x07e0)
						pixel = NEW_TRANSPARENT_COLOR;
					else
					{
						int r = ((pixel & 0xF800) >> 11) << 3;
						int g = ((pixel & 0x07e0) >> 5) << 2;
						int b = (pixel & 0x001f) << 3;

						pixel = OPAQUE_FLAG | GR_RGB16(r, g, b);
					}
				}

				if (count < total)
				{
					int i = count / width;
					int t = count % width;
					dest_data[i * width + t] = pixel;
				}
				count++;
			}
			else if (command >= 2 && command <= 250)	// next pixel is run of pixels
			{
				pixel = tga_read_short();

				if (Bad_tga)
					return 0;

				if (type != OUTRAGE_1555_COMPRESSED_MIPPED && type != OUTRAGE_4444_COMPRESSED_MIPPED)
				{
					if (pixel == 0x07e0)
						pixel = NEW_TRANSPARENT_COLOR;
					else
					{
						int r = ((pixel & 0xF800) >> 11) << 3;
						int g = ((pixel & 0x07e0) >> 5) << 2;
						int b = (pixel & 0x001f) << 3;

						pixel = OPAQUE_FLAG | GR_RGB16(r, g, b);
					}
				}

				for (int k = 0; k < command; k++)
				{
					int i = count / width;
					int t = count % width;
					dest_data[i * width + t] = pixel;
					count++;
				}
			}
			else 
				Int3();		// bad compression run
		}
	}

	//DAJ added to fill out the mip maps down to the 1x1 size (memory is already there)
	//does not average since we are only a pixel or two in size
	if (num_mips > 1) {
		GameBitmaps[n].mip_levels = bm_miplevels(n);
		for (m = num_mips; m < bm_miplevels(n); m++) 
		{
			width = bm_w(n, m);
			height = bm_h(n, m);

			ushort w_prev = bm_w(n, m - 1);
			ushort* dst = bm_data(n, m);
			ushort* src = bm_data(n, m - 1);

			for (int h_inc = 0; h_inc < height; h_inc++)
			{
				for (int w_inc = 0; w_inc < width; w_inc++)
				{
					dst[h_inc * width + w_inc] = src[2 * (h_inc * w_prev + w_inc)];
				}
			}
		}
	}

	return 1;

}

// Loads a tga or ogf file into a bitmap...returns handle to bm or -1 on error
int bm_tga_alloc_file(CFILE* infile, char* name, int format)
{
	ubyte image_id_len, color_map_type, image_type, pixsize, descriptor;
	ubyte upside_down = 0;
	ushort width, height;
	unsigned int pixel;
	int i, t, n, data8bit = 0, savepos;
	int mipped = 0;
	int num_mips = 1;
	int read_ok = 1;

	image_id_len = cf_ReadByte(infile);
	color_map_type = cf_ReadByte(infile);
	image_type = cf_ReadByte(infile);

	if (color_map_type != 0 || (image_type != 10 && image_type != 2 && image_type != OUTRAGE_TGA_TYPE && image_type != OUTRAGE_COMPRESSED_OGF && image_type != OUTRAGE_COMPRESSED_MIPPED && image_type != OUTRAGE_NEW_COMPRESSED_MIPPED && image_type != OUTRAGE_1555_COMPRESSED_MIPPED && image_type != OUTRAGE_4444_COMPRESSED_MIPPED))
	{
		mprintf((0, "bm_tga: Can't read this type of TGA.\n"));
		return -1;
	}

	if (image_type == OUTRAGE_COMPRESSED_OGF_8BIT)
		data8bit = 1;

	if (image_type == OUTRAGE_4444_COMPRESSED_MIPPED || image_type == OUTRAGE_1555_COMPRESSED_MIPPED || image_type == OUTRAGE_NEW_COMPRESSED_MIPPED || image_type == OUTRAGE_TGA_TYPE || image_type == OUTRAGE_COMPRESSED_MIPPED || image_type == OUTRAGE_COMPRESSED_OGF || image_type == OUTRAGE_COMPRESSED_OGF_8BIT)
	{
		if (image_type == OUTRAGE_4444_COMPRESSED_MIPPED || image_type == OUTRAGE_NEW_COMPRESSED_MIPPED || image_type == OUTRAGE_1555_COMPRESSED_MIPPED)
		{
			cf_ReadString(name, BITMAP_NAME_LEN - 1, infile);
		}
		else
		{
			for (i = 0; i < BITMAP_NAME_LEN; i++)
				name[i] = cf_ReadByte(infile);
		}
		if (image_type == OUTRAGE_4444_COMPRESSED_MIPPED || image_type == OUTRAGE_1555_COMPRESSED_MIPPED || image_type == OUTRAGE_COMPRESSED_MIPPED || image_type == OUTRAGE_NEW_COMPRESSED_MIPPED)
			num_mips = cf_ReadByte(infile);
		else
			num_mips = 1;

		if (num_mips > 1)
			mipped = 1;
	}

	for (i = 0; i < 9; i++)			// ingore next 9 bytes
		cf_ReadByte(infile);

	width = cf_ReadShort(infile);
	height = cf_ReadShort(infile);
	pixsize = cf_ReadByte(infile);

	if (pixsize != 32 && pixsize != 24)
	{
		mprintf((0, "bm_tga: This file has a pixsize of field of %d, it should be 32. ", pixsize));
		return -1;
	}

	descriptor = cf_ReadByte(infile);
	if (((descriptor & 0x0F) != 8) && ((descriptor & 0x0F) != 0))
	{
		mprintf((0, "bm_tga: Descriptor field & 0x0F must be 8 or 0, but this is %d.", descriptor & 0x0F));
		return -1;
	}

	for (i = 0; i < image_id_len; i++)
		cf_ReadByte(infile);

	n = bm_AllocBitmap(width, height, mipped * ((width * height * 2) / 3));

	if (format == BITMAP_FORMAT_4444 || image_type == OUTRAGE_4444_COMPRESSED_MIPPED)
		GameBitmaps[n].format = BITMAP_FORMAT_4444;

	// Copy the name
	strcpy(GameBitmaps[n].name, name);

	if (mipped)
		GameBitmaps[n].flags |= BF_MIPMAPPED;

	if (n < 0)
	{
		mprintf((0, "bm_tga: Failed to allocate memory.\n"));
		Int3();
		return -1;
	}

	upside_down = (descriptor & 0x20) >> 5;
	upside_down = 1 - upside_down;

	// Load the actual bitmap data in, converting it from 32 bit to 16 bit, and replacing
	// that pesky transparency color without our replacement
	if (image_type == 10 || image_type == 2)
	{
		if (image_type == 10)		// compressed tga
		{
			int total = 0;

			while (total < (height * width))
			{
				ubyte command = cf_ReadByte(infile);
				ubyte len = (command & 127) + 1;

				if (command & 128)	// rle chunk
				{
					if (pixsize == 32)
						pixel = cf_ReadInt(infile);
					else
					{
						int r, g, b;
						r = cf_ReadByte(infile);
						g = cf_ReadByte(infile);
						b = cf_ReadByte(infile);
						pixel = (255 << 24) | (r << 16) | (g << 8) | b;
					}

					ushort newpix = bm_tga_translate_pixel(pixel, format);

					for (int k = 0; k < len; k++, total++)
					{
						i = total / width;
						t = total % width;

						if (upside_down)
							GameBitmaps[n].data16[((height - 1) - i) * width + t] = newpix;
						else
							GameBitmaps[n].data16[i * width + t] = newpix;
					}
				}
				else	// raw chunk
				{
					for (int k = 0; k < len; k++, total++)
					{
						if (pixsize == 32)
							pixel = cf_ReadInt(infile);
						else
						{
							int r, g, b;
							b = (ubyte)cf_ReadByte(infile);
							g = (ubyte)cf_ReadByte(infile);
							r = (ubyte)cf_ReadByte(infile);
							pixel = (255 << 24) | (r << 16) | (g << 8) | b;
						}
						ushort newpix = bm_tga_translate_pixel(pixel, format);

						i = total / width;
						t = total % width;

						if (upside_down)
							GameBitmaps[n].data16[((height - 1) - i) * width + t] = newpix;
						else
							GameBitmaps[n].data16[i * width + t] = newpix;
					}
				}
			}
		}
		else	// uncompressed TGA
		{
			for (i = 0; i < height; i++)
			{
				for (t = 0; t < width; t++)
				{
					if (pixsize == 32)
						pixel = cf_ReadInt(infile);
					else
					{
						int r, g, b;
						b = (ubyte)cf_ReadByte(infile);
						g = (ubyte)cf_ReadByte(infile);
						r = (ubyte)cf_ReadByte(infile);
						pixel = (255 << 24) | (r << 16) | (g << 8) | b;
					}

					ushort newpix = bm_tga_translate_pixel(pixel, format);

					if (upside_down)
						GameBitmaps[n].data16[((height - 1) - i) * width + t] = newpix;
					else
						GameBitmaps[n].data16[i * width + t] = newpix;
				}
			}
		}
	}
	else if (image_type == OUTRAGE_4444_COMPRESSED_MIPPED || image_type == OUTRAGE_1555_COMPRESSED_MIPPED || image_type == OUTRAGE_NEW_COMPRESSED_MIPPED || image_type == OUTRAGE_COMPRESSED_MIPPED || image_type == OUTRAGE_COMPRESSED_OGF || image_type == OUTRAGE_COMPRESSED_OGF_8BIT) // COMPRESSED OGF
	{
		// read this ogf in all at once (much faster)

		savepos = cftell(infile);
		cfseek(infile, 0, SEEK_END);
		int lastpos = cftell(infile);
		int numleft = lastpos - savepos;

		cfseek(infile, savepos, SEEK_SET);

		Tga_file_data = (ubyte*)mem_malloc(numleft);
		ASSERT(Tga_file_data != NULL);
		Fake_pos = 0;
		Bad_tga = 0;
		Fake_file_size = numleft;

		cf_ReadBytes((ubyte*)Tga_file_data, numleft, infile);

		read_ok = bm_tga_read_outrage_compressed16(infile, n, num_mips, image_type);
	}

	else
		Int3();		// Get Jason

	if (image_type == OUTRAGE_COMPRESSED_OGF_8BIT)
		Int3();
	//bm_tga_read_outrage_compressed8 (infile,n);

	if (Tga_file_data != NULL)
	{
		mem_free(Tga_file_data);
		Tga_file_data = NULL;
		cfseek(infile, savepos + Fake_pos, SEEK_SET);
	}

	if (!read_ok)
		return -1;
	else
		return (n);
}

extern int paged_in_count;
extern int paged_in_num;
// Pages in bitmap index n.  Returns 1 if successful, 0 if not
int bm_page_in_file(int n)
{
	ubyte image_id_len, color_map_type, image_type, pixsize, descriptor;
	ubyte upside_down = 0;
	ushort width, height;
	int i, data8bit = 0, savepos;
	int mipped = 0, file_mipped = 0;
	int num_mips = 1;
	char name[BITMAP_NAME_LEN];
	CFILE* infile;

	ASSERT((GameBitmaps[n].flags & BF_NOT_RESIDENT));

	infile = (CFILE*)cfopen(GameBitmaps[n].name, "rb");
	if (!infile)
	{
		mprintf((0, "Couldn't page in bitmap %s!\n", GameBitmaps[n].name));
		return 0;
	}
	//Used for progress bar when loading the level
	paged_in_count += cfilelength(infile);
	paged_in_num++;
	image_id_len = cf_ReadByte(infile);
	color_map_type = cf_ReadByte(infile);
	image_type = cf_ReadByte(infile);

	if (color_map_type != 0 || (image_type != 10 && image_type != 2 && image_type != OUTRAGE_TGA_TYPE && image_type != OUTRAGE_COMPRESSED_OGF && image_type != OUTRAGE_COMPRESSED_MIPPED && image_type != OUTRAGE_NEW_COMPRESSED_MIPPED && image_type != OUTRAGE_1555_COMPRESSED_MIPPED && image_type != OUTRAGE_4444_COMPRESSED_MIPPED))
	{
		mprintf((0, "bm_tga: Can't read this type of TGA.\n"));
		return -1;
	}

	if (image_type == OUTRAGE_COMPRESSED_OGF_8BIT)
		data8bit = 1;

	if (image_type == OUTRAGE_4444_COMPRESSED_MIPPED || image_type == OUTRAGE_1555_COMPRESSED_MIPPED || image_type == OUTRAGE_NEW_COMPRESSED_MIPPED || image_type == OUTRAGE_TGA_TYPE || image_type == OUTRAGE_COMPRESSED_MIPPED || image_type == OUTRAGE_COMPRESSED_OGF || image_type == OUTRAGE_COMPRESSED_OGF_8BIT)
	{
		if (image_type == OUTRAGE_4444_COMPRESSED_MIPPED || image_type == OUTRAGE_1555_COMPRESSED_MIPPED || image_type == OUTRAGE_NEW_COMPRESSED_MIPPED)
		{
			cf_ReadString(name, BITMAP_NAME_LEN - 1, infile);
		}
		else
		{
			for (i = 0; i < BITMAP_NAME_LEN; i++)
				name[i] = cf_ReadByte(infile);
		}
		if (image_type == OUTRAGE_4444_COMPRESSED_MIPPED || image_type == OUTRAGE_1555_COMPRESSED_MIPPED || image_type == OUTRAGE_COMPRESSED_MIPPED || image_type == OUTRAGE_NEW_COMPRESSED_MIPPED)
			num_mips = cf_ReadByte(infile);
		else
			num_mips = 1;

		if (num_mips > 1)
			file_mipped = 1;
	}

	for (i = 0; i < 9; i++)			// ingore next 9 bytes
		cf_ReadByte(infile);

	width = cf_ReadShort(infile);
	height = cf_ReadShort(infile);
	pixsize = cf_ReadByte(infile);

	if (pixsize != 32 && pixsize != 24)
	{
		mprintf((0, "bm_tga: This file has a pixsize of field of %d, it should be 32. ", pixsize));
		return 0;
	}

	descriptor = cf_ReadByte(infile);
	if (((descriptor & 0x0F) != 8) && ((descriptor & 0x0F) != 0))
	{
		mprintf((0, "bm_tga: Descriptor field & 0x0F must be 8 or 0, but this is %d.", descriptor & 0x0F));
		return 0;
	}

	for (i = 0; i < image_id_len; i++)
		cf_ReadByte(infile);

	if ((GameBitmaps[n].flags & BF_WANTS_MIP) || file_mipped)
		mipped = 1;

	int size = (width * height * 2) + (mipped * ((width * height * 2) / 3)) + 2;
	GameBitmaps[n].data16 = (ushort*)mem_malloc(size);
	if (!GameBitmaps[n].data16)
	{
		mprintf((0, "Out of memory in bm_page_in_file!\n"));
		return 0;
	}

	Bitmap_memory_used += size;


	if ((GameBitmaps[n].flags & BF_WANTS_4444) || image_type == OUTRAGE_4444_COMPRESSED_MIPPED)
		GameBitmaps[n].format = BITMAP_FORMAT_4444;
	else
		GameBitmaps[n].format = BITMAP_FORMAT_STANDARD;

	GameBitmaps[n].width = width;
	GameBitmaps[n].height = height;
	GameBitmaps[n].flags &= ~BF_NOT_RESIDENT;

	// Copy the name
//	if ((stricmp(GameBitmaps[n].name,name)))
//			Int3(); //Get Jason!

	strcpy(GameBitmaps[n].name, name);

	mprintf((0, "Paging in bitmap %s!\n", GameBitmaps[n].name));

	if (file_mipped)
		GameBitmaps[n].flags |= BF_MIPMAPPED;

	upside_down = (descriptor & 0x20) >> 5;
	upside_down = 1 - upside_down;

	// Load the actual bitmap data in, converting it from 32 bit to 16 bit, and replacing
	// that pesky transparency color without our replacement
	if (image_type == OUTRAGE_4444_COMPRESSED_MIPPED || image_type == OUTRAGE_1555_COMPRESSED_MIPPED || image_type == OUTRAGE_NEW_COMPRESSED_MIPPED || image_type == OUTRAGE_COMPRESSED_MIPPED || image_type == OUTRAGE_COMPRESSED_OGF || image_type == OUTRAGE_COMPRESSED_OGF_8BIT) // COMPRESSED OGF
	{
		// read this ogf in all at once (much faster)

		savepos = cftell(infile);
		cfseek(infile, 0, SEEK_END);
		int lastpos = cftell(infile);
		int numleft = lastpos - savepos;

		cfseek(infile, savepos, SEEK_SET);

		Tga_file_data = (ubyte*)mem_malloc(numleft);
		ASSERT(Tga_file_data != NULL);
		Fake_pos = 0;
		Bad_tga = 0;
		Fake_file_size = numleft;

		cf_ReadBytes((ubyte*)Tga_file_data, numleft, infile);

		bm_tga_read_outrage_compressed16(infile, n, num_mips, image_type);
	}

	else
		Int3();		// Get Jason

	if (image_type == OUTRAGE_COMPRESSED_OGF_8BIT)
		Int3();
	//bm_tga_read_outrage_compressed8 (infile,n);

	if (Tga_file_data != NULL)
	{
		mem_free(Tga_file_data);
		Tga_file_data = NULL;
		cfseek(infile, savepos + Fake_pos, SEEK_SET);
	}

	cfclose(infile);


	if ((GameBitmaps[n].flags & BF_WANTS_MIP) && !file_mipped)
	{
		bm_GenerateMipMaps(n);
	}

	return 1;
}


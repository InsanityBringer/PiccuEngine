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

#include <string.h>

static void gspy_key_swap(unsigned char *a, unsigned char *b)
{
	unsigned char x = *a;
	*a = *b;
	*b = x;
}

static void gspy_gen_key(unsigned char *secret, int secretlen, unsigned char *key)
{
	for (int i = 0; i < 256; i++)
		key[i] = i;
	key[256] = key[257] = 0;
	int si = 0;
	int newi = 0;
	for (int i = 0; i < 256; i++)
	{
		newi = (secret[si] + key[i] + newi) % 256;
		gspy_key_swap(key + i, key + newi);
		si = (si + 1) % secretlen;
	}
}

static void gspy_encrypt_with_key(unsigned char *buf, int len, unsigned char *key)
{
	int pos1 = key[256], pos2 = key[257];
	for (int i = 0; i < len; i++)
	{
		pos1 = (pos1 + 1 + buf[i]) % 256;
		pos2 = (pos2 + key[pos1]) % 256;
		gspy_key_swap(key + pos1, key + pos2);
		buf[i] ^= key[(key[pos1] + key[pos2]) % 256];
	}
	key[256] = pos1;
	key[257] = pos2;
}

void gspy_encrypt(unsigned char *val, int len, unsigned char *secret)
{
	unsigned char key[260];
	int secretlen = strlen((char *)secret);
	gspy_gen_key(secret, secretlen, key);
	gspy_encrypt_with_key(val, len, key);
}

static char gspy_b64_char(int n)
{
	return n < 26 ? n + 'A' :
		n < 52 ? n - 26 + 'a' :
		n < 62 ? n - 52 + '0' :
		n == 62 ? '+' : '/';
}

static void gspy_b64_encode_part(unsigned char *val, char *out)
{
	out[0] = gspy_b64_char(val[0] >> 2);
	out[1] = gspy_b64_char(((val[0] & 3) << 4) | (val[1] >> 4));
	out[2] = gspy_b64_char(((val[1] & 15) << 2) | (val[2] >> 6));
	out[3] = gspy_b64_char(val[2] & 63);
}

void gspy_encode(unsigned char *val, int len, unsigned char *result)
{
	unsigned char part[3];
	char partout[4];

	int val_idx = 0;
	while (val_idx < len)
	{
		for (int j = 0; j < 3; j++)
		{
			part[j] = val_idx < len ? val[val_idx] : 0;
			val_idx++;
		}
		gspy_b64_encode_part(part, partout);
		memcpy(result, partout, 4);
		result += 4;
	}
	*result = 0;
}

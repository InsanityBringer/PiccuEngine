/*
Copyright (c) 2019 SaladBadger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//hogdir: takes a directory from the command line and packages it into a hog
//file for usage in <s>ICDP</s><s>Neptune</s>Piccu Engine. 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <filesystem>
#include <wchar.h>
#include <vector>
#include <stdexcept>

constexpr int FILENAME_LEN = 36;
constexpr int DIRENTRY_LEN = FILENAME_LEN + 12;
const char* sig = "HOG2";

FILE* hogfile;

struct fileinfo_data
{
	std::filesystem::path fullpath;
	char name[FILENAME_LEN];
	uint32_t size;
};

void write_uint32_t(FILE* fp, uint32_t value)
{
	uint8_t buffer[4];
	buffer[0] = value & 255;
	buffer[1] = (value >> 8) & 255;
	buffer[2] = (value >> 16) & 255;
	buffer[3] = (value >> 24) & 255;

	fwrite(buffer, 1, 4, fp);
}

void generate_header(std::vector<fileinfo_data>& files)
{
	fwrite(sig, 1, strlen(sig), hogfile);
	write_uint32_t(hogfile, files.size());
	write_uint32_t(hogfile, files.size() * DIRENTRY_LEN + 68);

	uint8_t padding[56];
	memset(padding, 0xFF, sizeof(padding));
	fwrite(padding, 1, sizeof(padding), hogfile);

	for (fileinfo_data& file : files)
	{
		uint32_t placeholder = 0;
		fwrite(file.name, 1, FILENAME_LEN, hogfile);
		write_uint32_t(hogfile, placeholder); //flags
		write_uint32_t(hogfile, file.size); //file size
		write_uint32_t(hogfile, placeholder); //modified time but the build system don't care
	}

	//wait why not just write all the files now
	for (fileinfo_data& file : files)
	{
		std::string fullpath = file.fullpath.u8string();
		FILE* fp = fopen(fullpath.c_str(), "rb");
		if (!fp)
		{
			char errorstr[256];
			snprintf(errorstr, sizeof(errorstr), "generate_header: Cannot open file %s!", fullpath.c_str());
			throw std::runtime_error(errorstr);
		}
			
		uint8_t* buffer = (uint8_t*)malloc(file.size);
		if (!buffer)
		{
			char errorstr[256];
			snprintf(errorstr, sizeof(errorstr), "generate_header: Error allocating buffer for file %s!", fullpath.c_str());
			throw std::runtime_error(errorstr);
		}

		fread(buffer, 1, file.size, fp);
		fclose(fp);

		fwrite(buffer, 1, file.size, hogfile);

		free(buffer);
	}
}

void add_dir(const char* directory)
{
	if (strlen(directory) == 0)
	{
		fprintf(stderr, "Zero-length search string\n");
		return;
	}

	std::filesystem::path path(directory);
	std::filesystem::directory_iterator it = std::filesystem::directory_iterator(path);

	std::vector<fileinfo_data> pathlist;

	for (std::filesystem::directory_entry const& entry : it)
	{
		if (entry.is_regular_file())
		{
			//Eventually, Piccu should support UTF-8 proper. Eventually.
			std::string filename = entry.path().filename().u8string();

			if (filename.size() >= FILENAME_LEN)
				continue;

			uintmax_t filesize = entry.file_size();
			if (filesize > UINT32_MAX) //why not
				continue;

			fileinfo_data file = {};
			file.fullpath = entry.path();
			file.size = (uint32_t)filesize;
			strncpy(file.name, filename.c_str(), FILENAME_LEN);

			pathlist.push_back(file);

			/*pathlist.push_back(entry.path());
			std::filesystem::path const& filepath = entry.path();
#ifdef _MSC_VER
			FILE* fp = _wfopen(filepath.c_str(), L"rb");
#else
			FILE* fp = fopen(filepath.c_str(), "rb");
#endif
			if (!fp)
			{
#ifdef _MSC_VER
				fprintf(stderr, "Failed to open file %ls.\n", filepath.c_str());
#else
				fprintf(stderr, "Failed to open file %s.\n", filepath.c_str());
#endif
			}
			else
			{
				add_file(filepath.filename().c_str(), fp);
				fclose(fp);
			}*/
		}
	}

	//Generate the header
	generate_header(pathlist);
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("usage: hogdir [output file] [source dir]\n");
		return 0;
	}

	hogfile = fopen(argv[1], "wb");
	if (!hogfile)
	{
		fprintf(stderr, "Failed to open output file %s.\n", argv[1]);
		return 1;
	}

	try
	{
		add_dir(argv[2]);
	}
	catch (const std::runtime_error& err)
	{
		fprintf(stderr, "Error creating hogfile %s:\n%s\n", argv[1], err.what());
	}

	fclose(hogfile);

	return 0;
}


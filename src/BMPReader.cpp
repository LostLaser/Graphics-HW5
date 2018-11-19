#include "BMPReader.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>

using namespace std;

BMPImage::BMPImage() :
	data(0), w(0), h(0)
{
}

BMPImage::~BMPImage()
{
	reset();
}
void BMPImage::reset(int image_width, int image_height)
{
	if (data)
	{
		delete[] data;
	}
	if (image_width <= 0 || image_height <= 0)						// Invalid input.
	{
		w = 0;
		h = 0;
		data = 0;
	}
	else
	{
		w = image_width;
		h = image_height;
		data = new color[w * h];
		memset(data, 0, w * h * sizeof(color));
	}
}

void BMPImage::read(const char *file_name)
{
	ifstream ifs;
	ifs.open(file_name, ios::binary);
	if (ifs.bad())
	{
		cout << "Fail to open file: " << file_name << "." << endl;
		return;
	}

	unsigned char *buffer = new unsigned char[30];
	ifs.read((char *)buffer, 30);									// Read first 30 bytes.
	int data_offset = *(int *)(buffer + 0x0A);						// Actual data address.
	int _w = *(int *)(buffer + 0x12);								// Width.
	int _h = *(int *)(buffer + 0x16);								// Height.
	int bit = (*(int *)(buffer + 0x1A)) >> 16;						// Bits per pixel.
	reset(_w, _h);													// Reset the image.
	ifs.ignore(data_offset - 30);									// Skip useless bits.
	int byte_per_line = ((bit * w + 31) / 32) * 4;					// How many bytes per row.
	delete[] buffer;
	buffer = new unsigned char[byte_per_line];


	if (bit == 24)													// 24 bpp: bgr.
	{
		for (int i = 0; i < h; i++)
		{
			ifs.read((char *)buffer, byte_per_line);
			for (int j = 0; j < w; j++)
			{
				data[i * w + j].b = buffer[3 * j];
				data[i * w + j].g = buffer[3 * j + 1];
				data[i * w + j].r = buffer[3 * j + 2];
				data[i * w + j].a = 255;
			}
		}
	}
	else if (bit == 32)												// 32 bpp: bgra.
	{
		for (int i = 0; i < h; i++)
		{
			ifs.read((char *)buffer, byte_per_line);
			for (int j = 0; j < w; j++)
			{
				data[i * w + j].b = buffer[4 * j];
				data[i * w + j].g = buffer[4 * j + 1];
				data[i * w + j].r = buffer[4 * j + 2];
				data[i * w + j].a = buffer[4 * j + 3];
			}
		}
	}
	else
	{
		cout << "Error: Doesn't support " << bit << "-bit images." << endl;
		return;
	}
}

void BMPImage::write(const char *file_name)
{
	ofstream ofs;
	ofs.open(file_name, ios::binary);
	if (ofs.bad())
	{
		cout << "Fail to open file: " << file_name << "." << endl;
		return;
	}
	unsigned char buffer[54];										// Header, 54 bytes.
	memset(buffer, 0, 54);
	buffer[0] = 'B';
	buffer[1] = 'M';
	*(int *)(buffer + 0x02) = 54 + w * h * 4;						// Total size.
	*(int *)(buffer + 0x0A) = 54;									// Header size.
	*(int *)(buffer + 0x0E) = 40;									// DIB header size.
	*(int *)(buffer + 0x12) = w;									// Width.
	*(int *)(buffer + 0x16) = h;									// Height.
	buffer[0x1A] = 1;												// Constant 1
	buffer[0x1C] = 32;												// 32 bpp.
	*(int *)(buffer + 0x22) = w * h * 4;							// data size.
	*(int *)(buffer + 0x26) = 2835;									// DPI = 72.
	*(int *)(buffer + 0x2A) = 2835;									// DPI = 72.

	ofs.write((char *)buffer, 54);									// Write header.
	ofs.write((char *)data, w * h * 4);								// Write data.

	ofs.close();
}

color& BMPImage::at(const int i, const int j)
{
	if (i < 0 || i >= h || j < 0 || j >= w)
	{
		cout << "Error: Access error!" << endl;
	}
	return data[(h - 1 - i) * w + j];
}
int BMPImage::getWidth() const
{
	return w;
}
int BMPImage::getHeight() const
{
	return h;
}

color* BMPImage::getData() const
{
	return data;
}
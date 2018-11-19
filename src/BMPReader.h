#pragma once

struct color
{
	unsigned char b, g, r, a;				// R, G, B and alpha. 0-255.
};

class BMPImage
{
private:
	color *data;							// Pixel data. Start from bottom left corner. Row major.
	int w, h;								// Image width and height
public:
	BMPImage();
	~BMPImage();
	// Reset image as blank with given size.
	void reset(int image_width = 0, int image_height = 0);
	void read(const char *file_name);		// Load a bmp file. Must be 24 or 32 bits per pixel format.
	void write(const char *file_name);		// Save current bmp file. 32 bits pre pixel format.
	color& at(const int i, const int j);	// Access to the i-th row j-th column pixel. Can be read or written.
	int getWidth() const;					// Image width.
	int getHeight() const;					// Image height.
	color* getData() const;					// Image data pointer.
};
#include "bmp.h"
#include "video.h"

#define H_RES   1024
#define V_RES   768

static Bitmap * background;

Bitmap * loadBitmap(const char* filename)
{
	// allocating necessary size
	Bitmap* bmp = (Bitmap*) malloc(sizeof(Bitmap));

	// open filename in read binary mode
	FILE *filePtr;
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
        return NULL;

	// read the bitmap file header
	BitmapFileHeader bitmapFileHeader;
	fread(&bitmapFileHeader, 2, 1, filePtr);

	// verify that this is a bmp file by check bitmap id
	if (bitmapFileHeader.type != 0x4D42) {
		fclose(filePtr);
		return NULL;
	}

	int rd;
	do {
		if ((rd = fread(&bitmapFileHeader.size, 4, 1, filePtr)) != 1)
			break;
		if ((rd = fread(&bitmapFileHeader.reserved, 4, 1, filePtr)) != 1)
			break;
		if ((rd = fread(&bitmapFileHeader.offset, 4, 1, filePtr)) != 1)
			break;
	} while (0);

	if (rd == !1) {
		fprintf(stderr, "Error reading file\n");
		exit(-1);
	}

	// read the bitmap info header
	BitmapInfoHeader bitmapInfoHeader;
	fread(&bitmapInfoHeader, sizeof(BitmapInfoHeader), 1, filePtr);

	// move file pointer to the begining of bitmap data
	fseek(filePtr, bitmapFileHeader.offset, SEEK_SET);

	// allocate enough memory for the bitmap image data
	uint32_t* bitmapImage = (uint32_t*) malloc(
			bitmapInfoHeader.image_size);

	// verify memory allocation
	if (!bitmapImage) {
		free(bitmapImage);
		fclose(filePtr);
		return NULL;
	}

	// read in the bitmap image data
	fread(bitmapImage, bitmapInfoHeader.image_size, 1, filePtr);

	// make sure bitmap image data was read
	if (bitmapImage == NULL) {
		fclose(filePtr);
		return NULL;
	}

	// close file and return bitmap image data
	fclose(filePtr);

	bmp->bitmapData = bitmapImage;
	bmp->bitmapInfoHeader = bitmapInfoHeader;

	return bmp;
}

void setBackground(Bitmap * bg)
{
	background = bg;
}

int drawBitmap(Bitmap* bmp, int x, int y, bool check_col) {

	if (bmp == NULL)
		return 1;
	int width = bmp->bitmapInfoHeader.width;
	int height = bmp->bitmapInfoHeader.height;
	int x_res = background->bitmapInfoHeader.width;
	int y_res = background->bitmapInfoHeader.height;

	if (x < 0 || x + width > H_RES || y < 0 || y + height > V_RES)
		return 1;

	void * buffer = get_buffer();

	if(check_col)
	{//Check for collisions
		for (int yc = y; yc < y + height; yc++)
    	{
        	for (int xc = x; xc < x + width; xc++)
        	{
		  	if(((uint32_t*)buffer)[yc*x_res+xc] != background->bitmapData[(y_res-yc-1)*x_res+xc])
		      if(((uint32_t*)buffer)[yc*x_res+xc] != CROSSHAIR_COLOR && (bmp->bitmapData[(height-(yc-y)-1)*width+(xc-x)] & 0xFF000000) != 0)
				return ((uint32_t*)buffer)[yc*x_res+xc];
        	}
    	}
	}

	for (int yc = y; yc < y + height; yc++)
    {
        for (int xc = x; xc < x + width; xc++)
        {
		  if((bmp->bitmapData[(height-(yc-y)-1)*width+(xc-x)] & 0xFF000000) != 0)
          	vg_draw_pixel(xc, yc, bmp->bitmapData[(height-(yc-y)-1)*width+(xc-x)]);
        }
    }
	return 0;
}

void hideBitmap(Bitmap * mainbitmap, int x, int y)
{
	if (mainbitmap == NULL)
		return;

	int width = mainbitmap->bitmapInfoHeader.width;
	int height = mainbitmap->bitmapInfoHeader.height;

	for (int yc = y; yc < y + height; yc++)
    {
        for (int xc = x; xc < x + width; xc++)
        {
			if (xc < 0 || xc > H_RES || yc < 0 || yc > V_RES)
				continue;
			if((mainbitmap->bitmapData[(height-(yc-y)-1)*width+(xc-x)] & 0xFF000000) != 0)
        		vg_draw_pixel(xc, yc, background->bitmapData[(V_RES-yc-1)*H_RES+xc]);
        }
    }
	return;
}

void deleteBitmap(Bitmap* bmp) {
	if (bmp == NULL)
		return;

	free(bmp->bitmapData);
	free(bmp);
}

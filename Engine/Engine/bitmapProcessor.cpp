#include "bitmapProcessor.h"


bitmapProcessor::bitmapProcessor(void)
{
	width = 0;
	height = 0;
	alphaChannelMap = 0;
}


bitmapProcessor::~bitmapProcessor(void)
{
}


void bitmapProcessor::readBMP(char* filename)
{
    FILE* f = fopen(filename, "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    width = *(int*)&info[18];
    height = *(int*)&info[22];

    long long int size = 4 * width * height;
	unsigned char* dataRaw = new unsigned char[size];
    alphaChannelMap = new D3DXVECTOR4*[height];
	for(int i = 0; i< height; i++)
	{
		alphaChannelMap[i] = new D3DXVECTOR4[width];
		for(int j = 0; j< width; j++)
		{
			alphaChannelMap[i][j] = D3DXVECTOR4(0,0,0,0);
		}
	}
    fread(dataRaw, sizeof(unsigned char), size, f);
    fclose(f);

	if(dataRaw == 0)
	{
		return;
	}

	for(int j = 0; j<height; j++)
	{
		for(int i = 0; i<width;i++)
		{
			alphaChannelMap[height-j-1][i].x=dataRaw[(j*height+i)*4+2];
			alphaChannelMap[height-j-1][i].y=dataRaw[(j*height+i)*4+1];
			alphaChannelMap[height-j-1][i].z=dataRaw[(j*height+i)*4];
			alphaChannelMap[height-j-1][i].w=dataRaw[(j*height+i)*4+3];	//looks ok... 

			if ( dataRaw[(j*height+i)*4+3] > 0.5f)
			{
				printf("wat");
			}
		}
	}

    delete[] dataRaw;
	dataRaw = 0;
}

//this to test all the rings and unite them!
void bitmapProcessor::drawImage(D3DXVECTOR4** image)
{
	FILE *f;
	unsigned char *img = NULL;
	int filesize = 54 + 3*width*height;  //w is your image width, h is image height, both int
	if( img )
		free( img );
	img = (unsigned char *)malloc(3*width*height);
	memset(img,0,sizeof(img));

	for(int i=0; i<width; i++)
	{
		for(int j=0; j<height; j++)
		{
			int x =i; 
			int y =(width-1)-j;
			unsigned char r = image[j][i].w;
			unsigned char g = 0;
			unsigned char b = 0;
			if (r > 255) r=255;
			if (g > 255) g=255;
			if (b > 255) b=255;
			img[(x+y*width)*3+2] = (unsigned char)(r);
			img[(x+y*width)*3+1] = (unsigned char)(g);
			img[(x+y*width)*3+0] = (unsigned char)(b);
		}
	}

	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
	unsigned char bmppad[3] = {0,0,0};

	bmpfileheader[ 2] = (unsigned char)(filesize    );
	bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
	bmpfileheader[ 4] = (unsigned char)(filesize>>16);
	bmpfileheader[ 5] = (unsigned char)(filesize>>24);

	bmpinfoheader[ 4] = (unsigned char)(       width    );
	bmpinfoheader[ 5] = (unsigned char)(       width>> 8);
	bmpinfoheader[ 6] = (unsigned char)(       width>>16);
	bmpinfoheader[ 7] = (unsigned char)(       width>>24);
	bmpinfoheader[ 8] = (unsigned char)(       height    );
	bmpinfoheader[ 9] = (unsigned char)(       height>> 8);
	bmpinfoheader[10] = (unsigned char)(       height>>16);
	bmpinfoheader[11] = (unsigned char)(       height>>24);
}

void bitmapProcessor::Shutdown()
{
	for (int i = 0; i<height; i++)
	{
		delete alphaChannelMap[i];
	}
	delete alphaChannelMap;
}
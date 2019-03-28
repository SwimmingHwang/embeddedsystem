#include <iostream>
#include <opencv2/opencv.hpp>
#include<stdlib.h>
#include<ctime>
#include<time.h>
#include<stdio.h>
#include <opencv/cv.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <dirent.h>
#include<math.h>
using namespace cv;

#define WIDTH 1024
#define HEIGHT 600


int main(int argc, char * argv[])
{

	//device open
	int fd;
	int i, j, k;


	FILE *fp;
	char buffer[600][2048] = { 0 };

	VideoCapture cap(atoi(argv[1]));

	Mat new_frame;
	Mat bkg_frame;
	Mat gray_frame;
	Mat diff_frame;
	Mat match_frame;

	unsigned short pix;
	unsigned short save_pix;
	unsigned short r;
	unsigned short g;
	unsigned short b;
	unsigned short *fdData;

	int flag_check_motion = 1;

	int flag1_1;
	int flag1_2;
	int flag1_3;
	int flag1_4;
	int flag1_5;

	int cnt_0 = 0;
	int cnt_255 = 0;

	Mat motion1_1_frame;
	Mat motion1_2_frame;
	Mat motion1_3_frame;
	Mat motion1_4_frame;
	Mat motion1_5_frame;
	Mat motion1_6_frame;

	Mat gray1_1_frame;
	Mat gray1_2_frame;
	Mat gray1_3_frame;
	Mat gray1_4_frame;
	Mat gray1_5_frame;
	Mat gray1_6_frame;

	Mat diff_motion1_frame;
	Mat diff_motion2_frame;
	Mat diff_motion3_frame;
	Mat diff_motion4_frame;
	Mat diff_motion5_frame;
	Mat diff_motion6_frame;

	int diffvalue1;
	int diffvalue2;
	int diffvalue3;
	int diffvalue4;
	int diffvalue5;
	int diffvalue6;


	// TFTLCD device open
	if ((fd = open("/dev/fb0", O_RDWR)) < 0)
	{
		printf("fb open error\n");
		exit(0);
	}

	//memory mapping
	fdData = (unsigned short*)mmap(0, WIDTH*HEIGHT * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	//screen initialize
	for (int i = 0; i < HEIGHT*WIDTH; i++) *(fdData + i) = 0x0000;


	//sleep
	clock_t clock_start = 0, clock_end = 0;
	clock_start = clock();
	clock_end = clock();

	int nThreshold = 50;


	//Sleep(1000)
	//left screen : mode print
	while ((clock() - clock_start) / CLOCKS_PER_SEC != 3)
	{
		//sleep(1000);	
		//frame = cvQueryFrame(cap);
		cap >> new_frame;
		//new_frame=cap.get(CV_CAP_PROP_FPS);
		//printf("Capture!!\n");
		clock_end = clock();
	}//in 1 seconds ,window3= captu

	bkg_frame = new_frame.clone();

	//generate bkg_frame
	//window 3
	for (int k =108 ; k >= 0; k--)
	{
		for (int j = 255; j >= 0; j--)
		{
			bkg_frame.at<Vec3b>(k, j)[0] = (new_frame.at<Vec3b>(k, j)[0] + new_frame.at<Vec3b>(k, j)[1] + new_frame.at<Vec3b>(k, j)[2]) / 3;
			bkg_frame.at<Vec3b>(k, j)[1] = bkg_frame.at<Vec3b>(k, j)[0];
			bkg_frame.at<Vec3b>(k, j)[2] = bkg_frame.at<Vec3b>(k, j)[0];

			//Save new_frame 's rgb pixel value
			pix = bkg_frame.at<Vec3b>(k, j)[0] / 8 + ((bkg_frame.at<Vec3b>(k, j)[1] / 4) << 5) + ((bkg_frame.at<Vec3b>(k, j)[2] / 8) << 11);
			//LCD sreen -> pixel value    
			*(fdData + (k + 491)*WIDTH + j + 511) = pix;
		}
	}

	//Sleep(1000)


	while (1)
	{

		//from video capture to new_frame
		cap >> new_frame;

		if ((clock() - clock_start) / CLOCKS_PER_SEC == 6)
		{
			motion1_1_frame=new_frame.clone();
			gray1_1_frame = motion1_1_frame;
			for (int k = 383; k >= 0; k--)
			{
				for (int j = 511; j >= 0; j--)
				{
					////Save new_frame 's rgb pixel value
					//pix = motion1_1_frame.at<Vec3b>(k, j)[0] / 8 +
					//	((motion1_1_frame.at<Vec3b>(k, j)[1] / 4) << 5)
					//	+ ((motion1_1_frame.at<Vec3b>(k, j)[2] / 8) << 11);
					gray1_1_frame.at<Vec3b>(k, j)[0] = (gray1_1_frame.at<Vec3b>(k, j)[0] + gray1_1_frame.at<Vec3b>(k, j)[1] + gray1_1_frame.at<Vec3b>(k, j)[2]) / 3;
					gray1_1_frame.at<Vec3b>(k, j)[1] = gray1_1_frame.at<Vec3b>(k, j)[0];
					gray1_1_frame.at<Vec3b>(k, j)[2] = gray1_1_frame.at<Vec3b>(k, j)[0];

					//motion1_1_frame = 2진화 프레임
					motion1_1_frame.at<Vec3b>(k, j)[0] = gray1_1_frame.at<Vec3b>(k, j)[0] - bkg_frame.at<Vec3b>(k, j)[0];
					if (abs(motion1_1_frame.at<Vec3b>(k, j)[0]) >= nThreshold)
					{
						//printf("%d", abs(diff_frame.at<Vec3b>(k, j)[0]));
						motion1_1_frame.at<Vec3b>(k, j)[0] = 255;
						motion1_1_frame.at<Vec3b>(k, j)[1] = motion1_1_frame.at<Vec3b>(k, j)[0];
						motion1_1_frame.at<Vec3b>(k, j)[2] = motion1_1_frame.at<Vec3b>(k, j)[0];
					}
					else
					{
						motion1_1_frame.at<Vec3b>(k, j)[0] = 0;
						motion1_1_frame.at<Vec3b>(k, j)[1] = motion1_1_frame.at<Vec3b>(k, j)[0];
						motion1_1_frame.at<Vec3b>(k, j)[2] = motion1_1_frame.at<Vec3b>(k, j)[0];

						pix = (motion1_1_frame.at<Vec3b>(k, j)[0] / 8)
							+ ((motion1_1_frame.at<Vec3b>(k, j)[1] / 4) << 5)
							+ ((motion1_1_frame.at<Vec3b>(k, j)[2] / 8) << 11);

						//motion1_1_frame 동작의 이진화 프레임
						//LCD sreen -> pixel value    
						*(fdData + (k + 0)*WIDTH + j + 0) = pix;

					}
				}
			}

			if ((clock() - clock_start) / CLOCKS_PER_SEC == 7)
			{

				motion1_2_frame = new_frame.clone();
				gray1_2_frame = motion1_2_frame.clone();
				
				for (int k = 383; k >= 0; k--)
				{
					for (int j = 511; j >= 0; j--)
					{
						gray1_2_frame.at<Vec3b>(k, j)[0] = (gray1_2_frame.at<Vec3b>(k, j)[0] + gray1_2_frame.at<Vec3b>(k, j)[1] + gray1_2_frame.at<Vec3b>(k, j)[2]) / 3;
						gray1_2_frame.at<Vec3b>(k, j)[1] = gray1_2_frame.at<Vec3b>(k, j)[0];
						gray1_2_frame.at<Vec3b>(k, j)[2] = gray1_2_frame.at<Vec3b>(k, j)[0];

						motion1_2_frame.at<Vec3b>(k, j)[0] = gray1_2_frame.at<Vec3b>(k, j)[0] - bkg_frame.at<Vec3b>(k, j)[0];
						if (abs(motion1_2_frame.at<Vec3b>(k, j)[0]) >= nThreshold)
						{
							//printf("%d", abs(diff_frame.at<Vec3b>(k, j)[0]));
							motion1_2_frame.at<Vec3b>(k, j)[0] = 255;
							motion1_2_frame.at<Vec3b>(k, j)[1] = motion1_2_frame.at<Vec3b>(k, j)[0];
							motion1_2_frame.at<Vec3b>(k, j)[2] = motion1_2_frame.at<Vec3b>(k, j)[0];
						}
						else
						{
							motion1_2_frame.at<Vec3b>(k, j)[0] = 0;
							motion1_2_frame.at<Vec3b>(k, j)[1] = motion1_2_frame.at<Vec3b>(k, j)[0];
							motion1_2_frame.at<Vec3b>(k, j)[2] = motion1_2_frame.at<Vec3b>(k, j)[0];

							pix = (motion1_2_frame.at<Vec3b>(k, j)[0] / 8)
								+ ((motion1_2_frame.at<Vec3b>(k, j)[1] / 4) << 5)
								+ ((motion1_2_frame.at<Vec3b>(k, j)[2] / 8) << 11);
							////Save new_frame 's rgb pixel value
							//pix = motion1_2_frame.at<Vec3b>(k, j)[0] / 8 +
							//	((motion1_2_frame.at<Vec3b>(k, j)[1] / 4) << 5)
							//	+ ((motion1_2_frame.at<Vec3b>(k, j)[2] / 8) << 11);
							////LCD sreen -> pixel value    
							*(fdData + (k + 0)*WIDTH + j + 0) = pix;

						}
					}
				}
			}
			if ((clock() - clock_start) / CLOCKS_PER_SEC == 8)
			{

				motion1_3_frame = new_frame.clone();
				gray1_3_frame = motion1_3_frame.clone();
				for (int k = 383; k >= 0; k--)
				{
					for (int j = 511; j >= 0; j--)
					{
						gray1_3_frame.at<Vec3b>(k, j)[0] = (gray1_3_frame.at<Vec3b>(k, j)[0] + gray1_3_frame.at<Vec3b>(k, j)[1] + gray1_3_frame.at<Vec3b>(k, j)[2]) / 3;
						gray1_3_frame.at<Vec3b>(k, j)[1] = gray1_3_frame.at<Vec3b>(k, j)[0];
						gray1_3_frame.at<Vec3b>(k, j)[2] = gray1_3_frame.at<Vec3b>(k, j)[0];

						motion1_3_frame.at<Vec3b>(k, j)[0] = gray1_3_frame.at<Vec3b>(k, j)[0] - bkg_frame.at<Vec3b>(k, j)[0];
						if (abs(motion1_3_frame.at<Vec3b>(k, j)[0]) >= nThreshold)
						{
							//printf("%d", abs(diff_frame.at<Vec3b>(k, j)[0]));
							motion1_3_frame.at<Vec3b>(k, j)[0] = 255;
							motion1_3_frame.at<Vec3b>(k, j)[1] = motion1_3_frame.at<Vec3b>(k, j)[0];
							motion1_3_frame.at<Vec3b>(k, j)[2] = motion1_3_frame.at<Vec3b>(k, j)[0];
						}
						else
						{
							motion1_3_frame.at<Vec3b>(k, j)[0] = 0;
							motion1_3_frame.at<Vec3b>(k, j)[1] = motion1_3_frame.at<Vec3b>(k, j)[0];
							motion1_3_frame.at<Vec3b>(k, j)[2] = motion1_3_frame.at<Vec3b>(k, j)[0];

							pix = (motion1_3_frame.at<Vec3b>(k, j)[0] / 8)
								+ ((motion1_3_frame.at<Vec3b>(k, j)[1] / 4) << 5)
								+ ((motion1_3_frame.at<Vec3b>(k, j)[2] / 8) << 11);
							////Save new_frame 's rgb pixel value
							//pix = motion1_3_frame.at<Vec3b>(k, j)[0] / 8 +
							//	((motion1_3_frame.at<Vec3b>(k, j)[1] / 4) << 5)
							//	+ ((motion1_3_frame.at<Vec3b>(k, j)[2] / 8) << 11);
							//LCD sreen -> pixel value    
							*(fdData + (k + 0)*WIDTH + j + 0) = pix;

						}
					}
				}
			}
			if ((clock() - clock_start) / CLOCKS_PER_SEC == 9)
			{

				motion1_4_frame = new_frame.clone();
				gray1_4_frame = motion1_4_frame.clone();
				for (int k = 383; k >= 0; k--)
				{
					for (int j = 511; j >= 0; j--)
					{

						gray1_4_frame.at<Vec3b>(k, j)[0] = (gray1_4_frame.at<Vec3b>(k, j)[0] + gray1_4_frame.at<Vec3b>(k, j)[1] + gray1_4_frame.at<Vec3b>(k, j)[2]) / 3;
						gray1_4_frame.at<Vec3b>(k, j)[1] = gray1_4_frame.at<Vec3b>(k, j)[0];
						gray1_4_frame.at<Vec3b>(k, j)[2] = gray1_4_frame.at<Vec3b>(k, j)[0];

						motion1_4_frame.at<Vec3b>(k, j)[0] = gray1_4_frame.at<Vec3b>(k, j)[0] - bkg_frame.at<Vec3b>(k, j)[0];
						if (abs(motion1_4_frame.at<Vec3b>(k, j)[0]) >= nThreshold)
						{
							//printf("%d", abs(diff_frame.at<Vec3b>(k, j)[0]));
							motion1_4_frame.at<Vec3b>(k, j)[0] = 255;
							motion1_4_frame.at<Vec3b>(k, j)[1] = motion1_4_frame.at<Vec3b>(k, j)[0];
							motion1_4_frame.at<Vec3b>(k, j)[2] = motion1_4_frame.at<Vec3b>(k, j)[0];
						}
						else
						{
							motion1_4_frame.at<Vec3b>(k, j)[0] = 0;
							motion1_4_frame.at<Vec3b>(k, j)[1] = motion1_4_frame.at<Vec3b>(k, j)[0];
							motion1_4_frame.at<Vec3b>(k, j)[2] = motion1_4_frame.at<Vec3b>(k, j)[0];

							pix = (motion1_4_frame.at<Vec3b>(k, j)[0] / 8)
								+ ((motion1_4_frame.at<Vec3b>(k, j)[1] / 4) << 5)
								+ ((motion1_4_frame.at<Vec3b>(k, j)[2] / 8) << 11);
							////Save new_frame 's rgb pixel value
							//pix = motion1_4_frame.at<Vec3b>(k, j)[0] / 8 +
							//	((motion1_4_frame.at<Vec3b>(k, j)[1] / 4) << 5)
							//	+ ((motion1_4_frame.at<Vec3b>(k, j)[2] / 8) << 11);
							//LCD sreen -> pixel value    
							*(fdData + (k + 0)*WIDTH + j + 0) = pix;

						}
					}
				}
			}
			if ((clock() - clock_start) / CLOCKS_PER_SEC == 10)
			{
				motion1_5_frame = new_frame.clone();
				gray1_5_frame = motion1_5_frame.clone();
				for (int k = 383; k >= 0; k--)
				{
					for (int j = 511; j >= 0; j--)
					{
						gray1_5_frame.at<Vec3b>(k, j)[0] = (gray1_5_frame.at<Vec3b>(k, j)[0] + gray1_5_frame.at<Vec3b>(k, j)[1] + gray1_5_frame.at<Vec3b>(k, j)[2]) / 3;
						gray1_5_frame.at<Vec3b>(k, j)[1] = gray1_5_frame.at<Vec3b>(k, j)[0];
						gray1_5_frame.at<Vec3b>(k, j)[2] = gray1_5_frame.at<Vec3b>(k, j)[0];

						motion1_5_frame.at<Vec3b>(k, j)[0] = gray1_5_frame.at<Vec3b>(k, j)[0] - bkg_frame.at<Vec3b>(k, j)[0];
						if (abs(motion1_5_frame.at<Vec3b>(k, j)[0]) >= nThreshold)
						{
							//printf("%d", abs(diff_frame.at<Vec3b>(k, j)[0]));
							motion1_5_frame.at<Vec3b>(k, j)[0] = 255;
							motion1_5_frame.at<Vec3b>(k, j)[1] = motion1_5_frame.at<Vec3b>(k, j)[0];
							motion1_5_frame.at<Vec3b>(k, j)[2] = motion1_5_frame.at<Vec3b>(k, j)[0];
						}
						else
						{
							motion1_5_frame.at<Vec3b>(k, j)[0] = 0;
							motion1_5_frame.at<Vec3b>(k, j)[1] = motion1_5_frame.at<Vec3b>(k, j)[0];
							motion1_5_frame.at<Vec3b>(k, j)[2] = motion1_5_frame.at<Vec3b>(k, j)[0];

							pix = (motion1_5_frame.at<Vec3b>(k, j)[0] / 8)
								+ ((motion1_5_frame.at<Vec3b>(k, j)[1] / 4) << 5)
								+ ((motion1_5_frame.at<Vec3b>(k, j)[2] / 8) << 11);
							//Save new_frame 's rgb pixel value
							//pix = motion1_5_frame.at<Vec3b>(k, j)[0] / 8 +
							//	((motion1_5_frame.at<Vec3b>(k, j)[1] / 4) << 5)
							//	+ ((motion1_5_frame.at<Vec3b>(k, j)[2] / 8) << 11);
							////LCD sreen -> pixel value    
							*(fdData + (k + 0)*WIDTH + j + 0) = pix;

						}
					}
				}
			}
			if ((clock() - clock_start) / CLOCKS_PER_SEC == 11)
			{
				motion1_6_frame = new_frame.clone();
				gray1_6_frame = motion1_6_frame.clone();
				for (int k = 383; k >= 0; k--)
				{
					for (int j = 511; j >= 0; j--)
					{
						gray1_6_frame.at<Vec3b>(k, j)[0] = (gray1_6_frame.at<Vec3b>(k, j)[0] + gray1_6_frame.at<Vec3b>(k, j)[1] + gray1_6_frame.at<Vec3b>(k, j)[2]) / 3;
						gray1_6_frame.at<Vec3b>(k, j)[1] = gray1_6_frame.at<Vec3b>(k, j)[0];
						gray1_6_frame.at<Vec3b>(k, j)[2] = gray1_6_frame.at<Vec3b>(k, j)[0];

						motion1_6_frame.at<Vec3b>(k, j)[0] = gray1_6_frame.at<Vec3b>(k, j)[0] - bkg_frame.at<Vec3b>(k, j)[0];
						if (abs(motion1_6_frame.at<Vec3b>(k, j)[0]) >= nThreshold)
						{
							//printf("%d", abs(diff_frame.at<Vec3b>(k, j)[0]));
							motion1_6_frame.at<Vec3b>(k, j)[0] = 255;
							motion1_6_frame.at<Vec3b>(k, j)[1] = motion1_6_frame.at<Vec3b>(k, j)[0];
							motion1_6_frame.at<Vec3b>(k, j)[2] = motion1_6_frame.at<Vec3b>(k, j)[0];
						}
						else
						{
							motion1_6_frame.at<Vec3b>(k, j)[0] = 0;
							motion1_6_frame.at<Vec3b>(k, j)[1] = motion1_6_frame.at<Vec3b>(k, j)[0];
							motion1_6_frame.at<Vec3b>(k, j)[2] = motion1_6_frame.at<Vec3b>(k, j)[0];

							pix = (motion1_6_frame.at<Vec3b>(k, j)[0] / 8)
								+ ((motion1_6_frame.at<Vec3b>(k, j)[1] / 4) << 5)
								+ ((motion1_6_frame.at<Vec3b>(k, j)[2] / 8) << 11);
							////Save new_frame 's rgb pixel value
							//pix = motion1_6_frame.at<Vec3b>(k, j)[0] / 8 +
							//	((motion1_6_frame.at<Vec3b>(k, j)[1] / 4) << 5)
							//	+ ((motion1_6_frame.at<Vec3b>(k, j)[2] / 8) << 11);

							////LCD sreen -> pixel value    
							*(fdData + (k + 0)*WIDTH + j + 0) = pix;

						}
					}
				}
			}
			//current movie=window1
			for (int k = 383; k >= 0; k--)
			{
				for (int j = 511; j >= 0; j--)
				{
					//Save new_frame 's rgb pixel value
					pix = new_frame.at<Vec3b>(k, j)[0] / 8 +
						((new_frame.at<Vec3b>(k, j)[1] / 4) << 5)
						+ ((new_frame.at<Vec3b>(k, j)[2] / 8) << 11);
					//LCD sreen -> pixel value    
					*(fdData + (k + 0)*WIDTH + j + 511) = pix;

				}
			}
			gray_frame = new_frame;
			//current movie gray scale=window2	
			for (int k = 383; k >= 0; k--)
			{
				for (int j = 511; j >= 0; j--)
				{
					//Save new_frame 's rgb pixel value
					//gray Scale
					gray_frame.at<Vec3b>(k, j)[0] = (new_frame.at<Vec3b>(k, j)[0] + new_frame.at<Vec3b>(k, j)[1] + new_frame.at<Vec3b>(k, j)[2]) / 3;
					gray_frame.at<Vec3b>(k, j)[1] = gray_frame.at<Vec3b>(k, j)[0];
					gray_frame.at<Vec3b>(k, j)[2] = gray_frame.at<Vec3b>(k, j)[0];

					pix = (gray_frame.at<Vec3b>(k, j)[0] / 8)
						+ ((gray_frame.at<Vec3b>(k, j)[1] / 4) << 5)
						+ ((gray_frame.at<Vec3b>(k, j)[2] / 8) << 11);


					//LCD sreen -> pixel value    
					//*(fdData + (k + 203)*WIDTH + j + 767) = pix;

				}
			}
			//window4=generate diff movie
			diff_frame = gray_frame;
			for (int k = 383; k >= 0; k--)
			{
				for (int j = 511; j >= 0; j--)
				{
					//Save new_frame 's rgb pixel value
					//gray Scale
					diff_frame.at<Vec3b>(k, j)[0] = (gray_frame.at<Vec3b>(k, j)[0] - bkg_frame.at<Vec3b>(k, j)[0]);

					if (abs(diff_frame.at<Vec3b>(k, j)[0]) >= nThreshold)
					{
						//printf("%d", abs(diff_frame.at<Vec3b>(k, j)[0]));
						diff_frame.at<Vec3b>(k, j)[0] = 255;
						diff_frame.at<Vec3b>(k, j)[1] = diff_frame.at<Vec3b>(k, j)[0];
						diff_frame.at<Vec3b>(k, j)[2] = diff_frame.at<Vec3b>(k, j)[0];
					}
					else
					{
						diff_frame.at<Vec3b>(k, j)[0] = 0;
						diff_frame.at<Vec3b>(k, j)[1] = diff_frame.at<Vec3b>(k, j)[0];
						diff_frame.at<Vec3b>(k, j)[2] = diff_frame.at<Vec3b>(k, j)[0];
					}
					pix = (diff_frame.at<Vec3b>(k, j)[0] / 8)
						+ ((diff_frame.at<Vec3b>(k, j)[1] / 4) << 5)
						+ ((diff_frame.at<Vec3b>(k, j)[2] / 8) << 11);


					//LCD sreen -> pixel value    
					//*(fdData + (k + 395)*WIDTH + j + 767) = pix;

				}
			}
			////new_window4=이진화영상-이진화 자세 
			for (int k = 383; k >= 0; k--)
			{
				for (int j = 511; j >= 0; j--)
				{
					//Save new_frame 's rgb pixel value
					//gray Scale
					if ((diff_frame.at<Vec3b>(k, j)[0] - motion1_1_frame.at<Vec3b>(k, j)[0]) == 0) //255-255 or 0-0
						cnt_0++;
					if (abs((diff_frame.at<Vec3b>(k, j)[0] - motion1_1_frame.at<Vec3b>(k, j)[0])) == 255) //255-0 of 0-255
						cnt_255++;
					//diff_motion1_frame.at<Vec3b>(k, j)[0] += (diff_frame.at<Vec3b>(k, j)[0] - motion1_1_frame.at<Vec3b>(k, j)[0]);


					pix = (diff_motion1_frame.at<Vec3b>(k, j)[0] / 8)
						+ ((diff_motion1_frame.at<Vec3b>(k, j)[1] / 4) << 5)
						+ ((diff_motion1_frame.at<Vec3b>(k, j)[2] / 8) << 11);


					//LCD sreen -> pixel value    
					*(fdData + (k + 395)*WIDTH + j + 767) = pix;
				}
			}
			if (clock() % CLOCKS_PER_SEC == 0)
			{
//				printf("pix=%d\n", pix);
				printf("cnt_0=%d,", cnt_0);
				printf("cnt_255=%d\n",cnt_255);
				cnt_0 = 0;
				cnt_255 = 0;
			}

			////window=5
			////gray motion frame
			//for (int k = 203; k >= 0; k--)
			//{
			//	for (int j = 255; j >= 0; j--)
			//	{
			//		r = buffer[600 * k / 204][2048 * j * 2 / 2048];
			//		printf("r= %d ,g = %d ,b= %d", r, g, b);
			//		//16bit color value -> rgb value
			//		r >> 11;
			//		g << 21;
			//		g >> 26;
			//		b << 27;
			//		b >> 27;

			//		//motion gray scale
			//		r = (r + g + b) / 3;
			//		g = r;
			//		b = r;

			//		int temp = abs(gray_frame.at<Vec3b>(k, j)[0] - r);

			//		////Save new_frame 's rgb pixel value
			//		//pix = ((temp) / 8) +
			//		//	(((temp) / 4) << 5) +
			//		//	(((temp) / 8) << 11);

			//		if (temp >= nThreshold)
			//		{
			//			//printf("%d", abs(diff_frame.at<Vec3b>(k, j)[0]));
			//			match_frame.at<Vec3b>(k, j)[0] = 255;
			//			match_frame.at<Vec3b>(k, j)[1] = match_frame.at<Vec3b>(k, j)[0];
			//			match_frame.at<Vec3b>(k, j)[2] = match_frame.at<Vec3b>(k, j)[0];
			//		}
			//		else
			//		{
			//			match_frame.at<Vec3b>(k, j)[0] = 0;
			//			match_frame.at<Vec3b>(k, j)[1] = match_frame.at<Vec3b>(k, j)[0];
			//			match_frame.at<Vec3b>(k, j)[2] = match_frame.at<Vec3b>(k, j)[0];
			//		}


			//		pix = (match_frame.at<Vec3b>(k, j)[0] / 8)
			//			+ ((match_frame.at<Vec3b>(k, j)[1] / 4) << 5)
			//			+ ((match_frame.at<Vec3b>(k, j)[2] / 8) << 11);

			//		if (clock() % 1000 == 0)
			//			printf("match pix = %d\n", pix);


			//		//LCD sreen -> pixel value   
			//		*(fdData + (k + 0)*WIDTH + j + 511) = pix;
			//		`
			////window==6
			//for (int k = 203; k >= 0; k--)
			//{
			//	for (int j = 255; j >= 0; j--)
			//	{
			//		r = buffer[k][j];
			//		g = buffer[k][j];
			//		b = buffer[k][j];

			//		//16bit color value -> rgb value
			//		r >> 11;
			//		g << 21;
			//		g >> 26;
			//		b << 27;
			//		b >> 27;

			//		pix = (b )
			//			+ ((g ) << 5)
			//			+ ((r ) << 11);
			//		*(fdData + (k + 0)*WIDTH + j + 767) = pix;
			//	}
			//}
			//window=6
			//
			//for (int k = 191; k >= 0; k--)
			//{
			//	for (int j = 255; j >= 0; j--)
			//	{
			//		//Save new_frame 's rgb pixel value
			//		pix = new_frame.at<Vec3b>(k, j)[0] / 8 +
			//			((new_frame.at<Vec3b>(k, j)[1] / 4) << 5)
			//			+ ((new_frame.at<Vec3b>(k, j)[2] / 8) << 11);
			//		//LCD sreen -> pixel value    
			//		*(fdData + (k + 203)*WIDTH + j + 511) = pix;

			//	}
			//}
		}

		//cvDestroyAllWindows();
	///	cvReleaseImage(&grayImage);
	//	cvReleaseImage(&diffImage);
	//	cvReleaseCapture(&capture);

//		return 0;
	}
	return 0;
}

// segTool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Readvideo.h"
#include <string>
#include <atlstr.h>
#include <opencv2\opencv.hpp>
#include "windows.h"
using namespace std;
using namespace cv;

void drawSkeleton(IplImage* tempImage, SLR_ST_Skeleton skeleton)
{
	for (int j=2; j<=11; j++)
	{
		CvPoint p1;
		p1.x = skeleton._2dPoint[j].x;
		p1.y = skeleton._2dPoint[j].y;
		cvCircle(tempImage,p1,2,cvScalar(225,0,0),3,8,0);
		if (j!=7 && j!=3 && j!=11)
		{
			CvPoint p2;
			p2.x = skeleton._2dPoint[j+1].x;
			p2.y = skeleton._2dPoint[j+1].y;
			cvLine(tempImage,p1,p2,cvScalar(0,0,225),3,8,0);
		}
		if (j==2)
		{
			CvPoint p2;
			p2.x = skeleton._2dPoint[j+2].x;
			p2.y = skeleton._2dPoint[j+2].y;
			CvPoint p3;
			p3.x = skeleton._2dPoint[j+6].x;
			p3.y = skeleton._2dPoint[j+6].y;
			cvLine(tempImage,p1,p2,cvScalar(0,0,225),3,8,0);
			cvLine(tempImage,p1,p3,cvScalar(0,0,225),3,8,0);
		}

	}

	int b[5];
	b[0] = 3;
	b[1] = 5;
	b[2] = 7;
	b[3] = 9;
	b[4] = 11;

	for (int j=0; j<5; j++)
	{
		for (int k=j+1; k<5; k++)
		{
			CvPoint p1;
			p1.x = skeleton._2dPoint[b[j]].x;
			p1.y = skeleton._2dPoint[b[j]].y;
			CvPoint p2;
			p2.x = skeleton._2dPoint[b[k]].x;
			p2.y = skeleton._2dPoint[b[k]].y;
			cvLine(tempImage,p1,p2,cvScalar(0,255,0),2,8,0);
		}
	}
}

void copyImg(IplImage* des, IplImage* src, int height, int width)
{
	for (int rows=0; rows<height; rows++)
	{
		uchar* src_ptr = (uchar*)(des->imageData + rows*des->widthStep);
		uchar* src_ptr_depth = (uchar*)(src->imageData + rows*src->widthStep);
		for (int cols=0; cols<width; cols++)
		{
			src_ptr[3*cols + 0] = src_ptr_depth[3*cols + 0];
			src_ptr[3*cols + 1] = src_ptr_depth[3*cols + 1];
			src_ptr[3*cols + 2] = src_ptr_depth[3*cols + 2];
		}
	}
}

int _tmain()
{
	CString videoFileName;
	Readvideo myReadVideo;
	int frameSize;
	videoFileName.Format("..\\data\\P01_0000_1_0_20121117.oni");
	string  s   =   (LPCTSTR)videoFileName;
	myReadVideo.readvideo(s);
	frameSize = myReadVideo.vColorData.size();
	cvNamedWindow("color_skeleton",1);
	cout<<"The total frame is: "<<frameSize<<endl;
	bool isShowColor = false;

	for (int i=0; i<frameSize; i++)
	{
		CString fileName;
		IplImage* showImage;
		fileName.Format("..\\output\\p%d.jpg",i);

		// Select RGB or depth image to show
		if (isShowColor)
			showImage = myReadVideo.vColorData[i];
		else
		{
			Mat depthMat = myReadVideo.retrieveColorDepth(myReadVideo.vDepthData[i]);
			showImage = cvCreateImage(cvSize(r_width,r_heigth),8,3);
			copyImg(showImage, &(IplImage)depthMat, r_heigth, r_width);
		}
  		

		// Draw the skeleton on the image
		drawSkeleton(showImage, myReadVideo.vSkeletonData[i]);

		cvShowImage("color_skeleton",showImage);
		//cvSaveImage(fileName,tempImage);
		cvReleaseImage(&showImage);
		
		cvWaitKey(10);
	}

	cvDestroyWindow("color_skeleton");
	cout<<"Done!"<<endl;
	getchar();
	return 0;
}


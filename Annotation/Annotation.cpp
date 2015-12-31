// segTool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Readvideo.h"
#include <string>
#include <atlstr.h>
#include <opencv2\opencv.hpp>
#include "windows.h"
#include <direct.h>
using namespace std;
using namespace cv;

ofstream outfile;
CvRect box;
bool Drawing=false;
CvPoint mouse;
int clickTime;
int frameID;
CvPoint l_hand;
CvPoint r_hand;


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

void DrawRect(IplImage*img,CvRect rect)
{
	cvRectangle(img,
		cvPoint(box.x,box.y),
		cvPoint(box.x+box.width,box.y+box.height),
		cvScalar(0,0,255));
}

void MouseDraw(int event,int x,int y,int flags,void*param)
{
	IplImage*img=(IplImage*)param;
	switch(event)
	{
	case CV_EVENT_MOUSEMOVE:
		{
			mouse.x = x;
			mouse.y = y;
			if(Drawing)
			{
				box.width=x-box.x;
				box.height=y-box.y;
			}
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		{
			if (clickTime == 0)
			{
				CString frameID_str;
				frameID_str.Format("%04d", frameID);
				outfile<<frameID_str<<" ";

				outfile<<x<<" "<<y<<" ";
				clickTime += 1;
			}
			else if (clickTime == 1)
			{
				outfile<<x<<" "<<y<<" ";
				clickTime += 1;
			}
// 			else if (clickTime == 2)
// 			{
// 				outfile<<x<<" "<<y<<" ";
// 				clickTime += 1;
// 			}

			
			Drawing=true;
			box=cvRect(x,y,0,0);
		}
		break;
	case CV_EVENT_LBUTTONUP:
		{
			if (clickTime == 1)
			{
				outfile<<x<<" "<<y<<" ";
			}
			else if (clickTime == 2)
			{
				outfile<<x<<" "<<y<<" "<<l_hand.x<<" "<<l_hand.y<<" "<<r_hand.x<<" "<<r_hand.y<<endl;
			}
// 			else if (clickTime == 3)
// 			{
// 				outfile<<x<<" "<<y<<endl;
// 			}

			Drawing=false;
			if (box.width<0)
			{
				box.x+=box.width;
				box.width*=-1;
			}
			if (box.height<0)
			{
				box.y+=box.height;
				box.height*=-1;
			}
			DrawRect(img,box);
		}
		break;
	}
}

int _tmain()
{
	int everyNframes = 5;   // Down sample
	bool isShowColor = true;   //true: output the color. false: output the depth.
	BOOL videoFindFlag;
	CFileFind videoFileFind;
	// A route containing all the sign videos to be labeled. 
	CString route = "E:\\continuousDemoSign4test\\allTheSentences\\S10_1\\*.oni";
	videoFindFlag = TRUE;
	videoFindFlag = videoFileFind.FindFile(route);
	while(videoFindFlag)
	{ 
		cvNamedWindow("color_skeleton",1);
		Drawing=false;
		cout<<"----------------------------------"<<endl;

		// Find the sign video
		videoFindFlag = videoFileFind.FindNextFile();
		CString videoFileName = videoFileFind.GetFilePath();

		Readvideo myReadVideo;
		string  s   =   (LPCTSTR)videoFileName;
		myReadVideo.readvideo(s);
		int frameSize = myReadVideo.vColorData.size();
		cout<<"The total frame is: "<<frameSize<<endl;
		
		IplImage* showImage;

		CString fileName_record;
		fileName_record.Format("..\\output\\Label_%s.txt", videoFileName.Right(25).Left(21));
		CString fileName_imageFolder;
		fileName_imageFolder.Format("..\\output\\Image_%s", videoFileName.Right(25).Left(21));
		_mkdir(fileName_imageFolder);  

		for (int i=0; i<frameSize; i+=everyNframes)
		{
			frameID = i;  // The global var
			clickTime = 0;  // Initial the click time to 0. 
			outfile.open(fileName_record,ios::out | ios::app);

			CString fileName_image;
			fileName_image.Format("%s\\%04d.jpg", fileName_imageFolder, i);

			// Select RGB or depth image to show
			if (isShowColor)
				showImage = myReadVideo.vColorData[i];
			else
			{
				Mat depthMat = myReadVideo.retrieveColorDepth(myReadVideo.vDepthData[i]);
				showImage = cvCreateImage(cvSize(r_width,r_heigth),8,3);
				copyImg(showImage, &(IplImage)depthMat, r_heigth, r_width);
			}

			//The image for saving
			IplImage* saveImage;
			saveImage = cvCreateImage(cvSize(r_width,r_heigth),8,3);
			copyImg(saveImage, showImage, r_heigth, r_width);

			// Draw the skeleton on the image
			//drawSkeleton(showImage, myReadVideo.vSkeletonData[i]);
			int boundLen = 60;

			// Draw the hand joints.
			CvPoint p7;
			p7.x = myReadVideo.vSkeletonData[i]._2dPoint[7].x;
			p7.y = myReadVideo.vSkeletonData[i]._2dPoint[7].y;
			CvPoint p11;
			p11.x = myReadVideo.vSkeletonData[i]._2dPoint[11].x;
			p11.y = myReadVideo.vSkeletonData[i]._2dPoint[11].y;
			cvCircle(showImage,p7,2,cvScalar(225,0,0),3,8,0);
			cvCircle(showImage,p11,2,cvScalar(225,0,0),3,8,0);
			l_hand = p7;
			r_hand = p11;
			// 		cvRectangle(showImage,cvPoint(p7.x-boundLen, p7.y-boundLen),
			// 			cvPoint(p7.x+boundLen, p7.y+boundLen),cvScalar(255,0,0), 0.5,8,0);
			// 		cvRectangle(showImage,cvPoint(p11.x-boundLen, p11.y-boundLen),
			// 			cvPoint(p11.x+boundLen, p11.y+boundLen),cvScalar(255,0,0),0.5,8,0);


			box = cvRect(0,0,-1,-1);
			cvSetMouseCallback("color_skeleton", MouseDraw, (void*)showImage);
			IplImage* temp = cvCreateImage(cvSize(r_width,r_heigth),8,3); //cvCloneImage(showImage);
			// Draw the bounding box of bands in the frame.
			while (1)
			{
				copyImg(temp, showImage, r_heigth, r_width);
				// Draw the guidelines. 
				cvLine(temp,cvPoint(0,mouse.y), cvPoint(r_width-1, mouse.y),cvScalar(200,200,200),1,8,0);
				cvLine(temp,cvPoint(mouse.x, 0), cvPoint(mouse.x, r_heigth-1),cvScalar(200,200,200),1,8,0);

				if(Drawing)
				{
					if (clickTime == 1) 
					{// If the frame has been labeled, the frame will be saved. 
						cvSaveImage(fileName_image,saveImage);
					}

					DrawRect(temp,box);
				}
				cvShowImage("color_skeleton",temp);

				// ESC to break and label the next frame.
				if(cvWaitKey(100)==27)
					break;
			}
			cvReleaseImage(&temp);
			cvReleaseImage(&saveImage);
			outfile.close();
			cout<<"Frame "<<i<<"/"<<frameSize<<" has been processed"<<endl;
		}

		if (!isShowColor)
			cvReleaseImage(&showImage);
		cvDestroyWindow("color_skeleton");
	}
	

	cout<<"Done!"<<endl;
	getchar();
	return 0;
}


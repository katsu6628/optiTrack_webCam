#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "cameralibrary.h"
#include <ctime>

using namespace std;
using namespace cv;

int rotateMat(Mat srcMat, int rotDeg, double scale)
{
	Point2f center = Point2f(srcMat.cols / 2, srcMat.rows / 2);
	Mat affine;
	getRotationMatrix2D(center, rotDeg, scale).copyTo(affine);
	warpAffine(srcMat, srcMat, affine, srcMat.size(), cv::INTER_CUBIC);
	return 0;
}

void setOptiCam(CameraLibrary::Camera *camera_)
{
	// start optiTrack camera
	camera_->SetVideoType(Core::MJPEGMode);
	camera_->SetExposure(9000);
	camera_->SetImagerGain(CameraLibrary::eImagerGain::Gain_Level0);
	camera_->SetThreshold(200);
	camera_->SetIntensity(15);
	camera_->SetShutterDelay(0);
	camera_->SetAEC(false);
	camera_->SetAGC(false);
	camera_->SetIRFilter(false);
	camera_->Start();
}

int main(int argh, char* argv[])
{
	// camDet: 0 - optiTrack
	//         1 - webCamera
	int camDet = 0;

	//// Define opticTrack camera
	CameraLibrary::Camera *camera_;
	CameraLibrary_EnableDevelopment();
	CameraLibrary::CameraManager::X().WaitForInitialization();
	camera_ = CameraLibrary::CameraManager::X().GetCamera();
	bool isConnected_ = !(camera_->IsDisconnected());
	if (isConnected_)
	{
		cout << "optiTrack is Connected!" << endl;
	}
	else
	{
		cout << "no OptiTrack" << endl;
		return 0;
	}
	setOptiCam(camera_);

	camera_->SetTextOverlay(true);
	CameraLibrary::Frame *frame_ = nullptr;
	// Define openCV Mat for webCam and optiTrack
	Mat iRedFrame = cv::Mat(camera_->PhysicalPixelHeight(), camera_->PhysicalPixelWidth(), CV_8UC3);

	// Define components for webcamera
	VideoCapture webcap;
	Mat webframe;
	int webcamNum = 0;

	// Define rotation angle of Mat
	double iRedDeg = 0;
	double webDeg = 0;
	cout << "Key Commands \n q - quit \n s - save \n u - iredCam: rotate +30 deg \n i - iredCam: rotate -30 deg \n o - webCam: rotate +30 deg \n p - webCam: rotate -30 deg \n c - change webCam \n b - switch optiTrack/webCam" << endl;
	while (1)
	{
		Mat grayiRed;
		Mat iredHist;
		Mat outputMat;
		double outputDeg;
		//Key commands
		int key = waitKey(1);

		switch (camDet) {
		case 0:
			//update frame by optiTrack
			frame_ = camera_->GetFrame();
			if (frame_)
			{
				frame_->Rasterize(camera_->PhysicalPixelWidth(), camera_->PhysicalPixelHeight(), iRedFrame.step, 24, iRedFrame.data);
				frame_->Release();
			}
			cvtColor(iRedFrame, grayiRed, COLOR_RGB2GRAY);
			equalizeHist(grayiRed, iredHist);
			outputMat = iredHist;
			outputDeg = iRedDeg;
			// change output camera type
			if (key == 'b')
			{
				camDet = 1;
				camera_->Release();
				webcap.open(webcamNum);
				if (!webcap.isOpened())
				{
					return -1;
				}

			}
			break;
		case 1:
			// update frame by webcams
			webcap >> webframe;
			outputMat = webframe;
			outputDeg = webDeg;
			// change web camera
			if (key == 'c')
			{
				webcap.release();
				webcamNum++;
				webcap.open(webcamNum);
				if (!webcap.isOpened())
				{
					webcamNum = 0;
					webcap.open(webcamNum);
				}
			}
			// change output camera type
			if (key == 'b')
			{
				camDet = 0;
				webcap.release();
				setOptiCam(camera_);
			}
			break;
		}

		// key commandsa
		if (key == 'q') // "Q" key down -> quit
		{
			break;
		}
		if (key == 's') //"S" key down -> save one frame
		{
			imwrite("screenshot.png", outputMat);
		}
		if (key == 'u')
		{
			iRedDeg += 30;
		}
		if (key == 'i')
		{
			iRedDeg -= 30;
		}
		if (key == 'o')
		{
			webDeg += 30;
		}
		if (key == 'p')
		{
			webDeg -= 30;
		}
		rotateMat(outputMat, outputDeg, 1);
		imshow("cameraOutput", outputMat);
	}
	destroyAllWindows();
	return 0;
}

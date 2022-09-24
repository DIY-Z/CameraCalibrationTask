#pragma once

#include <QThread>
#include <opencv2/opencv.hpp>
#include <QMutex>
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <cctype>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>
using namespace cv;
using namespace std;


class ThreadGetCamPic  : public QThread
{
	Q_OBJECT

public:
	ThreadGetCamPic(QObject *parent);
	~ThreadGetCamPic();

	void run();
	void cameraCalibration(const cv::Mat& view);
	
	bool m_openCalibration = true;

//signals是修饰信号函数的关键字
signals:
	void sigSendCurImg(const QImage& img);

private:
	bool m_bStop = true;
	QMutex m_mux;
};

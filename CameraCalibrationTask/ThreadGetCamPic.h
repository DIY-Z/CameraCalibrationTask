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
	clock_t prevTimestamp = 0;
	vector<vector<Point2f> > imagePoints;
	int nframes = 10;
	float squareSize = 1, aspectRatio = 1;
	Mat cameraMatrix, distCoeffs;

	bool runAndSave(const vector<vector<Point2f> >& imagePoints,
		Size imageSize, Size boardSize, float squareSize,
		float grid_width, bool release_object,
		float aspectRatio, int flags, Mat& cameraMatrix,
		Mat& distCoeffs, bool writeExtrinsics, bool writePoints, bool writeGrid);
	bool runCalibration(vector<vector<Point2f> > imagePoints,
		Size imageSize, Size boardSize,
		float squareSize, float aspectRatio,
		float grid_width, bool release_object,
		int flags, Mat& cameraMatrix, Mat& distCoeffs,
		vector<Mat>& rvecs, vector<Mat>& tvecs,
		vector<float>& reprojErrs,
		vector<Point3f>& newObjPoints,
		double& totalAvgErr);
	double computeReprojectionErrors(
		const vector<vector<Point3f> >& objectPoints,
		const vector<vector<Point2f> >& imagePoints,
		const vector<Mat>& rvecs, const vector<Mat>& tvecs,
		const Mat& cameraMatrix, const Mat& distCoeffs,
		vector<float>& perViewErrors);
	void calcChessboardCorners(Size boardSize, float squareSize, vector<Point3f>& corners);

//signals是修饰信号函数的关键字
signals:
	void sigSendCurImg(const QImage& img);
	void sendInner_cameraMatrix(const QString& str);
	void sendInner_distCoeffs(const QString& str);
	void sendInner_rotationMatrix(const QString& str);
	void sendInner_translationMatrix(const QString& str);
private:
	bool m_bStop = true;
	QMutex m_mux;
	bool visit = false;
};

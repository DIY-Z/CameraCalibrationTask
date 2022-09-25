#include "ThreadGetCamPic.h"
#include <QImage>

ThreadGetCamPic::ThreadGetCamPic(QObject *parent)
	: QThread(parent)
{}

ThreadGetCamPic::~ThreadGetCamPic()
{
    if (!this->isInterruptionRequested()) //如果线程没有停止
    { 
        {
            QMutexLocker lock(&m_mux);
            m_bStop = true;
        }
        this->requestInterruption(); //将线程停止掉
        this->wait();  //然后wait
    }
}

void ThreadGetCamPic::run()
{

    cv::VideoCapture stVideoCapture;
    //bool bRet = stVideoCapture.open("D:\\FaceForensics++\\original_sequences\\youtube\\c23\\videos\\000.mp4");
    bool bRet = stVideoCapture.open(0);  //打开摄像头只需改成0
    cv::Mat matTemp;
    cv::Mat view;           //暂存相机输入的图像,将其用于相机标定
    QImage imgTemp;

    m_bStop = false;
    while (!m_bStop) {
        stVideoCapture >> matTemp;
        if (matTemp.empty()) {
            //如果没有从视频中拿到图像,则等待20ms再继续
            msleep(20);
            continue;
        }
        matTemp.copyTo(view);

        if (m_openCalibration)
        {
            //进行相机标定
            cameraCalibration(view);
            view.copyTo(matTemp);
        }
        
        
        //BGR转为RGB
        cvtColor(matTemp, matTemp, cv::COLOR_BGR2RGB);

        imgTemp = QImage(matTemp.data, matTemp.cols, matTemp.rows, matTemp.step, QImage::Format_RGB888).copy();
        
        //emit关键字用于发出信号,当发送信号时,imgTemp信息也会一并发出
        emit sigSendCurImg(imgTemp);
        
        msleep(20);
    }


}

void ThreadGetCamPic::cameraCalibration(const cv::Mat& view)
{
    Mat viewGray;
    Size boardSize, imageSize;
    boardSize.width = 3;
    boardSize.height = 4;
    imageSize = view.size();
    vector<Point2f> pointbuf;       //用于存储检测到的内角点图像坐标位置
    cvtColor(view, viewGray, COLOR_BGR2GRAY);
    bool found;
    //findChessboardCorners方法是用于检测图片中的内角点(参数解释:https://blog.csdn.net/Kalenee/article/details/80672785)
    found = findChessboardCorners(view, boardSize, pointbuf,
        CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
    if (found)
    {
        //cornerSubPix方法是用于提取亚像素角点信息(参数解释:https://blog.csdn.net/Kalenee/article/details/80672785)
        cornerSubPix(viewGray, pointbuf, Size(11, 11),
            Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.0001));
        drawChessboardCorners(view, boardSize, Mat(pointbuf), found);
    }
    int delay = 1000;
    if (found && (clock() - prevTimestamp > delay * 1e-3 * CLOCKS_PER_SEC))
    {
        imagePoints.push_back(pointbuf);
        prevTimestamp = clock();
    }
    float grid_width = squareSize * (boardSize.width - 1);
    bool release_object = false;
    int flags = 0;
    if (imagePoints.size() >= (unsigned)nframes)
    {
        //printf("yes");
        bool ok = runAndSave(imagePoints, imageSize,
            boardSize, squareSize, grid_width, release_object, aspectRatio,
            flags, cameraMatrix, distCoeffs,
            false, false, false);
    }
        

}

bool ThreadGetCamPic::runAndSave(const vector<vector<Point2f>>& imagePoints, 
                            Size imageSize, Size boardSize, float squareSize, float grid_width, 
                            bool release_object, float aspectRatio, int flags, Mat& cameraMatrix, 
                            Mat& distCoeffs, bool writeExtrinsics, bool writePoints, bool writeGrid)
{
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;
    vector<Point3f> newObjPoints;

    bool ok = runCalibration(imagePoints, imageSize, boardSize, squareSize,
        aspectRatio, grid_width, release_object, flags, cameraMatrix, distCoeffs,
        rvecs, tvecs, reprojErrs, newObjPoints, totalAvgErr);
    printf("%s. avg reprojection error = %.7f\n",
        ok ? "Calibration succeeded" : "Calibration failed",
        totalAvgErr);

    if (ok && !visit)
    {
        //printf("ok");
        stringstream stream;
        QString str_cameraMatrix, str_distCoeffs;
        stream << cameraMatrix;
        str_cameraMatrix = QString::fromStdString(stream.str());

        stream.clear();
        stream << distCoeffs;
        str_distCoeffs = QString::fromStdString(stream.str());

        //发送信号
        emit sendInner_cameraMatrix(str_cameraMatrix);
        emit sendInner_distCoeffs(str_distCoeffs);
        //msleep(20);
        visit = true;

        //saveCameraParams(outputFilename, imageSize,
        //    boardSize, squareSize, aspectRatio,
        //    flags, cameraMatrix, distCoeffs,
        //    writeExtrinsics ? rvecs : vector<Mat>(),
        //    writeExtrinsics ? tvecs : vector<Mat>(),
        //    writeExtrinsics ? reprojErrs : vector<float>(),
        //    writePoints ? imagePoints : vector<vector<Point2f> >(),
        //    writeGrid ? newObjPoints : vector<Point3f>(),
        //    totalAvgErr);
    }
    return ok;
}

bool ThreadGetCamPic::runCalibration(vector<vector<Point2f>> imagePoints, Size imageSize, Size boardSize, float squareSize, float aspectRatio, float grid_width, bool release_object, int flags, Mat& cameraMatrix, Mat& distCoeffs, vector<Mat>& rvecs, vector<Mat>& tvecs, vector<float>& reprojErrs, vector<Point3f>& newObjPoints, double& totalAvgErr)
{
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if (flags & CALIB_FIX_ASPECT_RATIO)
        cameraMatrix.at<double>(0, 0) = aspectRatio;

    distCoeffs = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f> > objectPoints(1);
    calcChessboardCorners(boardSize, squareSize, objectPoints[0]);
    objectPoints[0][boardSize.width - 1].x = objectPoints[0][0].x + grid_width;
    newObjPoints = objectPoints[0];

    objectPoints.resize(imagePoints.size(), objectPoints[0]);

    double rms;
    int iFixedPoint = -1;
    if (release_object)
        iFixedPoint = boardSize.width - 1;
    rms = calibrateCameraRO(objectPoints, imagePoints, imageSize, iFixedPoint,
        cameraMatrix, distCoeffs, rvecs, tvecs, newObjPoints,
        flags | CALIB_FIX_K3 | CALIB_USE_LU);
    printf("RMS error reported by calibrateCamera: %g\n", rms);

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    if (release_object) {
        cout << "New board corners: " << endl;
        cout << newObjPoints[0] << endl;
        cout << newObjPoints[boardSize.width - 1] << endl;
        cout << newObjPoints[boardSize.width * (boardSize.height - 1)] << endl;
        cout << newObjPoints.back() << endl;
    }

    objectPoints.clear();
    objectPoints.resize(imagePoints.size(), newObjPoints);
    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
        rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    return ok;
}

double ThreadGetCamPic::computeReprojectionErrors(const vector<vector<Point3f>>& objectPoints, const vector<vector<Point2f>>& imagePoints, const vector<Mat>& rvecs, const vector<Mat>& tvecs, const Mat& cameraMatrix, const Mat& distCoeffs, vector<float>& perViewErrors)
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for (i = 0; i < (int)objectPoints.size(); i++)
    {
        projectPoints(Mat(objectPoints[i]), rvecs[i], tvecs[i],
            cameraMatrix, distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), NORM_L2);
        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float)std::sqrt(err * err / n);
        totalErr += err * err;
        totalPoints += n;
    }

    return std::sqrt(totalErr / totalPoints);
}

void ThreadGetCamPic::calcChessboardCorners(Size boardSize, float squareSize, vector<Point3f>& corners)
{
    corners.resize(0);
    for (int i = 0; i < boardSize.height; i++)
        for (int j = 0; j < boardSize.width; j++)
            corners.push_back(Point3f(float(j * squareSize),
                float(i * squareSize), 0));
}

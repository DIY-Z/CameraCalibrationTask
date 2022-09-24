#include "ThreadGetCamPic.h"
#include <QImage>

ThreadGetCamPic::ThreadGetCamPic(QObject *parent)
	: QThread(parent)
{}

ThreadGetCamPic::~ThreadGetCamPic()
{
    if (!this->isInterruptionRequested()) //����߳�û��ֹͣ
    { 
        {
            QMutexLocker lock(&m_mux);
            m_bStop = true;
        }
        this->requestInterruption(); //���߳�ֹͣ��
        this->wait();  //Ȼ��wait
    }
}

void ThreadGetCamPic::run()
{

    cv::VideoCapture stVideoCapture;
    //bool bRet = stVideoCapture.open("D:\\FaceForensics++\\original_sequences\\youtube\\c23\\videos\\000.mp4");
    bool bRet = stVideoCapture.open(0);  //������ͷֻ��ĳ�0
    cv::Mat matTemp;
    cv::Mat view;           //�ݴ���������ͼ��,������������궨
    QImage imgTemp;

    m_bStop = false;
    while (!m_bStop) {
        stVideoCapture >> matTemp;
        if (matTemp.empty()) {
            //���û�д���Ƶ���õ�ͼ��,��ȴ�20ms�ټ���
            msleep(20);
            continue;
        }
        matTemp.copyTo(view);

        if (m_openCalibration)
        {
            //��������궨
            cameraCalibration(view);
            view.copyTo(matTemp);
        }
        

        //BGRתΪRGB
        cvtColor(matTemp, matTemp, cv::COLOR_BGR2RGB);

        imgTemp = QImage(matTemp.data, matTemp.cols, matTemp.rows, matTemp.step, QImage::Format_RGB888).copy();
        //emit�ؼ������ڷ����ź�,�������ź�ʱ,imgTemp��ϢҲ��һ������
        emit sigSendCurImg(imgTemp);
        msleep(20);
    }


}

void ThreadGetCamPic::cameraCalibration(const cv::Mat& view)
{
    Mat viewGray;
    Size boardSize;
    boardSize.width = 3;
    boardSize.height = 4;

    vector<Point2f> pointbuf;
    cvtColor(view, viewGray, COLOR_BGR2GRAY);

    bool found;
    found = findChessboardCorners(view, boardSize, pointbuf,
        CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
    if (found)
    {
        cornerSubPix(viewGray, pointbuf, Size(11, 11),
            Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.0001));
        drawChessboardCorners(view, boardSize, Mat(pointbuf), found);
    }
        

}

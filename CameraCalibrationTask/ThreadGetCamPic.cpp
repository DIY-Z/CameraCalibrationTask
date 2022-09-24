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
        
        //BGRתΪRGB
        cvtColor(matTemp, matTemp, cv::COLOR_BGR2RGB);

        imgTemp = QImage(matTemp.data, matTemp.cols, matTemp.rows, matTemp.step, QImage::Format_RGB888).copy();
        //emit�ؼ������ڷ����ź�,�������ź�ʱ,imgTemp��ϢҲ��һ������
        emit sigSendCurImg(imgTemp);
        msleep(20);
    }


}
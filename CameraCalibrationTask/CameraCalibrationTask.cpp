#include "CameraCalibrationTask.h"

CameraCalibrationTask::CameraCalibrationTask(QWidget *parent)
    : QMainWindow(parent)
    , m_stThreadGetCamPic(this)
{
    ui.setupUi(this);


    connect(&m_stThreadGetCamPic, &ThreadGetCamPic::sigSendCurImg, this, &CameraCalibrationTask::onFreshCurImg);
    connect(ui.openCameraBtn, &QPushButton::clicked, this, &CameraCalibrationTask::onOpenCamera);
}

CameraCalibrationTask::~CameraCalibrationTask()
{}


void CameraCalibrationTask::onFreshCurImg(const QImage& img)
{
    m_imgSrc = img.copy();   //如果对视频中的帧图像或者摄像头拍到的视频的图像帧有用处的话,可以先将其暂存

    m_img2Show = m_imgSrc.scaled(ui.oldPic->size(), Qt::KeepAspectRatio, Qt::FastTransformation);

    m_pix2Show = QPixmap::fromImage(m_img2Show);

    ui.oldPic->setPixmap(m_pix2Show);
}


void CameraCalibrationTask::onOpenCamera()
{
    //启动这个线程
    m_stThreadGetCamPic.start();
}
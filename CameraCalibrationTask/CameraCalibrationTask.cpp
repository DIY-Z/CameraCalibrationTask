#include "CameraCalibrationTask.h"

CameraCalibrationTask::CameraCalibrationTask(QWidget *parent)
    : QMainWindow(parent)
    , m_stThreadGetCamPic(this)
{
    ui.setupUi(this);


    connect(&m_stThreadGetCamPic, &ThreadGetCamPic::sigSendCurImg, this, &CameraCalibrationTask::onFreshCurImg);
    connect(ui.openCameraBtn, &QPushButton::clicked, this, &CameraCalibrationTask::onOpenCamera);
    connect(&m_stThreadGetCamPic, &ThreadGetCamPic::sendInner_cameraMatrix, this, &CameraCalibrationTask::onSetInnerText_cameraMatrix);
    connect(&m_stThreadGetCamPic, &ThreadGetCamPic::sendInner_distCoeffs, this, &CameraCalibrationTask::onSetInnerText_distCoeffs);
    connect(&m_stThreadGetCamPic, &ThreadGetCamPic::sendInner_rotationMatrix, this, &CameraCalibrationTask::onSetInnerText_rotationMatrix);
    connect(&m_stThreadGetCamPic, &ThreadGetCamPic::sendInner_translationMatrix, this, &CameraCalibrationTask::onSetInnerText_translationMatrix);
    connect(&m_stThreadGetCamPic, &ThreadGetCamPic::sendUndistortedImg, this, &CameraCalibrationTask::onFreshUndistortedImg);
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

void CameraCalibrationTask::onSetInnerText_cameraMatrix(const QString& str)
{
    ui.textEdit_inner->append("cameraMatrix: " + str);
}

void CameraCalibrationTask::onSetInnerText_distCoeffs(const QString& str)
{
    ui.textEdit_inner->append("distCoeffs: " + str);
}

void CameraCalibrationTask::onSetInnerText_rotationMatrix(const QString& str)
{
    ui.textEdit_outer->append("rotationMatrix: " + str);
}

void CameraCalibrationTask::onSetInnerText_translationMatrix(const QString& str)
{
    ui.textEdit_outer->append("translationMatrix: " + str);
}

void CameraCalibrationTask::onFreshUndistortedImg(const QImage& img)
{
    m_imgSrc = img.copy();   //如果对视频中的帧图像或者摄像头拍到的视频的图像帧有用处的话,可以先将其暂存

    m_img2Show = m_imgSrc.scaled(ui.oldPic->size(), Qt::KeepAspectRatio, Qt::FastTransformation);

    m_pix2Show = QPixmap::fromImage(m_img2Show);

    ui.newPic->setPixmap(m_pix2Show);
}

void CameraCalibrationTask::onOpenCamera()
{
    //启动这个线程
    m_stThreadGetCamPic.start();
}
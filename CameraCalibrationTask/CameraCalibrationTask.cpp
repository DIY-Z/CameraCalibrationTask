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
}

CameraCalibrationTask::~CameraCalibrationTask()
{}


void CameraCalibrationTask::onFreshCurImg(const QImage& img)
{
    m_imgSrc = img.copy();   //�������Ƶ�е�֡ͼ���������ͷ�ĵ�����Ƶ��ͼ��֡���ô��Ļ�,�����Ƚ����ݴ�

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

void CameraCalibrationTask::onOpenCamera()
{
    //��������߳�
    m_stThreadGetCamPic.start();
}
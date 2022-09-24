#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CameraCalibrationTask.h"
#include <opencv2/opencv.hpp>
#include "ThreadGetCamPic.h"

class CameraCalibrationTask : public QMainWindow
{
    Q_OBJECT

public:
    CameraCalibrationTask(QWidget *parent = nullptr);
    ~CameraCalibrationTask();
    
public slots:
    void onFreshCurImg(const QImage& img);
    void onOpenCamera();

private:
    Ui::CameraCalibrationTaskClass ui;

    QImage m_imgSrc;
    QImage m_img2Show;
    QPixmap m_pix2Show;

    ThreadGetCamPic m_stThreadGetCamPic;  
};

#include "CameraCalibrationTask.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CameraCalibrationTask w;
    w.show();
    return a.exec();
}

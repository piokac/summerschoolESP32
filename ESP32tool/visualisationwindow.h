#ifndef VISUALISATIONWINDOW_H
#define VISUALISATIONWINDOW_H

#include "visualisationgl.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>
#include "madgwickahrs.h"
#include <QtCore/qmath.h>

//! [1]
class visualisationWindow : public visualisationGL
{
    Q_OBJECT
public:
    visualisationWindow();

    void initialize() override;
    void render() override;

public slots:
    void newDataIMU(QVector<short> accelData, QVector<short> gyroData, QVector<short> magnetData);

private:
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;
    QOpenGLShaderProgram *m_program;
    int m_frame;
    GLfloat ModelMatrix[16];
    QMatrix4x4 MMatrix;
    QQuaternion Qwat;
    MadgwickAHRS MadAHR;

};






#endif // VISUALISATIONWINDOW_H



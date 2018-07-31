#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "geometryengine.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QBasicTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

class GeometryEngine;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    ~GLWidget();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void initShaders();
    void initTextures();
    QQuaternion getBias() const;

private:
    QOpenGLShaderProgram program;
    GeometryEngine *geometries;

    QOpenGLTexture *texture;

    QMatrix4x4 projection;

    QQuaternion rotation;

    QQuaternion bias;
    bool removeBias;
public:
    void updateOrientation(float w, float x, float y, float z);
    void setBias(QQuaternion &value);
    bool getRemoveBias() const;
    void setRemoveBias(bool value);
public slots:
        void setRemoveBiasFalse();

};

#endif // GLWIDGET_H

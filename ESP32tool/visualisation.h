#ifndef VISUALISATION_H
#define VISUALISATION_H

#include <QWidget>

namespace Ui {
class visualisation;
}

class visualisation : public QWidget
{
    Q_OBJECT


protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void initShaders();
    void initTextures();

private:
    QOpenGLShaderProgram program;
    GeometryEngine *geometries;

    QOpenGLTexture *texture;

    QMatrix4x4 projection;

    QQuaternion rotation;
    Ui::visualisation *ui;

public:
    explicit visualisation(QWidget *parent = 0);
    void updateOrientation(float w, float x, float y, float z);
    ~visualisation();

};

#endif // VISUALISATION_H

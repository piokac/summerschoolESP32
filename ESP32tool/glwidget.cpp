#include <QtWidgets>
#include <QtOpenGL>
#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent) :
     QOpenGLWidget(parent),
     geometries(0),
     texture(0)
{
    bias.setScalar(1);
    bias.setX(0);
    bias.setY(0);
    bias.setZ(0);
    removeBias=true;
}

GLWidget::~GLWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete texture;
    delete geometries;
    doneCurrent();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    geometries = new GeometryEngine;

    // Light setup


}
void GLWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}
//! [3]

//! [4]
void GLWidget::initTextures()
{
    // Load cube.png image
    texture = new QOpenGLTexture(QImage(":/cube.png").mirrored());

    // Set nearest filtering mode for texture minification
    texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture->setWrapMode(QOpenGLTexture::Repeat);
}

QQuaternion GLWidget::getBias() const
{
    return bias;
}

void GLWidget::setBias(QQuaternion &value)
{
    bias=value.inverted();
}

bool GLWidget::getRemoveBias() const
{
    return removeBias;
}

void GLWidget::setRemoveBias(bool value)
{
    removeBias=value;
}
void GLWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texture->bind();

    // Calculate model view transformation
    QMatrix4x4 matrix;
    matrix.translate(0.0, 0.0, -5.0);

    //qDebug()<<bias*rotation;
    QQuaternion kwaternion(0.5,0.5,0.5,0.5);
    matrix.rotate(kwaternion.inverted()*(bias*rotation*kwaternion));
    //matrix.rotate(bias);
    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix);

    // Use texture unit 0 which contains cube.png
    program.setUniformValue("texture", 0);

    // Draw cube geometry
    geometries->drawCubeGeometry(&program);
}

void GLWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 3.0, zFar = 7.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}

void GLWidget::updateOrientation(float w, float x, float y, float z)
{
    rotation.setScalar(w);
    rotation.setX(x); rotation.setY(y); rotation.setZ(z);
    update();
}

void GLWidget::setRemoveBiasFalse()
{
    removeBias=false;
}


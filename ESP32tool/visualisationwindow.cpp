#include "visualisationwindow.h"

visualisationWindow::visualisationWindow()
    : m_program(0)
    , m_frame(0)
{
}

//! [3]
static const char *vertexShaderSource =
        "attribute highp vec4 posAttr;\n"
        "attribute lowp vec4 colAttr;\n"
        "varying lowp vec4 col;\n"
        "uniform highp mat4 matrix;\n"
        "void main() {\n"
        "   col = colAttr;\n"
        "   gl_Position = matrix * posAttr;\n"
        "}\n";

static const char *fragmentShaderSource =
        "varying lowp vec4 col;\n"
        "void main() {\n"
        "   gl_FragColor = col;\n"
        "}\n";
//! [3]

//! [4]
void visualisationWindow::initialize()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");

}
//! [4]

//! [5]
void visualisationWindow::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*
    glEnable(GL_DEPTH_TEST); // Depth Testing
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    */
    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective( 60, (double)640 / (double)480, 0.1, 100 );
    matrix.translate(0,0,-5);
    //matrix.rotate(100.0f * m_frame/screen()->refreshRate(), 0, 1, 0);
    matrix.rotate(Qwat);
    m_program->setUniformValue(m_matrixUniform, matrix);




    //MMatrix.perspective( 60, (double)640 / (double)480, 0.1, 100 );
    // MMatrix.translate(0,0,-5);
    // m_program->setUniformValue(m_matrixUniform, MMatrix);
    //glMatrixMode(GL_MODELVIEW_MATRIX);
    // glLoadMatrixf(MMatrix.constData());
    GLfloat vertices[] =
    {
        -1, -1, -1,   -1, -1,  1,   -1,  1,  1,   -1,  1, -1,
        1, -1, -1,    1, -1,  1,    1,  1,  1,    1,  1, -1,
        -1, -1, -1,   -1, -1,  1,    1, -1,  1,    1, -1, -1,
        -1,  1, -1,   -1,  1,  1,    1,  1,  1,    1,  1, -1,
        -1, -1, -1,   -1,  1, -1,    1,  1, -1,    1, -1, -1,
        -1, -1,  1,   -1,  1,  1,    1,  1,  1,    1, -1,  1
    };

    GLfloat vertices1[] =
    {
        0, 1, 0,   -1, -1, 1,   1,  -1,  1, //front face
        0, 1, 0,   1, -1, 1,    1,  -1,  -1, //right face
        0, 1, 0,   1, -1,  -1,    -1, -1,  -1, //back face
        0, 1, 0,   -1,  -1,  -1,    -1,  -1,  1, //left face
    };



    GLfloat colors[] =
    {
        0, 0, 0,   0, 0, 1,   0, 1, 1,   0, 1, 0,
        1, 0, 0,   1, 0, 1,   1, 1, 1,   1, 1, 0,
        0, 0, 0,   0, 0, 1,   1, 0, 1,   1, 0, 0,
        0, 1, 0,   0, 1, 1,   1, 1, 1,   1, 1, 0,
        0, 0, 0,   0, 1, 0,   1, 1, 0,   1, 0, 0,
        0, 0, 1,   0, 1, 1,   1, 1, 1,   1, 0, 1
    };

    GLfloat colors1[] =
    {
        1, 0, 0,   0, 1, 0,  0, 0, 1,
        1, 0, 0,   0, 0, 1,   0, 1, 0,
        1, 0, 0,   0, 1, 0,   0, 0, 1,
        1, 0, 0,   0, 0, 1,   0, 1, 0,
    };



    glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_QUADS, 0, 24);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    m_program->release();

    ++m_frame;
}

void visualisationWindow::newDataIMU(QVector<short> accelData, QVector<short> gyroData, QVector<short> magnetData)
{
    MadAHR.MadgwickAHRSupdate((float)gyroData[0],(float)gyroData[1],(float)gyroData[2],(float)accelData[0],(float)accelData[1],(float)accelData[2],(float)magnetData[0],(float)magnetData[1],(float)magnetData[2]);
    // MadAHR.MadgwickAHRSupdate(gyroData[0],gyroData[1],gyroData[2],accelData[0],accelData[1],accelData[2],magnetData[0],magnetData[1],magnetData[2]);
    //QVector<float> RotationMatrix= MadAHR.toRotMatrix();
    // float pitch=0;
    //float yaw=0;
    //float roll=0;
    Qwat=MadAHR.GetQuat();
    //Qwat.getEulerAngles(pitch,yaw,roll);
    //QVector3D EulerAngles=Qwat.toEulerAngles();
    //qDebug() << "Pitch:" << EulerAngles[0] << " Yaw: "<< EulerAngles[1] << " Roll: " << EulerAngles[2];

    //QMatrix4x4 matrix(RotationMatrix[0],RotationMatrix[1],RotationMatrix[2],RotationMatrix[3],RotationMatrix[4],RotationMatrix[5],RotationMatrix[6],RotationMatrix[7],RotationMatrix[8],RotationMatrix[9],RotationMatrix[10],RotationMatrix[11],RotationMatrix[12],RotationMatrix[13],RotationMatrix[14],RotationMatrix[15]);
    //MMatrix=matrix.transposed();
    //for(int i=0;i<RotationMatrix.size();i++)
    // {
    // float a=RotationMatrix[i];
    //ModelMatrix[i]=RotationMatrix[i];
    //qDebug()<< "Rot:" << i << " : "<< ModelMatrix[i];
    // }
}
//! [5]

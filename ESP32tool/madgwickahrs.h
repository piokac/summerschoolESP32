#ifndef MADGWICKAHRS_H
#define MADGWICKAHRS_H

#include <QVector>
#include <QQuaternion>
#include <QObject>
#include <QMatrix3x3>
#include <QVector3D>


class MadgwickAHRS: public QObject
{
    Q_OBJECT
public:
    explicit MadgwickAHRS(QObject *parent = 0);
    ~MadgwickAHRS();
    void MadgwickAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az);
    void MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
    QVector<float> toRotMatrix();
    float invSqrt(float x);
    QVector<float> getQ();
    QQuaternion GetQuat();
    bool getRemoveBias() const;
    void setRemoveBias(bool value);

    void setValZeroGyro(float xGyroBias,float yGyroBias, float zGyroBias);
    void getValZeroGyro(float *biasTable);
    QMatrix3x3 getU() const;
    void setU(const QMatrix3x3 &value);

    QVector3D getC() const;
    void setC(const QVector3D &value);

private:
    float beta;				// algorithm gain
    float q0, q1, q2, q3;	// quaternion of sensor frame relative to auxiliary frame
    float valZeroGyro[3];
    bool removeBias;
    QMatrix3x3 U;
    QVector3D c;
public slots:
        void setRemoveBiasFalse();
};





#endif // MADGWICKAHRS_H

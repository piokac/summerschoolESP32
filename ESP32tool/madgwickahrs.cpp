#include "madgwickahrs.h"
#include <math.h>
#include <QDebug>


//---------------------------------------------------------------------------------------------------
// Definitions

#define sampleFreq	50.0f		// sample frequency in Hz
#define betaDef		0.1f		// 0.1f było
#define Umatrix  {0.00124435975795164, -0.000085148564413, 0.000006254456535,\
                    0, 0.001276889689738, 0.000032162592671,\
                        0, 0, 0.001282381048053}
#define cvector {-610.764508083726,\
                   -69.3096073507087,\
                    159.164497052233}


//#define sampleFreq	62.5f		// sample frequency in Hz
//#define betaDef		0.041f		// 2 * proportional gain (optimal value)

//---------------------------------------------------------------------------------------------------
// Variable definitions

MadgwickAHRS::MadgwickAHRS(QObject *parent) : QObject(parent)
{
    beta = betaDef;								// 2 * proportional gain (Kp)
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;	// quaternion of sensor frame relative to auxiliary frame
    valZeroGyro[0]=0;
    valZeroGyro[1]=0;
    valZeroGyro[2]=0;
    removeBias=false;
    float UMatrixElements[]=Umatrix;
    QMatrix3x3 tempUMatrix(UMatrixElements);
    U=tempUMatrix;
    QVector3D tempcVector(cvector);
    c=tempcVector;
}

MadgwickAHRS::~MadgwickAHRS()
{

}

float MadgwickAHRS::invSqrt(float x)
{
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i>>1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}

QVector<float> MadgwickAHRS::getQ()
{
    QVector<float> qvec;
    qvec.push_back(q0);
    qvec.push_back(q1);
    qvec.push_back(q2);
    qvec.push_back(q3);
    return qvec;
}

QQuaternion MadgwickAHRS::GetQuat()
{
    //QQuaternion kwat(q0,q1,q2,q3);
    QQuaternion kwat;

    kwat.setScalar(q0);
    //kwat.setX(q2); kwat.setY(-q3); kwat.setZ(-q1);
    kwat.setX(q1);
    kwat.setY(q2);
    kwat.setZ(q3);
    //update();
    return kwat;
}

bool MadgwickAHRS::getRemoveBias() const
{
    return removeBias;
}

void MadgwickAHRS::setRemoveBias(bool value)
{
    removeBias = value;
}

void MadgwickAHRS::setRemoveBiasFalse()
{
    removeBias=false;
}

void MadgwickAHRS::setValZeroGyro(float xGyroBias, float yGyroBias, float zGyroBias)
{
    valZeroGyro[0]=xGyroBias;
    valZeroGyro[1]=yGyroBias;
    valZeroGyro[2]=zGyroBias;
}

void MadgwickAHRS::getValZeroGyro(float *biasTable)
{
    biasTable[0]=valZeroGyro[0];
    biasTable[1]=valZeroGyro[1];
    biasTable[2]=valZeroGyro[2];
}

QMatrix3x3 MadgwickAHRS::getU() const
{
    return U;
}

void MadgwickAHRS::setU(const QMatrix3x3 &value)
{
    U = value;
}

QVector3D MadgwickAHRS::getC() const
{
    return c;
}

void MadgwickAHRS::setC(const QVector3D &value)
{
    c = value;
}

void MadgwickAHRS::MadgwickAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az) {
    float recipNorm;
    float s0, s1, s2, s3;
    float qDot1, qDot2, qDot3, qDot4;
    float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

    // Rate of change of quaternion from gyroscope
    qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
    qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
    qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

        // Normalise accelerometer measurement
        recipNorm = invSqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Auxiliary variables to avoid repeated arithmetic
        _2q0 = 2.0f * q0;
        _2q1 = 2.0f * q1;
        _2q2 = 2.0f * q2;
        _2q3 = 2.0f * q3;
        _4q0 = 4.0f * q0;
        _4q1 = 4.0f * q1;
        _4q2 = 4.0f * q2;
        _8q1 = 8.0f * q1;
        _8q2 = 8.0f * q2;
        q0q0 = q0 * q0;
        q1q1 = q1 * q1;
        q2q2 = q2 * q2;
        q3q3 = q3 * q3;

        // Gradient decent algorithm corrective step
        s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
        s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
        s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
        s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
        recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
        s0 *= recipNorm;
        s1 *= recipNorm;
        s2 *= recipNorm;
        s3 *= recipNorm;

        // Apply feedback step
        qDot1 -= beta * s0;
        qDot2 -= beta * s1;
        qDot3 -= beta * s2;
        qDot4 -= beta * s3;
    }

    // Integrate rate of change of quaternion to yield quaternion
    q0 += qDot1 * (1.0f / sampleFreq);
    q1 += qDot2 * (1.0f / sampleFreq);
    q2 += qDot3 * (1.0f / sampleFreq);
    q3 += qDot4 * (1.0f / sampleFreq);

    // Normalise quaternion
    recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

}



void MadgwickAHRS::MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
    float recipNorm;
    float s0, s1, s2, s3;
    float qDot1, qDot2, qDot3, qDot4;
    float hx, hy;
    float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    // Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
    if((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
        MadgwickAHRSupdateIMU(gx, gy, gz, ax, ay, az);
        return;
    }

    // Rate of change of quaternion from gyroscope
    qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
    qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
    qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

        // Normalise accelerometer measurement
        recipNorm = invSqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Normalise magnetometer measurement
        recipNorm = invSqrt(mx * mx + my * my + mz * mz);
        mx *= recipNorm;
        my *= recipNorm;
        mz *= recipNorm;

        // Auxiliary variables to avoid repeated arithmetic
        _2q0mx = 2.0f * q0 * mx;
        _2q0my = 2.0f * q0 * my;
        _2q0mz = 2.0f * q0 * mz;
        _2q1mx = 2.0f * q1 * mx;
        _2q0 = 2.0f * q0;
        _2q1 = 2.0f * q1;
        _2q2 = 2.0f * q2;
        _2q3 = 2.0f * q3;
        _2q0q2 = 2.0f * q0 * q2;
        _2q2q3 = 2.0f * q2 * q3;
        q0q0 = q0 * q0;
        q0q1 = q0 * q1;
        q0q2 = q0 * q2;
        q0q3 = q0 * q3;
        q1q1 = q1 * q1;
        q1q2 = q1 * q2;
        q1q3 = q1 * q3;
        q2q2 = q2 * q2;
        q2q3 = q2 * q3;
        q3q3 = q3 * q3;

        // Reference direction of Earth's magnetic field
        hx = mx * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx * q1q1 + _2q1 * my * q2 + _2q1 * mz * q3 - mx * q2q2 - mx * q3q3;
        hy = _2q0mx * q3 + my * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my * q1q1 + my * q2q2 + _2q2 * mz * q3 - my * q3q3;
        _2bx = sqrt(hx * hx + hy * hy);
        _2bz = -_2q0mx * q2 + _2q0my * q1 + mz * q0q0 + _2q1mx * q3 - mz * q1q1 + _2q2 * my * q3 - mz * q2q2 + mz * q3q3;
        _4bx = 2.0f * _2bx;
        _4bz = 2.0f * _2bz;

        // Gradient decent algorithm corrective step
        s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay) - _2bz * q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + _2bz * q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q2 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q3 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + (-_4bx * q2 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q0 - _4bz * q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay) + (-_4bx * q3 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q0 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
        s0 *= recipNorm;
        s1 *= recipNorm;
        s2 *= recipNorm;
        s3 *= recipNorm;

        // Apply feedback step
        qDot1 -= beta * s0;
        qDot2 -= beta * s1;
        qDot3 -= beta * s2;
        qDot4 -= beta * s3;
    }

    // Integrate rate of change of quaternion to yield quaternion
    q0 += qDot1 * (1.0f / sampleFreq);
    q1 += qDot2 * (1.0f / sampleFreq);
    q2 += qDot3 * (1.0f / sampleFreq);
    q3 += qDot4 * (1.0f / sampleFreq);

    // Normalise quaternion
    recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

    //qDebug() << q0 << " : " << q1 << " : " << q2 << " : " << q3;
}

QVector<float> MadgwickAHRS::toRotMatrix()
{
    QVector<float> RotMatrix;
    float rot[16];

    rot[0*4+0] = 1*(2*(q1*q2-q0*q3));// zmieniłem z -1 na 1
    //RotMatrix.push_back(rot[0*4+0]);
    rot[0*4+1] = 1*(2*(q1*q3+q0*q2));
   // RotMatrix.push_back(rot[0*4+1]);

    rot[0*4+2] = 1*(q0*q0+q1*q1-q2*q2-q3*q3);//x->>y //2*4+0
    //RotMatrix.push_back(rot[0*4+2]);

    rot[0*4+3] = 0;
   // RotMatrix.push_back(rot[0*4+3]);

    rot[1*4+0] = q0*q0-q1*q1+q2*q2-q3*q3;//y->>z
  //  RotMatrix.push_back(rot[1*4+0]);

    rot[1*4+1] = 2*(q2*q3-q0*q1);
   // RotMatrix.push_back(rot[1*4+1]);

    rot[1*4+2] = 2*(q1*q2+q0*q3);//0*4+0
   // RotMatrix.push_back(rot[1*4+2]);

    rot[1*4+3] = 0;
   // RotMatrix.push_back(rot[1*4+3]);

    rot[2*4+0] = 2*(q2*q3+q0*q1);
   // RotMatrix.push_back(rot[2*4+0]);

    rot[2*4+1] = q0*q0-q1*q1-q2*q2+q3*q3;
   // RotMatrix.push_back(rot[2*4+1]);

    rot[2*4+2] = 2*(q1*q3-q0*q2);//z->>x//1
   // RotMatrix.push_back(rot[2*4+2]);

    rot[2*4+3] = 0;
    //RotMatrix.push_back(rot[2*4+3]);

    rot[3*4+0] = 0;
   // RotMatrix.push_back(rot[3*4+0]);

    rot[3*4+1] = 0;
    //RotMatrix.push_back(rot[3*4+1]);

    rot[3*4+2] = 0;
   // RotMatrix.push_back(rot[3*4+2]);

    rot[3*4+3] = 1;
   // RotMatrix.push_back(rot[3*4+3]);

    RotMatrix.push_back(rot[2]);
    RotMatrix.push_back(rot[6]);
    RotMatrix.push_back(rot[10]);
    RotMatrix.push_back(rot[12]);
    RotMatrix.push_back(rot[0]);
    RotMatrix.push_back(rot[4]);
    RotMatrix.push_back(rot[8]);
    RotMatrix.push_back(rot[13]);
    RotMatrix.push_back(rot[1]);
    RotMatrix.push_back(rot[5]);
    RotMatrix.push_back(rot[9]);
    RotMatrix.push_back(rot[14]);
    RotMatrix.push_back(rot[3]);
    RotMatrix.push_back(rot[7]);
    RotMatrix.push_back(rot[11]);
    RotMatrix.push_back(rot[15]);
    return RotMatrix;
}


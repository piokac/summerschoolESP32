#ifndef VISUALISATIONGL_H
#define VISUALISATIONGL_H

#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>

QT_BEGIN_NAMESPACE
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;
QT_END_NAMESPACE

class visualisationGL : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit visualisationGL(QWindow *parent = 0);
    ~visualisationGL();

    virtual void render(QPainter *painter);
    virtual void render();

    virtual void initialize();

    void setAnimating(bool animating);

public slots:
    void renderLater();
    void renderNow();

protected:
    bool event(QEvent *event) override;

    void exposeEvent(QExposeEvent *event) override;

private:
    bool m_animating;

    QOpenGLContext *m_context;
    QOpenGLPaintDevice *m_device;
};

#endif // VISUALISATIONGL_H

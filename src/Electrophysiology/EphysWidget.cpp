#include "EphysWidget.h"

#include "MEView.h"

#include <QPainter>
#include <QEvent>
#include <QResizeEvent>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace mv;

EphysWidget::EphysWidget(EphysView* plugin, Scene* scene) :
    _scene(scene)
    //_traceRenderer(scene)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);

    installEventFilter(this);

    QSurfaceFormat surfaceFormat;

    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);

    // Ask for an OpenGL 3.3 Core Context as the default
    surfaceFormat.setVersion(3, 3);
    surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    surfaceFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    surfaceFormat.setSamples(8);

    setFormat(surfaceFormat);
}

EphysWidget::~EphysWidget()
{

}

void EphysWidget::onWidgetInitialized()
{
    initializeOpenGLFunctions();

    //_traceRenderer.init();

    // Start timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(1000.0f / 60);

    isInitialized = true;
}

void EphysWidget::onWidgetResized(int w, int h)
{
    //_traceRenderer.resize(w, h);
}

void EphysWidget::onWidgetRendered()
{
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    //_traceRenderer.render();

    QPainter painter(this);

    painter.end();
}

void EphysWidget::onWidgetCleanup()
{

}

bool EphysWidget::eventFilter(QObject* target, QEvent* event)
{
    auto shouldPaint = false;

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        qDebug() << "Mouse click";

        //makeCurrent();
        //_traceRenderer.reloadShaders();

        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(target, event);
}

#include "MEWidget.h"

MEWidget::MEWidget() :
    _emRenderer()
{
    connect(&_emRenderer, &EMRenderer::requestNewAspectRatio, this, &MEWidget::onNewAspectRatioRequested);
}

void MEWidget::setCells(const std::vector<Cell>& cells)
{
    if (!isWidgetInitialized())
        return;

    _cells = cells;

    makeCurrent();
    _emRenderer.buildRenderObjects(cells);
}

void MEWidget::showAxons(bool enabled)
{
    _emRenderer.showAxons(enabled);
}

void MEWidget::onWidgetInitialized()
{
    _emRenderer.init();

    // Start 50 fps render timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(1000.0f / 50);
}

void MEWidget::onWidgetResized(int w, int h)
{
    qDebug() << "Widget resize";
    _width = w; _height = h;
    _emRenderer.resize(w, h);
}

void MEWidget::onWidgetRendered()
{
    // Increment time
    t += 0.3f;

    _emRenderer.update(t);
}

void MEWidget::onWidgetCleanup()
{
}

void MEWidget::onNewAspectRatioRequested(float aspectRatio)
{
    // Should be set in pre-scaled coordinates, because 32 pixels here results in 40px at 125%
    int newWidth = aspectRatio* (_height / devicePixelRatioF());
    qDebug() << "Requested new width: " << newWidth;
    setFixedWidth(newWidth);
}

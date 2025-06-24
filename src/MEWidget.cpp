#include "MEWidget.h"

MEWidget::MEWidget() :
    _lineRenderer()
{

}

void MEWidget::setCells(const std::vector<Cell>& cells)
{
    if (!isWidgetInitialized())
        return;

    _cells = cells;

    makeCurrent();
    _lineRenderer.buildRenderObjects(cells);
}

void MEWidget::onWidgetInitialized()
{
    _lineRenderer.init();

    // Start 50 fps render timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(1000.0f / 50);
}

void MEWidget::onWidgetResized(int w, int h)
{
    _lineRenderer.resize(w, h, 0, 0);
}

void MEWidget::onWidgetRendered()
{
    // Increment time
    t += 0.3f;

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    _lineRenderer.render(t);
}

void MEWidget::onWidgetCleanup()
{
}

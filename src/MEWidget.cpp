#include "MEWidget.h"

#include <QSizePolicy>

MEWidget::MEWidget() :
    _emRenderer(),
    _layerDrawing(this),
    _width(1),
    _height(1),
    _isCortical(false)
{
    connect(&_emRenderer, &EMRenderer::requestNewAspectRatio, this, &MEWidget::onNewAspectRatioRequested);

    //setMinimumSize(10, 10);
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
}

void MEWidget::setCells(const std::vector<Cell>& cells)
{
    if (!isWidgetInitialized())
        return;

    makeCurrent();
    _emRenderer.BuildRenderObjects(cells);
}

void MEWidget::setSelectedCells(const std::vector<Cell>& cells)
{
    //if (!isWidgetInitialized()) // Shouldn't be necessary
    //    return;

    _cells = cells;

    // makeCurrent(); // Shouldn't be necessary
    _emRenderer.SetSelectedCellIds(cells);
}

void MEWidget::SetCortical(bool isCortical)
{
    _isCortical = isCortical;

    _emRenderer.SetCortical(isCortical);
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

    QPainter painter(this);

    painter.beginNativePainting();
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    painter.endNativePainting();

    Scene& scene = Scene::getInstance();
    qDebug() << "Height: " << height();
    _layerDrawing.setDepthRange(scene.getCortexStructure().getMinDepth(), scene.getCortexStructure().getMaxDepth());
    _layerDrawing.drawAxes(painter);

    painter.beginNativePainting();
    _emRenderer.update(t);
    painter.endNativePainting();

    painter.end();
}

void MEWidget::onWidgetCleanup()
{
}

void MEWidget::onNewAspectRatioRequested(float aspectRatio)
{
    // Should be set in pre-scaled coordinates, because 32 pixels here results in 40px at 125%
    int newWidth = aspectRatio* (_height / devicePixelRatioF());
    qDebug() << "Requested new width: " << newWidth;
    setMinimumWidth(newWidth);
}

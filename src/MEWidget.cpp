#include "MEWidget.h"

#include <QSizePolicy>

namespace
{
    QFont textFont("Segoe UI", 9, 400, false);
}

MEWidget::MEWidget() :
    _scene(Scene::getInstance()),
    _meRenderer(),
    _layerDrawing(this),
    _width(1),
    _height(1),
    _isCortical(false)
{
    connect(&_meRenderer, &MERenderer::RequestNewAspectRatio, this, &MEWidget::onNewAspectRatioRequested);

    //setMinimumSize(10, 10);
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    setMouseTracking(true);
    cellCard = new CellCard();
    popup = new RoundedPopup();
    popup->SetWidget(cellCard);
}

void MEWidget::setCells(const std::vector<Cell>& cells)
{
    if (!isWidgetInitialized())
        return;

    makeCurrent();
    _meRenderer.BuildRenderObjects(cells);
}

void MEWidget::setSelectedCells(const std::vector<uint32_t>& indices)
{
    _scene.selectedCells.clear();
    for (uint32_t cellIndex : indices)
    {
        _scene.selectedCells.push_back(_scene.allCells[cellIndex]);
    }

    _meRenderer.SetSelectedCellIds(indices);
}

void MEWidget::SetCortical(bool isCortical)
{
    _isCortical = isCortical;

    _meRenderer.SetCortical(isCortical);
}

void MEWidget::onWidgetInitialized()
{
    _meRenderer.Init();

    // Start 40 fps render timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(1000.0f / 40);
}

void MEWidget::onWidgetResized(int w, int h)
{
    _width = w; _height = h;
    _meRenderer.Resize(w, h, devicePixelRatioF());
}

void MEWidget::onWidgetRendered()
{
    // Increment time
    t += 0.3f;

    QPainter painter(this);
#ifdef _WIN32
    painter.setFont(textFont);
#endif
    painter.beginNativePainting();
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    painter.endNativePainting();

    Scene& scene = Scene::getInstance();
    
    _layerDrawing.setDepthRange(scene.getCortexStructure().getMinDepth(), scene.getCortexStructure().getMaxDepth());
    _layerDrawing.drawAxes(painter, _isCortical);
    //_layerDrawing.drawSeparations(painter, _isCortical);

    painter.beginNativePainting();
    _meRenderer.Update(t, painter);
    painter.endNativePainting();

    _meRenderer.RenderLabels(painter);
    _meRenderer.RenderSeparations(painter);

    painter.end();
}

void MEWidget::onWidgetCleanup()
{
}

void MEWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPoint localPos = event->pos(); // position inside the widget
        QPoint globalPos = mapToGlobal(localPos);

        std::vector<float> cellLocations = _meRenderer.GetHorizontalCellLocations();
        Cell* cell = nullptr;
        float closestDist = std::numeric_limits<float>::max();
        for (int i = 0; i < cellLocations.size(); i++)
        {
            int xCoord = cellLocations[i] / devicePixelRatioF(); // Non-dpr coord
            float dist = abs(xCoord - localPos.x());
            //qDebug() << "X:" << xCoord << "Mx: " << localPos.x();
            if (dist < closestDist)
            {
                closestDist = dist;
                cell = &_scene.selectedCells[i];
            }
        }

        if (cell != nullptr)
        {
            popup->move(globalPos + QPoint(10, -400));
            popup->show();

            cellCard->SetCell(*cell);
        }
    }
    QWidget::mousePressEvent(event);
}

void MEWidget::mouseMoveEvent(QMouseEvent* event)
{
    //popup->hide();
    QWidget::mouseMoveEvent(event);
}

void MEWidget::onNewAspectRatioRequested(float aspectRatio)
{
    // Should be set in pre-scaled coordinates, because 32 pixels here results in 40px at 125%
    int newWidth = aspectRatio * (_height / devicePixelRatioF());

    setMinimumWidth(newWidth);
}

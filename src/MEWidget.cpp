#include "MEWidget.h"

#include <QSizePolicy>

MEWidget::MEWidget() :
    _scene(Scene::getInstance()),
    _emRenderer(),
    _layerDrawing(this),
    _width(1),
    _height(1),
    _isCortical(false)
{
    connect(&_emRenderer, &EMRenderer::requestNewAspectRatio, this, &MEWidget::onNewAspectRatioRequested);

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
    _emRenderer.BuildRenderObjects(cells);
}

void MEWidget::setSelectedCells(const std::vector<uint32_t>& indices)
{
    //if (!isWidgetInitialized()) // Shouldn't be necessary
    //    return;

    //_cells = cells;

    _scene.selectedCells.clear();
    for (uint32_t cellIndex : indices)
    {
        _scene.selectedCells.push_back(_scene.allCells[cellIndex]);
    }

    // makeCurrent(); // Shouldn't be necessary
    _emRenderer.SetSelectedCellIds(indices);
}

void MEWidget::SetCortical(bool isCortical)
{
    _isCortical = isCortical;

    _emRenderer.SetCortical(isCortical);
}

void MEWidget::onWidgetInitialized()
{
    _emRenderer.init();

    // Start 40 fps render timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(1000.0f / 40);
}

void MEWidget::onWidgetResized(int w, int h)
{
    qDebug() << "Widget resize";
    _width = w; _height = h;
    _emRenderer.resize(w, h, devicePixelRatioF());
}

void MEWidget::onWidgetRendered()
{
    // Increment time
    t += 0.3f;

    QPainter painter(this);

    painter.beginNativePainting();
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    painter.endNativePainting();

    Scene& scene = Scene::getInstance();
    
    _layerDrawing.setDepthRange(scene.getCortexStructure().getMinDepth(), scene.getCortexStructure().getMaxDepth());
    _layerDrawing.drawAxes(painter, _isCortical);

    painter.beginNativePainting();
    _emRenderer.update(t);
    painter.endNativePainting();

    std::vector<float> horizontalCellLocations = _emRenderer.GetHorizontalCellLocations();
    for (int i = 0; i < horizontalCellLocations.size(); i++)
    {
        int xCoord = horizontalCellLocations[i] / devicePixelRatioF();
        int yCoord = 16;

        QFontMetrics fm(painter.font());
        int textWidth = fm.horizontalAdvance(_scene.selectedCells[i].cluster);
        int textHeight = fm.height();

        // Calculate top-left corner to draw the text so that it is centered
        int x = xCoord - textWidth / 2;
        int y = yCoord + fm.ascent() - textHeight / 2;

        painter.drawText(x, y, _scene.selectedCells[i].cluster);
    }

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

        std::vector<float> cellLocations = _emRenderer.GetHorizontalCellLocations();
        Cell* cell = nullptr;
        float closestDist = std::numeric_limits<float>::max();
        for (int i = 0; i < cellLocations.size(); i++)
        {
            int xCoord = cellLocations[i];// / devicePixelRatioF(); // Non-dpr coord
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
            popup->move(globalPos + QPoint(10, -200));
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
    qDebug() << "Requested new width: " << newWidth;
    setMinimumWidth(newWidth);
}

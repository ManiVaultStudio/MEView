#include "MorphologyWidget.h"

#include "MEView.h"

#include "CellMorphologyData/CellMorphology.h"

#include <QPainter>
#include <QEvent>
#include <QResizeEvent>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace mv;

int M_MARGIN = 48;

MorphologyWidget::MorphologyWidget(MEView* plugin, Scene* scene) :
    _renderMode(RenderMode::LINE),
    _scene(scene),
    _lineRenderer(scene),
    _tubeRenderer(scene),
    _layerDrawing(this)
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
    surfaceFormat.setSamples(64);

    setFormat(surfaceFormat);

    setContentsMargins(0, 0, 0, 0);

    setMinimumHeight(300);
    setFixedHeight(400);
}

MorphologyWidget::~MorphologyWidget()
{

}

void MorphologyWidget::setRowWidth(float rowWidth)
{
    _lineRenderer.setRowWidth(rowWidth);
}

void MorphologyWidget::uploadMorphologies()
{
    if (!isInitialized)
        return;

    makeCurrent();
    _lineRenderer.buildRenderObjects();
}

void MorphologyWidget::onWidgetInitialized()
{
    _lineRenderer.init();
    _tubeRenderer.init();

    // Start timer
    QTimer* updateTimer = new QTimer();
    QObject::connect(updateTimer, &QTimer::timeout, this, [this]() { update(); });
    updateTimer->start(1000.0f / 60);

    isInitialized = true;
}

void MorphologyWidget::onWidgetResized(int w, int h)
{
    float px = devicePixelRatio();
    _lineRenderer.resize(w, h, M_MARGIN * px, M_MARGIN * px);
    _tubeRenderer.resize(w, h, M_MARGIN * px, M_MARGIN * px);
}

void MorphologyWidget::onWidgetRendered()
{
    t += 0.3f;

    int numObjects = _lineRenderer.getNumRenderObjects();

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (numObjects < 1)
        return;

    std::vector<float> offset;
    _lineRenderer.getCellMetadataLocations(offset);

    QPainter painter(this);

    int chartWidth = width() - M_MARGIN * 2;

    _layerDrawing.setDepthRange(_scene->getCortexStructure().getMinDepth(), _scene->getCortexStructure().getMaxDepth());
    _layerDrawing.drawAxes(painter, _scene);

    mv::Dataset<CellMorphologies> morphologyDataset = _scene->getMorphologyDataset();
    const std::vector<CellMorphology>& morphologies = morphologyDataset->getData();
    mv::Dataset<Text> cellMetadata = _scene->getCellMetadataDataset();

    const auto& selectionIndices = morphologyDataset->getSelectionIndices();
    std::vector<uint32_t> sortedSelectionIndices = selectionIndices;

    // Reorder selection based on soma depth
    std::sort(sortedSelectionIndices.begin(), sortedSelectionIndices.end(), [&morphologies](const uint32_t& a, const uint32_t& b)
    {
        return morphologies[a].somaPosition.y > morphologies[b].somaPosition.y;
    });

    QStringList morphCellIds = morphologyDataset->getCellIdentifiers();
    std::vector<QString> cellIds = cellMetadata->getColumn("Cell ID");

    if (cellMetadata->hasColumn("Cluster"))
    {
        std::vector<QString> clusters = cellMetadata->getColumn("Cluster");

        QFont font = painter.font();
        //font.setPointSizeF(font.pointSizeF() * 2);
        painter.setFont(font);

        painter.setPen(QPen(Qt::black, 1));
        for (int i = 0; i < sortedSelectionIndices.size(); i++)
        {
            int si = sortedSelectionIndices[i];
            CellMorphology& morphology = morphologyDataset->getData()[si];

            QString morphCellId = morphCellIds[si];
            for (int ci = 0; ci < cellIds.size(); ci++)
            {
                if (cellIds[ci] == morphCellId)
                {
                    painter.drawText(offset[i] * chartWidth - 42 + M_MARGIN, 16, "Cell ID: " + morphCellId);
                    painter.drawText(offset[i] * chartWidth - 16 + M_MARGIN, 32, clusters[ci]);
                }
            }
        }
    }
    
    painter.beginNativePainting();

    switch (_renderMode)
    {
    case RenderMode::LINE: _lineRenderer.render(0, t); break;
    case RenderMode::REAL: _tubeRenderer.render(0, t); break;
    default: _lineRenderer.render(0, t);
    }

    painter.endNativePainting();

    painter.end();
}

void MorphologyWidget::onWidgetCleanup()
{

}

bool MorphologyWidget::eventFilter(QObject* target, QEvent* event)
{
    auto shouldPaint = false;

    switch (event->type())
    {
    case QEvent::Resize:
    {
        const auto resizeEvent = static_cast<QResizeEvent*>(event);

        break;
    }

    case QEvent::MouseButtonPress:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        qDebug() << "Mouse click";

        makeCurrent();
        _tubeRenderer.reloadShaders();

        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(target, event);
}

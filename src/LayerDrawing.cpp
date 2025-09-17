#include "LayerDrawing.h"

#include "Scene.h"

int MARGIN = 48;
//int TOP_MARGIN = 16;
//int BOTTOM_MARGIN = 128;

LayerDrawing::LayerDrawing(QWidget* parent) :
    _parent(parent),
    _scene(Scene::getInstance()),
    _minDepth(0),
    _maxDepth(1),
    _depthRange(1)
{

}

void LayerDrawing::setDepthRange(float minDepth, float maxDepth)
{
    _minDepth = minDepth;
    _maxDepth = maxDepth;
    _depthRange = maxDepth - minDepth;
}

void LayerDrawing::drawAxes(QPainter& painter, bool isCortical)
{
    int topMargin = 32; // Non-pixel ratio margin
    int bottomMargin = _parent->height() / 3.0f; // Pixel ratio margin

    int chartWidth = _parent->width() - MARGIN * 2;
    int chartHeight = _parent->height() - topMargin - bottomMargin;

    const CortexStructure& cortexStructure = _scene.getCortexStructure();

    QPen axisPen(QColor(80, 80, 80, 255), 2, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    QPen midPen(QColor(80, 80, 80, 80), 1, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    midPen.setCosmetic(true);

    //int lightness = 240;
    std::vector<int> lineHeights(cortexStructure._layerDepths.size() + 1);

    QFont originalFont = painter.font();
    QFont layerFont = originalFont;
    layerFont.setPointSizeF(layerFont.pointSizeF() * 1.5f);
    layerFont.setBold(true);
    painter.setFont(layerFont);
    
    for (int i = 0; i < cortexStructure._layerDepths.size(); i++)
    {
        if (!isCortical && (i != 0 && i != cortexStructure._layerDepths.size() - 1))
            continue;
        //qDebug() << "cortical layer: " << cortexStructure._layerDepths.size();
        float layerDepth = cortexStructure.getLayerDepth(i);

        int lineY = (layerDepth / _depthRange) * chartHeight + topMargin;

        if (i == 0 || i == cortexStructure._layerDepths.size() - 1)
            painter.setPen(axisPen);
        else
            painter.setPen(midPen);

        drawHorizontalLine(painter, lineY);

        if (i != cortexStructure._layerDepths.size() - 1)
        {
            int bottomY = (cortexStructure.getLayerDepth(i+1) / _depthRange) * chartHeight + topMargin;
            int midPoint = (bottomY + lineY) / 2;

            if (isCortical)
            {
                painter.setPen(axisPen);
                painter.drawText(MARGIN - 28, midPoint + 8, "L" + QString::number(i + 1));
            }
        }

        //painter.fillRect(MARGIN, topY, chartWidth, abs(topY - bottomY), QColor::fromHsl(0, 0, lightness));
        //lightness -= 2;
    }

    painter.setFont(originalFont);
    // Vertical axis
    painter.setPen(axisPen);
    painter.drawLine(MARGIN, topMargin, MARGIN, _parent->height() - bottomMargin);
}

static qreal snapToDeviceRow(const QPainter& p, qreal y)
{
    const QTransform dev = p.deviceTransform();
    const qreal devY = dev.map(QPointF(0, y)).y();        // logical -> device
    const qreal snappedDevY = std::floor(devY) + 0.5;     // center of device pixel row
    const QTransform inv = dev.inverted();
    return inv.map(QPointF(0, snappedDevY)).y();          // device -> logical
}

void LayerDrawing::drawHorizontalLine(QPainter& painter, float y)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Draw line centered at a pixel, so it doesn't bleed onto multiple pixels
    const qreal py = snapToDeviceRow(painter, y);
    painter.drawLine(QPointF(MARGIN, py), QPointF(_parent->width() - MARGIN, py));

    painter.restore();
}

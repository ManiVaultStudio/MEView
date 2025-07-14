#include "LayerDrawing.h"

#include "Scene.h"

int MARGIN = 48;
int TOP_MARGIN = 16;
int BOTTOM_MARGIN = 128;

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

void LayerDrawing::drawAxes(QPainter& painter)
{
    int chartWidth = _parent->width() - MARGIN * 2;
    int chartHeight = _parent->height() - TOP_MARGIN - BOTTOM_MARGIN;

    const CortexStructure& cortexStructure = _scene.getCortexStructure();

    QPen axisPen(QColor(80, 80, 80, 255), 2, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);
    QPen midPen(QColor(80, 80, 80, 255), 1, Qt::DashLine, Qt::FlatCap, Qt::RoundJoin);

    //int lightness = 240;
    std::vector<int> lineHeights(cortexStructure._layerDepths.size() + 1);

    QFont originalFont = painter.font();
    QFont layerFont = originalFont;
    layerFont.setPointSizeF(layerFont.pointSizeF() * 1.5f);
    layerFont.setBold(true);
    painter.setFont(layerFont);

    for (int i = 0; i < cortexStructure._layerDepths.size(); i++)
    {
        float layerDepth = cortexStructure.getLayerDepth(i);

        int lineY = (layerDepth / _depthRange) * chartHeight + TOP_MARGIN;

        if (i == 0 || i == cortexStructure._layerDepths.size() - 1)
            painter.setPen(axisPen);
        else
            painter.setPen(midPen);

        drawHorizontalLine(painter, lineY);

        if (i != cortexStructure._layerDepths.size() - 1)
        {
            int bottomY = (cortexStructure.getLayerDepth(i+1) / _depthRange) * chartHeight + TOP_MARGIN;
            int midPoint = (bottomY + lineY) / 2;

            painter.drawText(MARGIN - 28, midPoint + 8, "L" + QString::number(i + 1));
        }

        //painter.fillRect(MARGIN, topY, chartWidth, abs(topY - bottomY), QColor::fromHsl(0, 0, lightness));
        //lightness -= 2;
    }

    painter.setFont(originalFont);
    // Vertical axis
    painter.setPen(axisPen);
    painter.drawLine(MARGIN, TOP_MARGIN, MARGIN, _parent->height() - BOTTOM_MARGIN);
}

void LayerDrawing::drawHorizontalLine(QPainter& painter, float y)
{
    painter.drawLine(MARGIN, y, _parent->width() - MARGIN, y);
}

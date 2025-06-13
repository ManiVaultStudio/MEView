#pragma once

#include <QWidget>
#include <QPainter>

class Scene;

class LayerDrawing
{
public:
    LayerDrawing(QWidget* parent);

    void setDepthRange(float minDepth, float maxDepth);

    void drawAxes(QPainter& painter, Scene* scene);

private:
    void drawHorizontalLine(QPainter& painter, float y);

private:
    QWidget* _parent;

    float _minDepth;
    float _maxDepth;
    float _depthRange;
};

#pragma once

#include "Scene.h"

#include <QWidget>
#include <QPainter>

class Scene;

class LayerDrawing
{
public:
    LayerDrawing(QWidget* parent);

    void setDepthRange(float minDepth, float maxDepth);

    void drawAxes(QPainter& painter, bool isCortical);

private:
    void drawHorizontalLine(QPainter& painter, float y);

private:
    QWidget* _parent;

    Scene& _scene;

    float _minDepth;
    float _maxDepth;
    float _depthRange;
};

#pragma once

#include "graphics/Vector3f.h"

#include <QMatrix4x4>

class RenderContext
{
public:
    QMatrix4x4 modelMatrix;

    std::vector<float> horizontalCellLocations;
    std::vector<float> xCoords;

    std::vector<mv::Vector3f> somaPositions;
};

#pragma once

#include "Rendering/CellRenderObject.h"

#include <QString>
#include <QHash>

class RenderState
{
public:
    QHash<QString, CellRenderObject>    _cellRenderObjects;
    std::vector<Cell>                   _selectedCells;

    float _stimChartRangeMin = -1;
    float _stimChartRangeMax = 1;

    float _acqChartRangeMin = -1;
    float _acqChartRangeMax = 1;
};

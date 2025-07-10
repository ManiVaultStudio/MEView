#pragma once

#include "Scene.h"

#include "Rendering/CellRenderObject.h"

#include <QOpenGLFunctions_3_3_Core>

class RenderState;
class Recording;

class RenderObjectBuilder
{
public:
    RenderObjectBuilder(QOpenGLFunctions_3_3_Core* f, RenderState* renderState);

public:
    void BuildCellRenderObjects(const std::vector<Cell>& cells);
    void BuildCellRenderObject(CellRenderObject& cro, const Cell& cell);

    void BuildMorphologyObject(MorphologyRenderObject& mro, const CellMorphology& cellMorpology);
    void BuildTraceObject(TraceRenderObject& tro, const Recording& recording, bool isStim);

private:
    QOpenGLFunctions_3_3_Core*  _f;             // Non-owning raw pointer
    RenderState*                _renderState;   // Non-owning raw pointer
};

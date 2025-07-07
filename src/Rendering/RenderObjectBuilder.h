#pragma once

#include "Scene.h"
#include "Rendering/RenderState.h"
#include "Rendering/CellRenderObject.h"

#include <QOpenGLFunctions_3_3_Core>

class RenderObjectBuilder
{
public:
    RenderObjectBuilder(QOpenGLFunctions_3_3_Core* f, RenderState* renderState);

public:
    void BuildCellRenderObject(CellRenderObject& out, Cell& cell);
    void BuildMorphologyObject(MorphologyRenderObject& mro, const CellMorphology& cellMorpology, bool showAxons);

private:
    QOpenGLFunctions_3_3_Core*  _f;             // Non-owning raw pointer
    RenderState*                _renderState;   // Non-owning raw pointer
};

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
    RenderObjectBuilder(QOpenGLFunctions_3_3_Core* f);

public:
    GLuint BuildCellSoma();
    void BuildCellRenderObjects(const std::vector<Cell>& cells, QHash<QString, CellRenderObject>& cellRenderObjects);
    void BuildCellRenderObject(CellRenderObject& cro, const Cell& cell);

    void BuildMorphologyObject(MorphologyRenderObject& mro, const CellMorphology& cellMorpology);
    void BuildTraceObject(TraceRenderObject& tro, const Recording& recording, StimulusType stimType, bool isStim);
    void BuildStimulusObject(TraceRenderObject& tro, const Stimulus& stimulus);

private:
    QOpenGLFunctions_3_3_Core*  _f;             // Non-owning raw pointer
    RenderState*                _renderState;   // Non-owning raw pointer
};

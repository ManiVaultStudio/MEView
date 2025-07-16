#include "CellRenderObject.h"

void CellRenderObject::Cleanup(QOpenGLFunctions_3_3_Core* f)
{
    //f->glDeleteBuffers(1, &morphologyObject.vbo);
    //f->glDeleteBuffers(1, &morphologyObject.rbo);
    //f->glDeleteBuffers(1, &morphologyObject.tbo);
    //f->glDeleteVertexArrays(1, &morphologyObject.vao);

    //f->glDeleteBuffers(1, &stimulusObject.vbo);
    //f->glDeleteBuffers(1, &acquisitionObject.vbo);

    //f->glDeleteVertexArrays(1, &stimulusObject.vao);
    //f->glDeleteVertexArrays(1, &acquisitionObject.vao);
}

void MorphologyRenderObject::ComputeExtents(std::vector<CellMorphology::Type> ignoredTypes)
{
    totalExtent.emin = mv::Vector3f(std::numeric_limits<float>::max());
    totalExtent.emax = mv::Vector3f(-std::numeric_limits<float>::max());

    for (auto it = processes.constBegin(); it != processes.constEnd(); ++it)
    {
        if (std::find(ignoredTypes.begin(), ignoredTypes.end(), it.key()) != ignoredTypes.end())
            continue;

        totalExtent.Extend(it.value().extents);
    }
}

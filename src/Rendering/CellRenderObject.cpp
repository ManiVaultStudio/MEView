#include "CellRenderObject.h"

void CellRenderObject::Cleanup(QOpenGLFunctions_3_3_Core* f)
{
    f->glDeleteBuffers(1, &morphologyObject.vbo);
    f->glDeleteBuffers(1, &morphologyObject.rbo);
    f->glDeleteBuffers(1, &morphologyObject.tbo);
    f->glDeleteVertexArrays(1, &morphologyObject.vao);

    f->glDeleteBuffers(1, &stimulusObject.vbo);
    f->glDeleteBuffers(1, &acquisitionObject.vbo);

    f->glDeleteVertexArrays(1, &stimulusObject.vao);
    f->glDeleteVertexArrays(1, &acquisitionObject.vao);
}

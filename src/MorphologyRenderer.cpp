#include "MorphologyRenderer.h"

#include "CellMorphologyData/CellMorphology.h"

void MorphologyRenderer::resize(int w, int h, int xMargin, int yMargin)
{
    glViewport(xMargin, yMargin, w - xMargin, h - yMargin);

    vx = xMargin; vy = yMargin; vw = w - xMargin * 2; vh = h - yMargin * 2;

    int width = w - xMargin * 2;
    int height = h - yMargin * 2;

    _aspectRatio = (float)width / height;

    _projMatrix.setToIdentity();
    _projMatrix.ortho(0, _aspectRatio, 0, 1, -1, 1);
}

void MorphologyRenderer::update(float t)
{
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    
}

void MorphologyRenderer::buildRenderObjects(const std::vector<Cell>& cells)
{
    // Delete previous render objects
    for (CellRenderObject& cellRenderObject : _cellRenderObjects)
    {
        glDeleteBuffers(1, &cellRenderObject.vbo);
        glDeleteBuffers(1, &cellRenderObject.rbo);
        glDeleteBuffers(1, &cellRenderObject.tbo);
        glDeleteVertexArrays(1, &cellRenderObject.vao);
    }

    _cellRenderObjects.clear();

    _cellRenderObjects.resize(cells.size());
    qDebug() << "Build render objects: " << cells.size();
    for (int i = 0; i < cells.size(); i++)
    {
        const CellMorphology& morphology = *cells[i].morphology;

        buildRenderObject(morphology, _cellRenderObjects[i]);
    }

    /////////////////////////////////////////////
    // Region debug
    glGenVertexArrays(1, &quadVao);
}

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

void MorphologyRenderer::buildRenderObjects()
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

    mv::Dataset<CellMorphologies> morphologyDataset = _scene->getMorphologyDataset();

    const std::vector<CellMorphology>& morphologies = morphologyDataset->getData();
    const auto& selectionIndices = morphologyDataset->getSelectionIndices();
    std::vector<uint32_t> sortedSelectionIndices = selectionIndices;

    // Reorder selection based on soma depth
    std::sort(sortedSelectionIndices.begin(), sortedSelectionIndices.end(), [&morphologies](const uint32_t& a, const uint32_t& b)
    {
        return morphologies[a].somaPosition.y > morphologies[b].somaPosition.y;
    });

    _cellRenderObjects.resize(sortedSelectionIndices.size());
    qDebug() << "Build render objects: " << sortedSelectionIndices.size();
    for (int i = 0; i < sortedSelectionIndices.size(); i++)
    {
        int si = sortedSelectionIndices[i];
        const CellMorphology& morphology = morphologies[si];

        buildRenderObject(morphology, _cellRenderObjects[i]);
    }

    /////////////////////////////////////////////
    // Region debug
    glGenVertexArrays(1, &quadVao);
}

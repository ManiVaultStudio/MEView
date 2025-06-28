#include "EMRenderer.h"

#include <cmath>

namespace
{
    float computeWidthExtent(CellMorphology& cm)
    {
        float minDiffX = fabs(cm.minRange.x - cm.somaPosition.x);
        float maxDiffX = fabs(cm.maxRange.x - cm.somaPosition.x);
        float minDiffY = fabs(cm.minRange.y - cm.somaPosition.y);
        float maxDiffY = fabs(cm.maxRange.y - cm.somaPosition.y);

        return std::max(std::max(minDiffX, maxDiffX), std::max(minDiffY, maxDiffY));
    }
}

void EMRenderer::init()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    loaded &= _lineShader.loadShaderFromFile(":me_viewer/shaders/PassThrough.vert", ":me_viewer/shaders/Lines.frag");
    loaded &= _traceShader.loadShaderFromFile(":me_viewer/shaders/Trace.vert", ":me_viewer/shaders/Trace.frag");
    //loaded &= _quadShader.loadShaderFromFile(":me_viewer/shaders/Quad.vert", ":me_viewer/shaders/Quad.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }

    glEnable(GL_LINE_SMOOTH);
    glLineWidth(5);
}

void EMRenderer::resize(int w, int h)
{
    float aspectRatio = (float)w / h;
    qDebug() << "Resize called";
    _projMatrix.setToIdentity();
    //_projMatrix.ortho(0, aspectRatio, 0, 1, -1, 1);

    _projMatrix.ortho(0, aspectRatio, 0, 1, 1, -1);

    vx = 0; vy = 0; vw = w; vh = h;
}

void EMRenderer::update(float t)
{
    glViewport(vx, vy, vw, vh);

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    float maxScale = 0.01f;
    for (int i = 0; i < _cellRenderObjects.size(); i++)
    {
        if (_cellRenderObjects[i].maxExtent > maxScale)
            maxScale = _cellRenderObjects[i].maxExtent;
    }
    maxScale *= 1.8f;

    _lineShader.bind();

    _viewMatrix.setToIdentity();
    _viewMatrix.scale(1.0f / maxScale);

    float xOffset = 0;
    //qDebug() << "Rendering " << _cellRenderObjects.size() << " objects.";
    for (int i = 0; i < _cellRenderObjects.size(); i++)
    {
        CellRenderObject& cellRenderObject = _cellRenderObjects[i];

        mv::Vector3f anchorPoint = cellRenderObject.anchorPoint;

        float maxWidth = sqrtf(powf(cellRenderObject.ranges.x, 2) + powf(cellRenderObject.ranges.z, 2)) * 1.2f;

        //qDebug() << "YOffset" << yOffset;
        _modelMatrix.setToIdentity();
        _modelMatrix.translate(xOffset + maxWidth / 2, maxScale/2, 0);
        _modelMatrix.rotate(t, 0, 1, 0);
        _modelMatrix.translate(-anchorPoint.x, -anchorPoint.y, -anchorPoint.z);
        //qDebug() << cellRenderObject.somaPosition.x << cellRenderObject.somaPosition.y << cellRenderObject.somaPosition.z;
        //qDebug() << cellRenderObject.maxExtent;

        _lineShader.uniformMatrix4f("projMatrix", _projMatrix.constData());
        _lineShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());
        _lineShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

        _lineShader.uniform3f("cellTypeColor", cellRenderObject.cellTypeColor);

        glBindVertexArray(cellRenderObject.vao);
        glDrawArrays(GL_LINES, 0, cellRenderObject.numVertices);
        glBindVertexArray(0);

        xOffset += maxWidth;
    }

    _lineShader.release();

    _traceShader.bind();

    xOffset = 0;
    for (int i = 0; i < _cellRenderObjects.size(); i++)
    {
        CellRenderObject& cellRenderObject = _cellRenderObjects[i];

        float maxWidth = sqrtf(powf(cellRenderObject.ranges.x, 2) + powf(cellRenderObject.ranges.z, 2)) * 1.2f;

        //qDebug() << "YOffset" << yOffset;
        _modelMatrix.setToIdentity();
        _modelMatrix.translate(xOffset + maxWidth / 2 - maxScale * 0.05f, maxScale * 0.1f, 0);
        _modelMatrix.scale(maxScale * 0.1f);
        //_modelMatrix.scale(1.0f / 100);
        //qDebug() << cellRenderObject.somaPosition.x << cellRenderObject.somaPosition.y << cellRenderObject.somaPosition.z;
        //qDebug() << cellRenderObject.maxExtent;

        _traceShader.uniformMatrix4f("projMatrix", _projMatrix.constData());
        _traceShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());
        _traceShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

        glBindVertexArray(cellRenderObject.traceVAO);
        glDrawArrays(GL_LINE_STRIP, 0, cellRenderObject.numTraceVertices);
        glBindVertexArray(0);

        xOffset += maxWidth;
    }
    _traceShader.release();
}

void EMRenderer::showAxons(bool enabled)
{
    _showAxons = enabled;
}

void EMRenderer::buildRenderObjects(const std::vector<Cell>& cells)
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

    // Build render objects
    _cellRenderObjects.resize(cells.size());
    qDebug() << "Build render objects: " << cells.size();
    for (int i = 0; i < cells.size(); i++)
        buildRenderObject(cells[i], _cellRenderObjects[i]);

    // Compute new widget width
    float newWidgetWidthToRequest = 0;
    for (int i = 0; i < cells.size(); i++)
    {
        newWidgetWidthToRequest += sqrtf(powf(_cellRenderObjects[i].ranges.x, 2) + powf(_cellRenderObjects[i].ranges.z, 2)) * 1.2f;
    }

    float maxScale = 0.01f;
    for (int i = 0; i < _cellRenderObjects.size(); i++)
    {
        if (_cellRenderObjects[i].maxExtent > maxScale)
            maxScale = _cellRenderObjects[i].maxExtent;
    }
    maxScale *= 1.8f;

    float aspectRatioRequest = newWidgetWidthToRequest / maxScale;

    emit requestNewAspectRatio(aspectRatioRequest);
}

void EMRenderer::buildRenderObject(const Cell& cell, CellRenderObject& cellRenderObject)
{
    // Morphology
    if (cell.morphology != nullptr)
    {
        const CellMorphology& cellMorphology = *cell.morphology;
        MorphologyLineSegments lineSegments;

        mv::Vector3f                somaPosition;
        float                       somaRadius;

        // Generate line segments
        try
        {
            for (int i = 0; i < cellMorphology.ids.size(); i++)
            {
                mv::Vector3f position = cellMorphology.positions.at(i);
                float radius = cellMorphology.radii.at(i);

                if (cellMorphology.types.at(i) == (int)CellMorphology::Type::Soma)
                {
                    somaPosition = position;
                    somaRadius = radius;
                    break;
                }
            }

            for (int i = 1; i < cellMorphology.parents.size(); i++)
            {
                if (cellMorphology.parents[i] == -1) // New root found, there is no line segment here so skip it
                    continue;

                int id = cellMorphology.idMap.at(cellMorphology.ids[i]);
                int parent = cellMorphology.idMap.at(cellMorphology.parents[i]);

                int type = cellMorphology.types[id];

                // Hide axons if so indicated
                if (!_showAxons && type == (int)CellMorphology::Type::Axon)
                    continue;

                float radius = cellMorphology.radii[id];

                lineSegments.segments.push_back(cellMorphology.positions[parent]);
                lineSegments.segments.push_back(cellMorphology.positions[id]);
                lineSegments.segmentRadii.push_back(radius);
                lineSegments.segmentRadii.push_back(radius);
                lineSegments.segmentTypes.push_back(type);
                lineSegments.segmentTypes.push_back(type);
            }
        }
        catch (std::out_of_range& oor)
        {
            qWarning() << "Out of range error in setCellMorphology(): " << oor.what();
            return;
        }

        // Initialize VAO and VBOs
        glGenVertexArrays(1, &cellRenderObject.vao);
        glBindVertexArray(cellRenderObject.vao);

        glGenBuffers(1, &cellRenderObject.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &cellRenderObject.rbo);
        glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.rbo);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &cellRenderObject.tbo);
        glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.tbo);
        glVertexAttribIPointer(2, 1, GL_INT, 0, 0);
        glEnableVertexAttribArray(2);

        // Store data on GPU
        glBindVertexArray(cellRenderObject.vao);

        glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.vbo);
        glBufferData(GL_ARRAY_BUFFER, lineSegments.segments.size() * sizeof(mv::Vector3f), lineSegments.segments.data(), GL_STATIC_DRAW);
        qDebug() << "VBO size: " << (lineSegments.segments.size() * sizeof(mv::Vector3f)) / 1000 << "kb";
        glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.rbo);
        glBufferData(GL_ARRAY_BUFFER, lineSegments.segmentRadii.size() * sizeof(float), lineSegments.segmentRadii.data(), GL_STATIC_DRAW);
        qDebug() << "RBO size: " << (lineSegments.segmentRadii.size() * sizeof(float)) / 1000 << "kb";
        glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.tbo);
        glBufferData(GL_ARRAY_BUFFER, lineSegments.segmentTypes.size() * sizeof(int), lineSegments.segmentTypes.data(), GL_STATIC_DRAW);
        qDebug() << "TBO size: " << (lineSegments.segmentTypes.size() * sizeof(int)) / 1000 << "kb";

        cellRenderObject.numVertices = (int)lineSegments.segments.size();

        if (_showAxons)
            cellRenderObject.anchorPoint = (cellMorphology.minRange + cellMorphology.maxRange) / 2;
        else
            cellRenderObject.anchorPoint = (cellMorphology.noAxonMinRange + cellMorphology.noAxonMaxRange) / 2;

        mv::Vector3f range = _showAxons ? (cellMorphology.maxRange - cellMorphology.minRange) : (cellMorphology.noAxonMaxRange - cellMorphology.noAxonMinRange);
        float maxExtent = std::max(std::max(range.x, range.y), range.z);
        cellRenderObject.ranges = range;
        cellRenderObject.maxExtent = maxExtent;

        cellRenderObject.cellTypeColor = cellMorphology.cellTypeColor;
    }
    if (cell.ephysTraces != nullptr)
    {
        const Experiment& experiment = *cell.ephysTraces;

        if (experiment.getAcquisitions().empty())
            return;

        // Generate line segments
        const Recording& trace = experiment.getAcquisitions()[0];
        const TimeSeries& ts = trace.GetData();

        std::vector<Vector3f> vertices;
        for (int i = 0; i < ts.xSeries.size(); i++)
        {
            vertices.emplace_back(ts.xSeries[i], ts.ySeries[i], 0);
        }
        float xMin = std::numeric_limits<float>::max(), xMax = -std::numeric_limits<float>::max(), yMin = std::numeric_limits<float>::max(), yMax = -std::numeric_limits<float>::max();
        for (int i = 0; i < vertices.size(); i++)
        {
            Vector3f& v = vertices[i];
            if (v.x < xMin) xMin = v.x;
            if (v.x > xMax) xMax = v.x;
            if (v.y < yMin) yMin = v.y;
            if (v.y > yMax) yMax = v.y;
        }
        float xRange = xMax - xMin;
        float yRange = yMax - yMin;
        for (int i = 0; i < vertices.size(); i++)
        {
            vertices[i].x = (vertices[i].x - xMin) / xRange;
            vertices[i].y = (vertices[i].y - yMin) / yRange;
        }

        // Initialize VAO and VBOs
        glGenVertexArrays(1, &cellRenderObject.traceVAO);
        glBindVertexArray(cellRenderObject.traceVAO);

        glGenBuffers(1, &cellRenderObject.traceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.traceVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vector3f), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        cellRenderObject.numTraceVertices = vertices.size();
    }
}

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

    float computeMaxCellHeight(const std::vector<CellRenderObject>& cellRenderObjects)
    {
        float maxHeight = std::numeric_limits<float>::min();
        for (int i = 0; i < cellRenderObjects.size(); i++)
        {
            if (cellRenderObjects[i].morphologyObject.dimensions.y > maxHeight)
                maxHeight = cellRenderObjects[i].morphologyObject.dimensions.y;
        }
        return maxHeight;
    }

    float computeOpenGLHeight(float maxCellHeight)
    {
        return maxCellHeight * 1.4; // 3/4 cell, 1/4 trace + some margin -> total height = 1.333 * maxCellHeight + margin ~ 1.4
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

    _lineShader.bind();

    float maxCellHeight = computeMaxCellHeight(_cellRenderObjects);
    float maxOpenGLHeight = computeOpenGLHeight(maxCellHeight);
    _viewMatrix.setToIdentity();
    _viewMatrix.scale(1.0f / maxOpenGLHeight); // Map [0, maxOpenGLHeight] to [0, 1]

    float xOffset = 0;
    //qDebug() << "Rendering " << _cellRenderObjects.size() << " objects.";
    for (int i = 0; i < _cellRenderObjects.size(); i++)
    {
        CellRenderObject& cellRenderObject = _cellRenderObjects[i];

        mv::Vector3f anchorPoint = cellRenderObject.morphologyObject.anchorPoint;

        float maxWidth = sqrtf(powf(cellRenderObject.morphologyObject.dimensions.x, 2) + powf(cellRenderObject.morphologyObject.dimensions.z, 2)) * 1.2f;

        //qDebug() << "YOffset" << yOffset;
        _modelMatrix.setToIdentity();
        _modelMatrix.translate(xOffset + maxWidth / 2, maxOpenGLHeight * 0.625, 0);
        _modelMatrix.rotate(t, 0, 1, 0);
        _modelMatrix.translate(-anchorPoint.x, -anchorPoint.y, -anchorPoint.z);
        //qDebug() << cellRenderObject.somaPosition.x << cellRenderObject.somaPosition.y << cellRenderObject.somaPosition.z;
        //qDebug() << cellRenderObject.maxExtent;

        _lineShader.uniformMatrix4f("projMatrix", _projMatrix.constData());
        _lineShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());
        _lineShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

        _lineShader.uniform3f("cellTypeColor", cellRenderObject.cellTypeColor);

        glBindVertexArray(cellRenderObject.morphologyObject.vao);
        glDrawArrays(GL_LINES, 0, cellRenderObject.morphologyObject.numVertices);
        glBindVertexArray(0);

        xOffset += maxWidth;
    }

    _lineShader.release();

    _traceShader.bind();

    xOffset = 0;
    for (int i = 0; i < _cellRenderObjects.size(); i++)
    {
        CellRenderObject& cellRenderObject = _cellRenderObjects[i];

        float maxWidth = sqrtf(powf(cellRenderObject.morphologyObject.dimensions.x, 2) + powf(cellRenderObject.morphologyObject.dimensions.z, 2)) * 1.2f;

        _traceShader.uniformMatrix4f("projMatrix", _projMatrix.constData());
        _traceShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());

        _modelMatrix.setToIdentity();
        _modelMatrix.translate(xOffset + maxWidth / 2 - maxOpenGLHeight * 0.1f, maxOpenGLHeight * 0.05f, 0); // FIXME change maxHeight to something else
        _modelMatrix.scale(maxOpenGLHeight * 0.2f, maxOpenGLHeight * 0.1f, 1);
        _traceShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

        _traceShader.uniform3f("lineColor", 0.2f, 0.4f, 0.839f);

        glBindVertexArray(cellRenderObject.acquisitionObject.vao);
        glDrawArrays(GL_LINE_STRIP, 0, cellRenderObject.acquisitionObject.numVertices);
        glBindVertexArray(0);

        _modelMatrix.setToIdentity();
        _modelMatrix.translate(xOffset + maxWidth / 2 - maxOpenGLHeight * 0.1f, maxOpenGLHeight * 0.15f, 0); // FIXME change maxHeight to something else
        _modelMatrix.scale(maxOpenGLHeight * 0.2f, maxOpenGLHeight * 0.1f, 1);
        _traceShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

        _traceShader.uniform3f("lineColor", 0.839f, 0.4f, 0.2f);

        glBindVertexArray(cellRenderObject.stimulusObject.vao);
        glDrawArrays(GL_LINE_STRIP, 0, cellRenderObject.stimulusObject.numVertices);
        glBindVertexArray(0);

        xOffset += maxWidth;
    }
    _traceShader.release();
}

void EMRenderer::showAxons(bool enabled)
{
    _showAxons = enabled;

    RebuildMorphologies();
}

void EMRenderer::setCurrentStimset(const QString& stimSet)
{
    _currentStimset = stimSet;

    RebuildTraces();
}

void EMRenderer::BuildRenderObjects(const std::vector<Cell>& cells)
{
    // Delete previous render objects
    for (CellRenderObject& cellRenderObject : _cellRenderObjects)
    {
        glDeleteBuffers(1, &cellRenderObject.morphologyObject.vbo);
        glDeleteBuffers(1, &cellRenderObject.morphologyObject.rbo);
        glDeleteBuffers(1, &cellRenderObject.morphologyObject.tbo);
        glDeleteVertexArrays(1, &cellRenderObject.morphologyObject.vao);

        glDeleteBuffers(1, &cellRenderObject.stimulusObject.vbo);
        glDeleteBuffers(1, &cellRenderObject.acquisitionObject.vbo);

        glDeleteVertexArrays(1, &cellRenderObject.stimulusObject.vao);
        glDeleteVertexArrays(1, &cellRenderObject.acquisitionObject.vao);
    }

    _cellRenderObjects.clear();

    // Compute stimulus chart height
    _stimChartRangeMin = -1;
    _stimChartRangeMax = 1;
    for (int i = 0; i < cells.size(); i++)
    {
        const Cell& cell = cells[i];
        if (cell.ephysTraces != nullptr)
        {
            const Experiment& experiment = *cell.ephysTraces;

            if (experiment.getStimuli().empty())
                return;

            for (int j = 0; j < experiment.getStimuli().size(); j++)
            {
                const Recording& stim = experiment.getStimuli()[j];
                if (stim.GetStimulusDescription() == "X4PS_SupraThresh")
                {
                    if (stim.GetData().yMin < _stimChartRangeMin) _stimChartRangeMin = stim.GetData().yMin;
                    if (stim.GetData().yMax > _stimChartRangeMax) _stimChartRangeMax = stim.GetData().yMax;
                    
                    break;
                }
                const Recording& acq = experiment.getAcquisitions()[j];

                if (acq.GetData().yMin < _acqChartRangeMin) _acqChartRangeMin = acq.GetData().yMin;
                if (acq.GetData().yMax > _acqChartRangeMax) _acqChartRangeMax = acq.GetData().yMax;
            }
        }
    }

    // Build render objects
    _cellRenderObjects.resize(cells.size());
    qDebug() << "Build render objects: " << cells.size();
    for (int i = 0; i < cells.size(); i++)
        buildRenderObject(cells[i], _cellRenderObjects[i]);

    // Compute new widget width
    float newWidgetWidthToRequest = 0;
    for (int i = 0; i < cells.size(); i++)
    {
        newWidgetWidthToRequest += sqrtf(powf(_cellRenderObjects[i].morphologyObject.dimensions.x, 2) + powf(_cellRenderObjects[i].morphologyObject.dimensions.z, 2)) * 1.2f;
    }

    float maxCellHeight = computeMaxCellHeight(_cellRenderObjects);
    float maxOpenGLHeight = computeOpenGLHeight(maxCellHeight);

    float aspectRatioRequest = newWidgetWidthToRequest / maxOpenGLHeight;

    emit requestNewAspectRatio(aspectRatioRequest);
}

void EMRenderer::buildRenderObject(const Cell& cell, CellRenderObject& cellRenderObject)
{
    // Morphology
    if (cell.morphology != nullptr)
    {
        MorphologyRenderObject& mro = cellRenderObject.morphologyObject;
        const CellMorphology& cellMorphology = *cell.morphology;

        _renderObjectBuilder.BuildMorphologyObject(mro, cellMorphology, _showAxons);

        cellRenderObject.cellTypeColor = cellMorphology.cellTypeColor;
    }
    if (cell.ephysTraces != nullptr)
    {
        const Experiment& experiment = *cell.ephysTraces;

        if (experiment.getAcquisitions().empty() || experiment.getStimuli().empty())
            return;

        for (int i = 0; i < experiment.getStimuli().size(); i++)
        {
            const Recording& recording = experiment.getStimuli()[i];
            if (recording.GetStimulusDescription() == _currentStimset)
            {
                buildTraceRenderObject(cellRenderObject.stimulusObject, experiment.getStimuli()[i], true);
                buildTraceRenderObject(cellRenderObject.acquisitionObject, experiment.getAcquisitions()[i], false);
                break;
            }
        }
    }
}

void EMRenderer::buildTraceRenderObject(TraceRenderObject& ro, const Recording& trace, bool isStim)
{
    // Generate line segments
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
        if (isStim)
            vertices[i].y = (vertices[i].y - _stimChartRangeMin) / (_stimChartRangeMax - _stimChartRangeMin);
        else
            vertices[i].y = (vertices[i].y - _acqChartRangeMin) / (_acqChartRangeMax - _acqChartRangeMin);
    }
    
    // Initialize VAO and VBOs
    glGenVertexArrays(1, &ro.vao);
    glBindVertexArray(ro.vao);

    glGenBuffers(1, &ro.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ro.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vector3f), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    ro.numVertices = vertices.size();
}

void EMRenderer::RebuildMorphologies()
{

}

void EMRenderer::RebuildTraces()
{

}

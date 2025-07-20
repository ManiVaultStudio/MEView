#include "EMRenderer.h"

#include <cmath>

namespace
{
    QMatrix4x4 MapYRangeToUnit(float bottom, float top)
    {
        // We want to map from [bottom, top] to [0, 1].
        // So therefore to map bottom to 0, translate by -bottom, then scale by 1/fabs(top-bottom).
        QMatrix4x4 M;
        M.scale(1.0f / fabs(top - bottom));
        M.translate(0, bottom, 0);
        return M;
    }

    float computeMaxCellHeight(const std::vector<CellRenderObject*>& cellRenderObjects)
    {
        float maxHeight = std::numeric_limits<float>::min();
        for (int i = 0; i < cellRenderObjects.size(); i++)
        {
            CellMorphology::Extent extent = cellRenderObjects[i]->morphologyObject.totalExtent;
            mv::Vector3f dimensions = extent.emax - extent.emin;

            if (dimensions.y > maxHeight)
                maxHeight = dimensions.y;
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

void EMRenderer::resize(int w, int h, float pixelRatio)
{
    float aspectRatio = (float)w / h;
    qDebug() << "Resize called";
    _projMatrix.setToIdentity();
    _projMatrix.ortho(0, aspectRatio, 0, 1, 1, -1);

    _fullViewport.Set(0, 0, w, h);

    int quarter = h / 3.0f;
    {
        int margin = 48 * pixelRatio;
        int topMargin = 32 * pixelRatio;
        int bottomMargin = quarter;
        _morphologyViewport.Set(margin, bottomMargin, w, h - topMargin - bottomMargin);
    }
    {
        int margin = 48 * pixelRatio;
        int topMargin = h - (quarter - (16 * pixelRatio));
        int bottomMargin = 16 * pixelRatio;
        _traceViewport.Set(margin, bottomMargin, w, h - topMargin - bottomMargin);
    }
}

void EMRenderer::update(float t)
{
    _fullViewport.Begin();

    _lineShader.bind();

    // Build list of selected cell render object references
    std::vector<CellRenderObject*> cellRenderObjects;
    for (const Cell& cell : _renderState._selectedCells)
    {
        auto it = _renderState._cellRenderObjects.find(cell.cellId);

        if (it != _renderState._cellRenderObjects.end())
            cellRenderObjects.push_back(&(*it));
        else
            qDebug() << "[EMRenderer] This should never happen, but cellId wasn't found in _cellRenderObjects";
    }

    float maxCellHeight = computeMaxCellHeight(cellRenderObjects);
    float maxOpenGLHeight = computeOpenGLHeight(maxCellHeight);
    _viewMatrix.setToIdentity();
    _lineShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());
    //_viewMatrix.scale(1.0f / maxOpenGLHeight); // Map [0, maxOpenGLHeight] to [0, 1]

    _morphologyViewport.Begin();
    _projMatrix.setToIdentity();
    _projMatrix.ortho(0, _morphologyViewport.GetAspectRatio(), 0, 1, -1, 1);

    _lineShader.uniformMatrix4f("projMatrix", _projMatrix.constData());

    float xOffset = 0;

    //QMatrix4x4 yToUnit;
    //if (cortical)
    //    yToUnit = MapYRangeToUnit(-_scene.getCortexStructure().getMaxDepth(), -_scene.getCortexStructure().getMinDepth());

    std::vector<CellMorphology::Type> ignoredTypes;
    if (!_showAxons)
        ignoredTypes.push_back(CellMorphology::Type::Axon);

    _horizontalCellLocations.clear();

    //qDebug() << "Rendering " << cellRenderObjects.size() << " objects.";
    for (int i = 0; i < cellRenderObjects.size(); i++)
    {
        CellRenderObject* cro = cellRenderObjects[i];

        cro->morphologyObject.ComputeExtents(ignoredTypes);
        const CellMorphology::Extent& extent = cro->morphologyObject.totalExtent;

        mv::Vector3f dimensions = extent.emax - extent.emin;

        float maxWidth = sqrtf(powf(dimensions.x, 2) + powf(dimensions.z, 2)) * 1.5f;

        float xCoord = 0;
        // Map from the original cell to its height being [0, 1], and the other dimensions proportional
        _modelMatrix.setToIdentity();
        if (_isCortical)
        {
            float depthRange = _scene.getCortexStructure().getDepthRange();
            QMatrix4x4 cortexMatrix = _scene.getCortexStructure().mapCellToStructure(cro->morphologyObject.somaPosition, extent.center);

            _modelMatrix.translate((xOffset + maxWidth / 2) / depthRange, 0, 0);
            _modelMatrix.rotate(t, 0, 1, 0);
            _modelMatrix *= cortexMatrix;

            xCoord = (xOffset + maxWidth / 2) / depthRange;
        }
        else
        {
            _modelMatrix.translate((xOffset + maxWidth / 2) / maxCellHeight, 0, 0);
            _modelMatrix.rotate(t, 0, 1, 0);
            _modelMatrix.scale(1.0f / maxCellHeight);
            _modelMatrix.translate(-extent.center.x, -extent.emin.y, -extent.center.z);

            xCoord = (xOffset + maxWidth / 2) / maxCellHeight;
        }
        _lineShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

        {
            QVector4D clipSpace = (_projMatrix * QVector4D(xCoord, 0, 0, 1));
            QVector4D ndc(clipSpace.x() / clipSpace.w(), clipSpace.y() / clipSpace.w(), clipSpace.z() / clipSpace.w(), 1);
            xCoord = _morphologyViewport.GetScreenCoordinates(ndc).x();
            _horizontalCellLocations.push_back(xCoord);
        }

        _lineShader.uniform3f("cellTypeColor", cro->cellTypeColor);

        for (auto it = cro->morphologyObject.processes.begin(); it != cro->morphologyObject.processes.end(); ++it)
        {
            CellMorphology::Type type = it.key();
            if (type == CellMorphology::Type::Axon && !_showAxons)
                continue;
            MorphologyProcessRenderObject mpro = it.value();
            _lineShader.uniform1i("type", (int)type);
            glBindVertexArray(mpro.vao);
            glDrawArrays(GL_LINES, 0, mpro.numVertices);
        }

        xOffset += maxWidth;
    }
    _lineShader.release();

    _morphologyViewport.End();

    // TRACES
    _traceViewport.Begin();

    _traceShader.bind();

    _projMatrix.setToIdentity();
    _projMatrix.ortho(0, _traceViewport.GetAspectRatio(), 0, 1, -1, 1);
    _viewMatrix.setToIdentity();

    _traceShader.uniformMatrix4f("projMatrix", _projMatrix.constData());
    _traceShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());

    xOffset = 0;
    for (int i = 0; i < cellRenderObjects.size(); i++)
    {
        CellRenderObject* cro = cellRenderObjects[i];

        const CellMorphology::Extent& extent = cro->morphologyObject.totalExtent;
        mv::Vector3f dimensions = extent.emax - extent.emin;
        float maxWidth = sqrtf(powf(dimensions.x, 2) + powf(dimensions.z, 2)) * 1.5f;

        for (int i = 0; i < cro->stimulusObjects.size(); i++)
        {
            TraceRenderObject& stimRO = cro->stimulusObjects[i];

            if (stimRO.stimulusDescription == _currentStimset)
            {
                TraceRenderObject& acqRO = cro->acquisitionsObjects[i];

                float r = _traceViewport.GetAspectRatio() / _morphologyViewport.GetAspectRatio();

                float height;
                if (_isCortical)
                    height = _scene.getCortexStructure().getDepthRange();
                else
                    height = maxCellHeight;

                // Acquisition
                _modelMatrix.setToIdentity();
                _modelMatrix.translate((xOffset + maxWidth / 2) / height * r, 0, 0);
                //_modelMatrix.scale(maxOpenGLHeight * 0.2f, maxOpenGLHeight * 0.1f, 1);
                _modelMatrix.translate(-0.35, 0, 0);
                _modelMatrix.scale(0.8f / acqRO.extents.getWidth(), 0.4f / (_renderState._acqChartRangeMax - _renderState._acqChartRangeMin), 1);
                _modelMatrix.translate(-acqRO.extents.getLeft(), -_renderState._acqChartRangeMin, 0);

                _traceShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

                _traceShader.uniform3f("lineColor", 0.2f, 0.4f, 0.839f);

                glBindVertexArray(acqRO.vao);
                glDrawArrays(GL_LINE_STRIP, 0, acqRO.numVertices);

                // Stimulus
                _modelMatrix.setToIdentity();
                _modelMatrix.translate((xOffset + maxWidth / 2) / height * r, 0, 0);
                //_modelMatrix.scale(maxOpenGLHeight * 0.2f, maxOpenGLHeight * 0.1f, 1);
                _modelMatrix.translate(-0.35, 0.5, 0);
                _modelMatrix.scale(0.8f / stimRO.extents.getWidth(), 0.4f / (_renderState._stimChartRangeMax - _renderState._stimChartRangeMin), 1);
                _modelMatrix.translate(-stimRO.extents.getLeft(), -_renderState._stimChartRangeMin, 0);
                _traceShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

                _traceShader.uniform3f("lineColor", 0.839f, 0.4f, 0.2f);

                glBindVertexArray(stimRO.vao);
                glDrawArrays(GL_LINE_STRIP, 0, stimRO.numVertices);

                break;
            }
        }

        xOffset += maxWidth;
    }

    glBindVertexArray(0);
    _traceShader.release();

    _fullViewport.End();
}

void EMRenderer::showAxons(bool enabled)
{
    _showAxons = enabled;

    RequestNewWidgetWidth();
}

void EMRenderer::setCurrentStimset(const QString& stimSet)
{
    _currentStimset = stimSet;
}

void EMRenderer::SetCortical(bool isCortical)
{
    _isCortical = isCortical;
}

std::vector<float> EMRenderer::GetHorizontalCellLocations()
{
    return _horizontalCellLocations;
}

void EMRenderer::SetSelectedCellIds(const std::vector<Cell>& cells)
{
    // Build list of selected cell render object references
    std::vector<CellRenderObject*> cellRenderObjects;
    BuildListOfCellRenderObjects(cells, cellRenderObjects);

    _renderState._selectedCells = cells;

    // Compute stimulus chart height
    _renderState._stimChartRangeMin = std::numeric_limits<float>::max();
    _renderState._stimChartRangeMax = -std::numeric_limits<float>::max();
    _renderState._acqChartRangeMin = std::numeric_limits<float>::max();
    _renderState._acqChartRangeMax = -std::numeric_limits<float>::max();
    for (int i = 0; i < cells.size(); i++)
    {
        const Cell& cell = cells[i];
        if (cell.ephysTraces != nullptr)
        {
            const Experiment& experiment = *cell.ephysTraces;

            if (experiment.getStimuli().empty())
                break;

            for (int j = 0; j < experiment.getStimuli().size(); j++)
            {
                const Recording& stim = experiment.getStimuli()[j];
                if (stim.GetStimulusDescription() == _currentStimset)
                {
                    if (stim.GetData().yMin < _renderState._stimChartRangeMin) _renderState._stimChartRangeMin = stim.GetData().yMin;
                    if (stim.GetData().yMax > _renderState._stimChartRangeMax) _renderState._stimChartRangeMax = stim.GetData().yMax;

                    const Recording& acq = experiment.getAcquisitions()[j];

                    if (acq.GetData().yMin < _renderState._acqChartRangeMin) _renderState._acqChartRangeMin = acq.GetData().yMin;
                    if (acq.GetData().yMax > _renderState._acqChartRangeMax) _renderState._acqChartRangeMax = acq.GetData().yMax;

                    break;
                }
            }
        }
    }

    RequestNewWidgetWidth();
}

void EMRenderer::BuildRenderObjects(const std::vector<Cell>& cells)
{
    _renderObjectBuilder.BuildCellRenderObjects(cells);
}

void EMRenderer::BuildListOfCellRenderObjects(const std::vector<Cell>& cells, std::vector<CellRenderObject*>& cellRenderObjects)
{
    // Build list of selected cell render object references
    for (const Cell& cell : cells)
    {
        auto it = _renderState._cellRenderObjects.find(cell.cellId);

        if (it != _renderState._cellRenderObjects.end())
            cellRenderObjects.push_back(&(*it));
        else
            qDebug() << "[EMRenderer] This should never happen, but cellId wasn't found in _cellRenderObjects";
    }
}

void EMRenderer::RequestNewWidgetWidth()
{
    std::vector<CellRenderObject*> cellRenderObjects;
    BuildListOfCellRenderObjects(_renderState._selectedCells, cellRenderObjects);

    std::vector<CellMorphology::Type> ignoredTypes;
    if (!_showAxons)
        ignoredTypes.push_back(CellMorphology::Type::Axon);

    // Compute new widget width
    float newWidgetWidthToRequest = 0;
    for (int i = 0; i < cellRenderObjects.size(); i++)
    {
        cellRenderObjects[i]->morphologyObject.ComputeExtents(ignoredTypes);
        CellMorphology::Extent extent = cellRenderObjects[i]->morphologyObject.totalExtent;
        mv::Vector3f dimensions = extent.emax - extent.emin;

        newWidgetWidthToRequest += sqrtf(powf(dimensions.x, 2) + powf(dimensions.z, 2)) * 1.2f;
    }

    float maxCellHeight = computeMaxCellHeight(cellRenderObjects);
    float maxOpenGLHeight = computeOpenGLHeight(maxCellHeight);

    float aspectRatioRequest = newWidgetWidthToRequest / maxOpenGLHeight;

    emit requestNewAspectRatio(aspectRatioRequest);
}

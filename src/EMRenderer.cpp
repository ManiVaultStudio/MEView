#include "EMRenderer.h"

#include <cmath>

namespace
{
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

    int FindHighestPriorityStimulus(const CellRenderObject& cro, QString currentStimset)
    {
        float maxPriority = -std::numeric_limits<float>::max();
        int stimulusIndex = -1;
        for (int i = 0; i < cro.stimulusObjects.size(); i++)
        {
            const TraceRenderObject& stimRO = cro.stimulusObjects[i];

            if (stimRO.stimulusDescription == currentStimset)
            {
                if (stimRO.priority > maxPriority)
                {
                    maxPriority = stimRO.priority;
                    stimulusIndex = i;
                }
            }
        }
        return stimulusIndex;
    }
}

void EMRenderer::init()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    loaded &= _lineShader.loadShaderFromFile(":me_view/shaders/PassThrough.vert", ":me_view/shaders/Lines.frag");
    loaded &= _traceShader.loadShaderFromFile(":me_view/shaders/Trace.vert", ":me_view/shaders/Trace.frag");
    //loaded &= _quadShader.loadShaderFromFile(":me_view/shaders/Quad.vert", ":me_view/shaders/Quad.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }

    glEnable(GL_LINE_SMOOTH);
}

void EMRenderer::resize(int w, int h, float pixelRatio)
{
    float aspectRatio = (float)w / h;
    qDebug() << "Resize called";

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
        int topMargin = h - (quarter - (8 * pixelRatio));
        int bottomMargin = 16 * pixelRatio;
        _traceViewport.Set(margin, bottomMargin, w, h - topMargin - bottomMargin);
    }

    _morphProjMatrix.setToIdentity();
    _morphProjMatrix.ortho(0, _morphologyViewport.GetAspectRatio(), 0, 1, -3, 3);
    _traceProjMatrix.setToIdentity();
    _traceProjMatrix.ortho(0, _traceViewport.GetAspectRatio(), 0, 1, -3, 3);
}

void EMRenderer::update(float t)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _fullViewport.Begin();

    _lineShader.bind();

    // Build list of selected cell render object references
    std::vector<CellRenderObject*> cellRenderObjects;
    BuildListOfCellRenderObjects(_renderState._selectedCells, cellRenderObjects);

    float maxCellHeight = computeMaxCellHeight(cellRenderObjects);

    _morphologyViewport.Begin();

    _lineShader.uniformMatrix4f("projMatrix", _morphProjMatrix.constData());

    float xOffset = 0;

    std::vector<CellMorphology::Type> ignoredTypes;
    if (!_enabledProcesses.contains("Axon"))
        ignoredTypes.push_back(CellMorphology::Type::Axon);
    if (!_enabledProcesses.contains("Apical Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::ApicalDendrite);
    if (!_enabledProcesses.contains("Basal Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::BasalDendrite);
    
    _horizontalCellLocations.clear();

    for (int i = 0; i < cellRenderObjects.size(); i++)
    {
        CellRenderObject* cro = cellRenderObjects[i];

        cro->morphologyObject.ComputeExtents(ignoredTypes);
        const CellMorphology::Extent& extent = cro->morphologyObject.totalExtent;

        mv::Vector3f dimensions = extent.emax - extent.emin;

        float maxWidth = sqrtf(powf(dimensions.x, 2) + powf(dimensions.z, 2)) * 1.8f;

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
            QVector4D clipSpace = (_morphProjMatrix * QVector4D(xCoord, 0, 0, 1));
            QVector4D ndc(clipSpace.x() / clipSpace.w(), clipSpace.y() / clipSpace.w(), clipSpace.z() / clipSpace.w(), 1);
            xCoord = _morphologyViewport.GetScreenCoordinates(ndc).x();
            _horizontalCellLocations.push_back(xCoord);
        }

        _lineShader.uniform3f("cellTypeColor", cro->cellTypeColor);

        for (auto it = cro->morphologyObject.processes.begin(); it != cro->morphologyObject.processes.end(); ++it)
        {
            CellMorphology::Type type = it.key();
            if (std::find(ignoredTypes.begin(), ignoredTypes.end(), type) != ignoredTypes.end())
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

    _traceShader.uniformMatrix4f("projMatrix", _traceProjMatrix.constData());

    xOffset = 0;
    for (int i = 0; i < cellRenderObjects.size(); i++)
    {
        CellRenderObject* cro = cellRenderObjects[i];

        const CellMorphology::Extent& extent = cro->morphologyObject.totalExtent;
        mv::Vector3f dimensions = extent.emax - extent.emin;
        float maxWidth = sqrtf(powf(dimensions.x, 2) + powf(dimensions.z, 2)) * 1.8f;

        int stimIndex = FindHighestPriorityStimulus(*cro, _currentStimset);

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
                _modelMatrix.translate(-0.3, 0, 0);
                _modelMatrix.scale(0.6f / acqRO.extents.getWidth(), 0.4f / (_renderState._acqChartRangeMax - _renderState._acqChartRangeMin), 1);
                _modelMatrix.translate(-acqRO.extents.getLeft(), -_renderState._acqChartRangeMin, 0);

                _traceShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

                Vector3f acqColor = i == stimIndex ? Vector3f(0.27f, 0.51f, 0.71f) : Vector3f(0.5f);
                _traceShader.uniform3f("lineColor", acqColor);
                _traceShader.uniform1f("alpha", i == stimIndex ? 1.0f : 0.1f);

                glBindVertexArray(acqRO.vao);
                glDrawArrays(GL_LINE_STRIP, 0, acqRO.numVertices);

                // Stimulus
                _modelMatrix.setToIdentity();
                _modelMatrix.translate((xOffset + maxWidth / 2) / height * r, 0, 0);
                _modelMatrix.translate(-0.3, 0.5, 0);
                _modelMatrix.scale(0.6f / stimRO.extents.getWidth(), 0.4f / (_renderState._stimChartRangeMax - _renderState._stimChartRangeMin), 1);
                _modelMatrix.translate(-stimRO.extents.getLeft(), -_renderState._stimChartRangeMin, 0);
                _traceShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

                Vector3f stimColor = i == stimIndex ? Vector3f(0.839f, 0.4f, 0.2f) : Vector3f(0.5f);
                _traceShader.uniform3f("lineColor", stimColor);

                glBindVertexArray(stimRO.vao);
                glDrawArrays(GL_LINE_STRIP, 0, stimRO.numVertices);
            }
        }

        xOffset += maxWidth;
    }
    glDisable(GL_BLEND);

    glBindVertexArray(0);
    _traceShader.release();

    _fullViewport.End();
}

void EMRenderer::SetEnabledProcesses(const QStringList& enabledProcesses)
{
    _enabledProcesses = enabledProcesses;

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
                continue;

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
    // If the QOpenGLFunctions are not yet initialized, don't try to request a size yet
    if (!isInitialized())
        return;

    std::vector<CellRenderObject*> cellRenderObjects;
    BuildListOfCellRenderObjects(_renderState._selectedCells, cellRenderObjects);

    std::vector<CellMorphology::Type> ignoredTypes;
    if (!_enabledProcesses.contains("Axon"))
        ignoredTypes.push_back(CellMorphology::Type::Axon);
    if (!_enabledProcesses.contains("Apical Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::ApicalDendrite);
    if (!_enabledProcesses.contains("Basal Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::BasalDendrite);

    // Compute new widget width
    float newWidgetWidthToRequest = 0;
    for (int i = 0; i < cellRenderObjects.size(); i++)
    {
        cellRenderObjects[i]->morphologyObject.ComputeExtents(ignoredTypes);
        CellMorphology::Extent extent = cellRenderObjects[i]->morphologyObject.totalExtent;
        mv::Vector3f dimensions = extent.emax - extent.emin;

        newWidgetWidthToRequest += sqrtf(powf(dimensions.x, 2) + powf(dimensions.z, 2)) * 1.8f;
    }

    if (newWidgetWidthToRequest == 0)
        return;

    _morphProjMatrix.setToIdentity();
    _morphProjMatrix.ortho(0, _morphologyViewport.GetAspectRatio(), 0, 1, -1, 1);

    float morphHeight = _isCortical ? _scene.getCortexStructure().getDepthRange() : computeMaxCellHeight(cellRenderObjects);
    QVector4D clipSpace = (_morphProjMatrix * QVector4D(newWidgetWidthToRequest / morphHeight, 0, 0, 1));
    QVector4D ndc(clipSpace.x() / clipSpace.w(), clipSpace.y() / clipSpace.w(), clipSpace.z() / clipSpace.w(), 1);
    newWidgetWidthToRequest = _morphologyViewport.GetScreenCoordinates(ndc).x();

    // Limit new width request to GPU limits
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    newWidgetWidthToRequest = newWidgetWidthToRequest > maxTextureSize ? maxTextureSize : newWidgetWidthToRequest;

    float aspectRatioRequest = newWidgetWidthToRequest / _fullViewport.GetHeight();

    emit requestNewAspectRatio(aspectRatioRequest);
}

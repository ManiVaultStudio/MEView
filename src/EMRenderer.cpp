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

    GLuint CreateSomaVAO(QOpenGLFunctions_3_3_Core* f)
    {
        GLuint vao, vbo, tbo;
        f->glGenVertexArrays(1, &vao);
        f->glBindVertexArray(vao);

        std::vector<Vector3f> vertices
        {
            Vector3f(-1, -1, 0),
            Vector3f(1, -1, 0),
            Vector3f(-1, 1, 0),
            Vector3f(1, 1, 0)
        };

        std::vector<Vector2f> textureCoords
        {
            Vector2f(0, 0),
            Vector2f(1, 0),
            Vector2f(0, 1),
            Vector2f(1, 1)
        };
        qDebug() << vertices.size();
        f->glGenBuffers(1, &vbo);
        f->glBindBuffer(GL_ARRAY_BUFFER, vbo);
        f->glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(mv::Vector3f), vertices.data(), GL_STATIC_DRAW);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        f->glEnableVertexAttribArray(0);

        f->glGenBuffers(1, &tbo);
        f->glBindBuffer(GL_ARRAY_BUFFER, tbo);
        f->glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(mv::Vector2f), textureCoords.data(), GL_STATIC_DRAW);
        f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        f->glEnableVertexAttribArray(1);

        return vao;
    }
}

void EMRenderer::init()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    loaded &= _lineShader.loadShaderFromFile(":me_view/shaders/PassThrough.vert", ":me_view/shaders/Lines.frag");
    loaded &= _somaShader.loadShaderFromFile(":me_view/shaders/Soma.vert", ":me_view/shaders/Soma.frag");
    loaded &= _traceShader.loadShaderFromFile(":me_view/shaders/Trace.vert", ":me_view/shaders/Trace.frag");
    //loaded &= _quadShader.loadShaderFromFile(":me_view/shaders/Quad.vert", ":me_view/shaders/Quad.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }

    _somaVAO = CreateSomaVAO(this);

    glEnable(GL_LINE_SMOOTH);
}

void EMRenderer::resize(int w, int h, float pixelRatio)
{
    float aspectRatio = (float)w / h;

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
    //glEnable(GL_DEPTH_TEST);

    _fullViewport.Begin();

    _lineShader.bind();

    // Build list of selected cell render object references
    std::vector<CellRenderObject*> cellRenderObjects;
    BuildListOfCellRenderObjects(_renderState._selectedCells, cellRenderObjects);

    float maxCellHeight = computeMaxCellHeight(cellRenderObjects);

    _morphologyViewport.Begin();

    _lineShader.uniformMatrix4f("projMatrix", _morphProjMatrix.constData());

    std::vector<CellMorphology::Type> ignoredTypes;
    if (!_enabledProcesses.contains("Axon"))
        ignoredTypes.push_back(CellMorphology::Type::Axon);
    if (!_enabledProcesses.contains("Apical Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::ApicalDendrite);
    if (!_enabledProcesses.contains("Basal Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::BasalDendrite);

    std::vector<Vector3f> somaPositions;
    for (int i = 0; i < cellRenderObjects.size(); i++)
    {
        CellRenderObject* cro = cellRenderObjects[i];

        if (cro->hasMorphology)
        {
            cro->morphologyObject.ComputeExtents(ignoredTypes);
            const CellMorphology::Extent& extent = cro->morphologyObject.totalExtent;

            float xCoord = _xCoords[i];
            // Map from the original cell to its height being [0, 1], and the other dimensions proportional
            _modelMatrix.setToIdentity();
            if (_isCortical)
            {
                float depthRange = _scene.getCortexStructure().getDepthRange();
                QMatrix4x4 cortexMatrix = _scene.getCortexStructure().mapCellToStructure(cro->morphologyObject.somaPosition, extent.center);

                _modelMatrix.translate(xCoord, 0, 0);
                _modelMatrix.rotate(t, 0, 1, 0);
                _modelMatrix *= cortexMatrix;
            }
            else
            {
                _modelMatrix.translate(xCoord, 0, 0);
                _modelMatrix.rotate(t, 0, 1, 0);
                _modelMatrix.scale(1.0f / maxCellHeight);
                _modelMatrix.translate(-extent.center.x, -extent.emin.y, -extent.center.z);
            }
            _lineShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

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

            // Save soma positions
            QVector4D somaPosition = _modelMatrix * QVector4D(cro->morphologyObject.somaPosition.x, cro->morphologyObject.somaPosition.y, cro->morphologyObject.somaPosition.z, 1);
            somaPositions.push_back(Vector3f(somaPosition.x(), somaPosition.y(), somaPosition.z()));
        }
    }
    _lineShader.release();

    // Soma rendering
    _somaShader.bind();
    _somaShader.uniformMatrix4f("projMatrix", _morphProjMatrix.constData());
    glBindVertexArray(_somaVAO);
    
    for (const Vector3f& somaPosition : somaPositions)
    {
        _somaShader.uniform3f("somaPosition", somaPosition);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    _somaShader.release();

    _morphologyViewport.End();

    // TRACES
    glDisable(GL_DEPTH_TEST);
    _traceViewport.Begin();

    _traceShader.bind();

    _traceShader.uniformMatrix4f("projMatrix", _traceProjMatrix.constData());

    for (int i = 0; i < cellRenderObjects.size(); i++)
    {
        CellRenderObject* cro = cellRenderObjects[i];

        float xCoord = _xCoords[i];

        int stimIndex = FindHighestPriorityStimulus(*cro, _currentStimset);

        for (int i = 0; i < cro->stimulusObjects.size(); i++)
        {
            TraceRenderObject& stimRO = cro->stimulusObjects[i];

            if (stimRO.stimulusDescription == _currentStimset)
            {
                TraceRenderObject& acqRO = cro->acquisitionsObjects[i];

                float r = _traceViewport.GetAspectRatio() / _morphologyViewport.GetAspectRatio();

                // Acquisition
                _modelMatrix.setToIdentity();
                _modelMatrix.translate(xCoord * r, 0, 0);
                _modelMatrix.translate(-0.3, 0, 0);
                _modelMatrix.scale(0.6f, 0.4f, 1);
                _modelMatrix.scale(1.0f / (cro->_acqChartDomainMax - cro->_acqChartDomainMin), 1.0f / (_renderState._acqChartRangeMax - _renderState._acqChartRangeMin), 1.0f); // Rescale to [0, 1]
                _modelMatrix.translate(-cro->_acqChartDomainMin, -_renderState._acqChartRangeMin, 0); // Map bottom-left corner to 0,0

                _traceShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

                Vector3f acqColor = i == stimIndex ? Vector3f(0.27f, 0.51f, 0.71f) : Vector3f(0.5f);
                _traceShader.uniform3f("lineColor", acqColor);
                _traceShader.uniform1f("alpha", i == stimIndex ? 1.0f : 0.1f);

                glBindVertexArray(acqRO.vao);
                glDrawArrays(GL_LINE_STRIP, 0, acqRO.numVertices);

                // Stimulus
                _modelMatrix.setToIdentity();
                _modelMatrix.translate(xCoord * r, 0, 0);
                _modelMatrix.translate(-0.3, 0.5, 0);
                _modelMatrix.scale(0.6f / (cro->_stimChartDomainMax - cro->_stimChartDomainMin), 0.4f / (_renderState._stimChartRangeMax - _renderState._stimChartRangeMin), 1);
                _modelMatrix.translate(-cro->_stimChartDomainMin, -_renderState._stimChartRangeMin, 0);
                _traceShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

                Vector3f stimColor = i == stimIndex ? Vector3f(0.839f, 0.4f, 0.2f) : Vector3f(0.5f);
                _traceShader.uniform3f("lineColor", stimColor);

                glBindVertexArray(stimRO.vao);
                glDrawArrays(GL_LINE_STRIP, 0, stimRO.numVertices);
            }
        }
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

void EMRenderer::SetSelectedCellIds(const std::vector<uint32_t>& indices)
{
    // Build list of selected cell render object references
    std::vector<CellRenderObject*> cellRenderObjects;
    BuildListOfCellRenderObjects(_scene.selectedCells, cellRenderObjects);

    _renderState._selectedCells = _scene.selectedCells;

    // Compute stimulus chart height
    _renderState._stimChartRangeMin = std::numeric_limits<float>::max();
    _renderState._stimChartRangeMax = -std::numeric_limits<float>::max();
    _renderState._acqChartRangeMin = std::numeric_limits<float>::max();
    _renderState._acqChartRangeMax = -std::numeric_limits<float>::max();
    for (int i = 0; i < _scene.selectedCells.size(); i++)
    {
        const Cell& cell = _scene.selectedCells[i];
        if (cell.ephysTraces != nullptr)
        {
            const Experiment& experiment = *cell.ephysTraces;

            if (experiment.getStimuli().empty())
                continue;

            CellRenderObject* cro = cellRenderObjects[i];
            cro->_stimChartDomainMin = std::numeric_limits<float>::max();
            cro->_stimChartDomainMax = -std::numeric_limits<float>::max();
            cro->_acqChartDomainMin = std::numeric_limits<float>::max();
            cro->_acqChartDomainMax = -std::numeric_limits<float>::max();

            for (int j = 0; j < experiment.getStimuli().size(); j++)
            {
                const Recording& stim = experiment.getStimuli()[j];
                if (stim.GetStimulusDescription() == _currentStimset)
                {
                    if (stim.GetData().xMin < cro->_stimChartDomainMin) cro->_stimChartDomainMin = stim.GetData().xMin;
                    if (stim.GetData().xMax > cro->_stimChartDomainMax) cro->_stimChartDomainMax = stim.GetData().xMax;

                    if (stim.GetData().yMin < _renderState._stimChartRangeMin) _renderState._stimChartRangeMin = stim.GetData().yMin;
                    if (stim.GetData().yMax > _renderState._stimChartRangeMax) _renderState._stimChartRangeMax = stim.GetData().yMax;

                    const Recording& acq = experiment.getAcquisitions()[j];

                    if (acq.GetData().xMin < cro->_acqChartDomainMin) cro->_acqChartDomainMin = acq.GetData().xMin;
                    if (acq.GetData().xMax > cro->_acqChartDomainMax) cro->_acqChartDomainMax = acq.GetData().xMax;

                    if (acq.GetData().yMin < _renderState._acqChartRangeMin) _renderState._acqChartRangeMin = acq.GetData().yMin;
                    if (acq.GetData().yMax > _renderState._acqChartRangeMax) _renderState._acqChartRangeMax = acq.GetData().yMax;
                }
            }
        }
    }

    RequestNewWidgetWidth();
}

void EMRenderer::ComputeRenderLocations(const std::vector<CellRenderObject*>& cellRenderObjects)
{
    float maxCellHeight = computeMaxCellHeight(cellRenderObjects);

    std::vector<CellMorphology::Type> ignoredTypes;
    if (!_enabledProcesses.contains("Axon"))
        ignoredTypes.push_back(CellMorphology::Type::Axon);
    if (!_enabledProcesses.contains("Apical Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::ApicalDendrite);
    if (!_enabledProcesses.contains("Basal Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::BasalDendrite);

    // Compute drawing locations
    _xCoords.clear();
    _horizontalCellLocations.clear();
    float xOffset = 0;
    float minWidth = _isCortical ? 0.3f : 0.6f;
    for (int i = 0; i < cellRenderObjects.size(); i++)
    {
        CellRenderObject* cro = cellRenderObjects[i];

        float xCoord = 0;
        if (cro->hasMorphology)
        {
            cro->morphologyObject.ComputeExtents(ignoredTypes);
            const CellMorphology::Extent& extent = cro->morphologyObject.totalExtent;

            mv::Vector3f dimensions = extent.emax - extent.emin;
            float maxWidth = sqrtf(powf(dimensions.x, 2) + powf(dimensions.z, 2)) * 1.2f;

            float height;
            if (_isCortical)
                height = _scene.getCortexStructure().getDepthRange();
            else
                height = maxCellHeight;

            if (_isCortical)
            {
                float depthRange = _scene.getCortexStructure().getDepthRange();
                xCoord = xOffset + std::max(minWidth / 2, (maxWidth / 2) / depthRange);
                xOffset += std::max(minWidth, maxWidth / depthRange);
            }
            else
            {
                xCoord = xOffset + std::max(minWidth / 2, (maxWidth / 2) / maxCellHeight);
                xOffset += std::max(minWidth, maxWidth / maxCellHeight);
            }
        }
        else
        {
            float r = _traceViewport.GetAspectRatio() / _morphologyViewport.GetAspectRatio();
            // FIXME why is this /2 necessary?
            xCoord = xOffset + (_isCortical ? 0.15f : 0.3f);
            xOffset += (_isCortical ? 0.3f : 0.6f);
        }
        {
            QVector4D clipSpace = (_morphProjMatrix * QVector4D(xCoord, 0, 0, 1));
            QVector4D ndc(clipSpace.x() / clipSpace.w(), clipSpace.y() / clipSpace.w(), clipSpace.z() / clipSpace.w(), 1);
            float xsCoord = _morphologyViewport.GetScreenCoordinates(ndc).x();
            _horizontalCellLocations.push_back(xsCoord);
        }

        _xCoords.push_back(xCoord);
    }
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

    ComputeRenderLocations(cellRenderObjects);

    if (_xCoords.empty())
        return;

    // Compute new widget width
    float newWidgetWidthToRequest = _xCoords[_xCoords.size() - 1] + 0.6; // FIXME little hack for extra space

    if (newWidgetWidthToRequest == 0)
        return;

    _morphProjMatrix.setToIdentity();
    _morphProjMatrix.ortho(0, _morphologyViewport.GetAspectRatio(), 0, 1, -1, 1);

    //float morphHeight = _isCortical ? _scene.getCortexStructure().getDepthRange() : computeMaxCellHeight(cellRenderObjects);
    QVector4D clipSpace = (_morphProjMatrix * QVector4D(newWidgetWidthToRequest, 0, 0, 1));
    QVector4D ndc(clipSpace.x() / clipSpace.w(), clipSpace.y() / clipSpace.w(), clipSpace.z() / clipSpace.w(), 1);
    newWidgetWidthToRequest = _morphologyViewport.GetScreenCoordinates(ndc).x();

    // Limit new width request to GPU limits
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    newWidgetWidthToRequest = newWidgetWidthToRequest > maxTextureSize ? maxTextureSize : newWidgetWidthToRequest;

    float aspectRatioRequest = newWidgetWidthToRequest / _fullViewport.GetHeight();

    emit requestNewAspectRatio(aspectRatioRequest);
}

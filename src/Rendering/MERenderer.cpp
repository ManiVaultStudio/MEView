#include "Rendering/MERenderer.h"

#include <QPainter>

constexpr float DIVISION_F = 0.25f;

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

    int FindHighestPriorityStimulus(const CellRenderObject& cro, StimulusType currentStimType)
    {
        float maxPriority = -std::numeric_limits<float>::max();
        int stimulusIndex = -1;
        for (int i = 0; i < cro.stimulusObjects.size(); i++)
        {
            const TraceRenderObject& stimRO = cro.stimulusObjects[i];

            if (stimRO.stimulusType == currentStimType)
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

MERenderer::MERenderer() :
    _scene(Scene::getInstance()),
    _renderObjectBuilder(this),
    _fullViewport(this),
    _morphologyViewport(this),
    _traceViewport(this)
{

}

void MERenderer::Init()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    loaded &= _lineShader.loadShaderFromFile(":me_view/shaders/PassThrough.vert", ":me_view/shaders/Lines.frag");
    loaded &= _somaShader.loadShaderFromFile(":me_view/shaders/Soma.vert", ":me_view/shaders/Soma.frag");
    loaded &= _traceShader.loadShaderFromFile(":me_view/shaders/Trace.vert", ":me_view/shaders/Trace.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }

    _somaVAO = _renderObjectBuilder.BuildCellSoma();

    glEnable(GL_LINE_SMOOTH);
}

void MERenderer::Resize(int w, int h, float pixelRatio)
{
    _pixelRatio = pixelRatio;
    _fullViewport.Set(0, 0, w, h);

    int division = h * DIVISION_F;

    // left, right, bottom, top
    mv::Bounds morphologyBounds(48 * pixelRatio, w, division, h - 32 * pixelRatio);
    mv::Bounds traceBounds(48 * pixelRatio, w, 8 * pixelRatio, morphologyBounds.getBottom() - (8 * pixelRatio));

    _morphologyViewport.Set(morphologyBounds);
    _traceViewport.Set(traceBounds);

    ComputeRenderLocations(_selectedCellRenderObjects);
}

void MERenderer::Update(float t, QPainter& painter)
{
    //RenderMorphologies();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_DEPTH_TEST);

    _fullViewport.Begin();

    RenderMorphologies(t);
    RenderSomas();
    RenderTraces();

    _fullViewport.End();
}

void MERenderer::RenderMorphologies(float t)
{
    _lineShader.bind();

    float maxCellHeight = computeMaxCellHeight(_selectedCellRenderObjects);

    _morphologyViewport.Begin();

    _lineShader.uniformMatrix4f("projMatrix", _morphologyViewport.GetProjectionMatrix().constData());

    _context.somaPositions.clear();
    std::vector<CellMorphology::Type> ignoredTypes;
    if (!_enabledProcesses.contains("Axon"))
        ignoredTypes.push_back(CellMorphology::Type::Axon);
    if (!_enabledProcesses.contains("Apical Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::ApicalDendrite);
    if (!_enabledProcesses.contains("Basal Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::BasalDendrite);

    for (int i = 0; i < _selectedCellRenderObjects.size(); i++)
    {
        CellRenderObject* cro = _selectedCellRenderObjects[i];

        // Set CRO color
        // FIXME getGuiName is fragile
        QString clusterName = _scene.getCellMetadataDataset()->getColumn(_scene.currentClusterDataset->getGuiName())[cro->cellMetadataIndex];
        auto& clusters = _scene.currentClusterDataset->getClusters();
        for (const Cluster& cluster : clusters)
        {
            if (cluster.getName() == clusterName)
            {
                QColor color = cluster.getColor();
                cro->cellTypeColor = mv::Vector3f(color.redF(), color.greenF(), color.blueF());
            }
        }

        if (cro->hasMorphology)
        {
            cro->morphologyObject.ComputeExtents(ignoredTypes);
            const CellMorphology::Extent& extent = cro->morphologyObject.totalExtent;

            float xCoord = _context.xCoords[i];
            // Map from the original cell to its height being [0, 1], and the other dimensions proportional
            _context.modelMatrix.setToIdentity();
            if (_isCortical)
            {
                float depthRange = _scene.getCortexStructure().getDepthRange();
                QMatrix4x4 cortexMatrix = _scene.getCortexStructure().mapCellToStructure(cro->morphologyObject.somaPosition, extent.center);

                _context.modelMatrix.translate(xCoord, 0, 0);
                _context.modelMatrix.rotate(t, 0, 1, 0);
                _context.modelMatrix *= cortexMatrix;
            }
            else
            {
                _context.modelMatrix.translate(xCoord, 0, 0);
                _context.modelMatrix.rotate(t, 0, 1, 0);
                _context.modelMatrix.scale(1.0f / maxCellHeight);
                _context.modelMatrix.translate(-extent.center.x, -extent.emin.y, -extent.center.z);
            }
            _lineShader.uniformMatrix4f("modelMatrix", _context.modelMatrix.constData());
            //_scene.getCellMetadataDataset()->getColumn()

            // Set cell color
            _lineShader.uniform3f("cellTypeColor", cro->cellTypeColor);
            _lineShader.uniform1f("axonTransparency", _axonTransparency);

            //
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
            QVector4D somaPosition = _context.modelMatrix * QVector4D(cro->morphologyObject.somaPosition.x, cro->morphologyObject.somaPosition.y, cro->morphologyObject.somaPosition.z, 1);
            _context.somaPositions.push_back(mv::Vector3f(somaPosition.x(), somaPosition.y(), somaPosition.z()));
        }
    }
    _lineShader.release();

    _morphologyViewport.End();
}

void MERenderer::RenderSomas()
{
    _morphologyViewport.Begin();

    // Soma rendering
    _somaShader.bind();
    _somaShader.uniformMatrix4f("projMatrix", _morphologyViewport.GetProjectionMatrix().constData());
    glBindVertexArray(_somaVAO);

    for (const mv::Vector3f& somaPosition : _context.somaPositions)
    {
        _somaShader.uniform3f("somaPosition", somaPosition);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    _somaShader.release();

    _morphologyViewport.End();
}

void MERenderer::RenderTraces()
{
    constexpr float TRACE_WIDTH = 1.6f;
    constexpr float ACQ_HEIGHT = 0.6f;
    constexpr float STIM_HEIGHT = 0.3f;

    // TRACES
    glDisable(GL_DEPTH_TEST);
    _traceViewport.Begin();

    _traceShader.bind();

    _traceShader.uniformMatrix4f("projMatrix", _traceViewport.GetProjectionMatrix().constData());

    for (int i = 0; i < _selectedCellRenderObjects.size(); i++)
    {
        CellRenderObject* cro = _selectedCellRenderObjects[i];

        float xCoord = _context.xCoords[i];

        int stimIndex = FindHighestPriorityStimulus(*cro, _currentStimType);

        float r = _traceViewport.GetAspectRatio() / _morphologyViewport.GetAspectRatio();

        auto LRenderTrace = [&](int traceIndex)
        {
            TraceRenderObject& stimRO = cro->stimulusObjects[traceIndex];

            if (stimRO.stimulusType != _currentStimType)
                return;

            TraceRenderObject& acqRO = cro->acquisitionsObjects[traceIndex];

            const bool isHighlighted = traceIndex == stimIndex;

            // Acquisition
            _context.modelMatrix.setToIdentity();
            _context.modelMatrix.translate(xCoord * r, 0.0f, 0.0f);
            _context.modelMatrix.translate(-TRACE_WIDTH * 0.5f, 0.5f, 0.0f);
            _context.modelMatrix.scale(TRACE_WIDTH, ACQ_HEIGHT, 1.0f);
            _context.modelMatrix.scale(1.0f / (cro->_acqChartDomainMax - cro->_acqChartDomainMin), 1.0f / (_acqChartRange.max - _acqChartRange.min), 1.0f); // Rescale to [0, 1]
            _context.modelMatrix.translate(-cro->_acqChartDomainMin, -_acqChartRange.min, 0.0f); // Map bottom-left corner to 0,0
            _traceShader.uniformMatrix4f("modelMatrix", _context.modelMatrix.constData());

            const mv::Vector3f acqColor = isHighlighted ? cro->cellTypeColor : mv::Vector3f(0.7f);

            _traceShader.uniform3f("lineColor", acqColor);
            _traceShader.uniform1f("alpha", isHighlighted ? 1.0f : 0.05f);

            glBindVertexArray(acqRO.vao);
            glDrawArrays(GL_LINE_STRIP, 0, acqRO.numVertices);

            // Stimulus
            _context.modelMatrix.setToIdentity();
            _context.modelMatrix.translate(xCoord * r, 0.0f, 0.0f);
            _context.modelMatrix.translate(-TRACE_WIDTH * 0.5f, 0.0f, 0.0f);
            _context.modelMatrix.scale(TRACE_WIDTH / (cro->_stimChartDomainMax - cro->_stimChartDomainMin), STIM_HEIGHT / (_stimChartRange.max - _stimChartRange.min), 1);
            _context.modelMatrix.translate(-cro->_stimChartDomainMin, -_stimChartRange.min, 0);
            _traceShader.uniformMatrix4f("modelMatrix", _context.modelMatrix.constData());

            const mv::Vector3f stimColor = isHighlighted ? mv::Vector3f(0.2f) : mv::Vector3f(0.5f);

            _traceShader.uniform3f("lineColor", stimColor);

            glBindVertexArray(stimRO.vao);
            glDrawArrays(GL_LINE_STRIP, 0, stimRO.numVertices);
        };

        // Draw all unhighlighted traces first
        for (int traceIndex = 0; traceIndex < cro->stimulusObjects.size(); ++traceIndex)
        {
            if (traceIndex != stimIndex)
                LRenderTrace(traceIndex);
        }

        // Draw the highlighted trace last so it appears on top
        if (stimIndex >= 0 && stimIndex < cro->stimulusObjects.size())
            LRenderTrace(stimIndex);
    }

    glDisable(GL_BLEND);

    glBindVertexArray(0);
    _traceShader.release();
}

void MERenderer::RenderLabels(QPainter& painter)
{
    float height = _fullViewport.GetHeight() / _pixelRatio;
    int bottomMargin = height * DIVISION_F; // Pixel ratio margin
    int yCoord = 4;

    QFont originalFont = painter.font();
    QFont layerFont = originalFont;
    layerFont.setPointSizeF(layerFont.pointSizeF());
    //layerFont.setBold(true);
    painter.setFont(layerFont);

    QFontMetrics fm(layerFont);
    int textHeight = fm.height();

    QPen textPen(QColor(100, 100, 100, 255), 2, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);

    std::vector<float> horizontalCellLocations = GetHorizontalCellLocations();
    for (int i = 0; i < horizontalCellLocations.size(); i++)
    {
        int xCoord = horizontalCellLocations[i] / _pixelRatio;

        int textWidth = fm.horizontalAdvance(_scene.selectedCells[i].cluster);

        CellRenderObject* cro = _selectedCellRenderObjects[i];
        mv::Vector3f color = cro->cellTypeColor;
        textPen.setColor(QColor(color.x * 0.65f * 255, color.y * 0.65f * 255, color.z * 0.65f * 255, 255));
        painter.setPen(textPen);

        const QRect boundingRect = QRect(xCoord - 50, yCoord, 100, 28);
        painter.drawText(boundingRect, Qt::AlignCenter | Qt::AlignTop | Qt::TextWordWrap, _scene.selectedCells[i].cluster);
    }

    painter.setFont(originalFont);
}

void MERenderer::RenderVerticalLine(QPainter& painter, float x)
{
    float height = _fullViewport.GetHeight() / _pixelRatio;
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);

    int topMargin = 32; // Non-pixel ratio margin
    int bottomMargin = height * DIVISION_F; // Pixel ratio margin
    int chartHeight = height - topMargin - bottomMargin;

    // Draw line centered at a pixel, so it doesn't bleed onto multiple pixels
    const qreal px = x; // snapToDeviceRow(painter, x);
    QPen pen;
    pen.setColor(QColor(200, 200, 200, 255));
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);
    painter.drawLine(QPointF(px, topMargin), QPointF(px, chartHeight + topMargin));

    painter.restore();
}

void MERenderer::RenderSeparations(QPainter& painter)
{
    for (int i = 0; i + 1 < _context.horizontalCellLocations.size(); i++)
    {
        QString cluster1 = _scene.selectedCells[i].cluster;
        QString cluster2 = _scene.selectedCells[i+1].cluster;

        if (cluster1 != cluster2)
        {
            float x1 = _context.horizontalCellLocations[i];
            float x2 = _context.horizontalCellLocations[i + 1];
            RenderVerticalLine(painter, (x1 + x2) / (2 * _pixelRatio));
        }
    }
}

void MERenderer::ComputeRenderLocations(const std::vector<CellRenderObject*>& cellRenderObjects)
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
    _context.xCoords.clear();
    _context.horizontalCellLocations.clear();
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
            qDebug() << "Extent: " << extent.emin.str() << extent.emax.str();
            mv::Vector3f dimensions = extent.emax - extent.emin;
            float maxWidth = sqrtf(powf(dimensions.x, 2) + powf(dimensions.z, 2)) * 1.2f;

            float height;
            if (_isCortical)
                height = _scene.getCortexStructure().getDepthRange();
            else
                height = maxCellHeight;

            xCoord = xOffset + std::max(minWidth / 2, (maxWidth / 2) / height);
            xOffset += std::max(minWidth, maxWidth / height);

            qDebug() << "xCoord c: " << xCoord << height;
            qDebug() << "xOffset: " << xOffset;
        }
        else
        {
            float r = _traceViewport.GetAspectRatio() / _morphologyViewport.GetAspectRatio();
            // FIXME why is this /2 necessary?
            xCoord = xOffset + (_isCortical ? 0.15f : 0.3f);
            xOffset += (_isCortical ? 0.3f : 0.6f);
        }
        {
            QVector4D clipSpace = (_morphologyViewport.GetProjectionMatrix() * QVector4D(xCoord, 0, 0, 1));
            QVector4D ndc(clipSpace.x() / clipSpace.w(), clipSpace.y() / clipSpace.w(), clipSpace.z() / clipSpace.w(), 1);
            float xsCoord = _morphologyViewport.GetScreenCoordinates(ndc).x();
            _context.horizontalCellLocations.push_back(xsCoord);
        }

        _context.xCoords.push_back(xCoord);
    }
}

void MERenderer::SetCortical(bool isCortical)
{
    _isCortical = isCortical;
}

void MERenderer::SetEnabledProcesses(const QStringList& enabledProcesses)
{
    _enabledProcesses = enabledProcesses;

    RequestNewWidgetWidth();
}

void MERenderer::SetCurrentStimType(const QString& stimType)
{
    _currentStimType = StimulusTypeFromString(stimType);

    RecalculateTraceBounds();
}

void MERenderer::BuildRenderObjects(const std::vector<Cell>& cells)
{
    _renderObjectBuilder.BuildCellRenderObjects(cells, _cellRenderObjects);
}

std::vector<float> MERenderer::GetHorizontalCellLocations()
{
    return _context.horizontalCellLocations;
}

void MERenderer::CompileSelectedCellRenderObjects(const std::vector<Cell>& cells)
{
    _selectedCellRenderObjects.clear();
    // Build list of selected cell render object references
    for (const Cell& cell : cells)
    {
        auto it = _cellRenderObjects.find(cell.cellId);

        if (it != _cellRenderObjects.end())
            _selectedCellRenderObjects.push_back(&(*it));
        else
            qDebug() << "[MERenderer] This should never happen, but cellId wasn't found in _cellRenderObjects";
    }
}

void MERenderer::SetSelectedCellIds(const std::vector<uint32_t>& indices)
{
    CompileSelectedCellRenderObjects(_scene.selectedCells);

    RecalculateTraceBounds();

    RequestNewWidgetWidth();
}

void MERenderer::RecalculateTraceBounds()
{
    // Compute stimulus chart height
    _stimChartRange.min = std::numeric_limits<float>::max();
    _stimChartRange.max = -std::numeric_limits<float>::max();
    _acqChartRange.min = std::numeric_limits<float>::max();
    _acqChartRange.max = -std::numeric_limits<float>::max();
    for (int i = 0; i < _scene.selectedCells.size(); i++)
    {
        const Cell& cell = _scene.selectedCells[i];
        if (cell.ephysTraces != nullptr)
        {
            const Experiment& experiment = *cell.ephysTraces;

            if (experiment.GetSweeps().empty())
                continue;

            CellRenderObject* cro = _selectedCellRenderObjects[i];
            cro->_stimChartDomainMin = std::numeric_limits<float>::max();
            cro->_stimChartDomainMax = -std::numeric_limits<float>::max();
            cro->_acqChartDomainMin = std::numeric_limits<float>::max();
            cro->_acqChartDomainMax = -std::numeric_limits<float>::max();

            for (int j = 0; j < experiment.GetSweeps().size(); j++)
            {
                const Sweep& sweep = experiment.GetSweeps()[j];

                if (sweep.stimulus.GetType() != _currentStimType)
                    continue;

                //const Recording& stimRec = sweep.stimulus.GetRecording();

                cro->_stimChartDomainMin = std::min(cro->_stimChartDomainMin, sweep.stimulus.GetWindowStart());
                cro->_stimChartDomainMax = std::max(cro->_stimChartDomainMax, sweep.stimulus.GetWindowEnd());

                _stimChartRange.min = std::min(_stimChartRange.min, sweep.stimulus.GetYMin());
                _stimChartRange.max = std::max(_stimChartRange.max, sweep.stimulus.GetYMax());

                const Recording& acq = experiment.GetSweeps()[j].acquisition.GetRecording();

                if (acq.GetData().xMin < cro->_acqChartDomainMin) cro->_acqChartDomainMin = acq.GetData().xMin;
                if (acq.GetData().xMax > cro->_acqChartDomainMax) cro->_acqChartDomainMax = acq.GetData().xMax;

                if (acq.GetData().yMin < _acqChartRange.min) _acqChartRange.min = acq.GetData().yMin;
                if (acq.GetData().yMax > _acqChartRange.max) _acqChartRange.max = acq.GetData().yMax;
            }
        }
    }
}

void MERenderer::RequestNewWidgetWidth()
{
    // If the QOpenGLFunctions are not yet initialized, don't try to request a size yet
    if (!isInitialized())
        return;

    std::vector<CellMorphology::Type> ignoredTypes;
    if (!_enabledProcesses.contains("Axon"))
        ignoredTypes.push_back(CellMorphology::Type::Axon);
    if (!_enabledProcesses.contains("Apical Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::ApicalDendrite);
    if (!_enabledProcesses.contains("Basal Dendrite"))
        ignoredTypes.push_back(CellMorphology::Type::BasalDendrite);

    ComputeRenderLocations(_selectedCellRenderObjects);

    if (_context.xCoords.empty())
        return;

    for (int i = 0; i < _context.xCoords.size(); i++)
        qDebug() << "xCoord:" << _context.xCoords[i];

    // Compute new widget width
    float newWidgetWidthToRequest = _context.xCoords[_context.xCoords.size() - 1] + 0.6; // FIXME little hack for extra space

    if (newWidgetWidthToRequest == 0)
        return;

    QMatrix4x4& projMatrix = _morphologyViewport.GetProjectionMatrix();
    qDebug() << "pre new width: " << newWidgetWidthToRequest;
    //float morphHeight = _isCortical ? _scene.getCortexStructure().getDepthRange() : computeMaxCellHeight(cellRenderObjects);
    QVector4D clipSpace = (projMatrix * QVector4D(newWidgetWidthToRequest, 0, 0, 1));
    QVector4D ndc(clipSpace.x() / clipSpace.w(), clipSpace.y() / clipSpace.w(), clipSpace.z() / clipSpace.w(), 1);
    newWidgetWidthToRequest = _morphologyViewport.GetScreenCoordinates(ndc).x();

    // Limit new width request to GPU limits
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    newWidgetWidthToRequest = newWidgetWidthToRequest > maxTextureSize ? maxTextureSize : newWidgetWidthToRequest;

    qDebug() << "Max tex size: " << maxTextureSize;
    qDebug() << "New framebuffer size requested: " << newWidgetWidthToRequest;

    float aspectRatioRequest = newWidgetWidthToRequest / _fullViewport.GetHeight();

    emit RequestNewAspectRatio(aspectRatioRequest);
}

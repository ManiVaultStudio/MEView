#include "Rendering/MERenderer.h"

#include <QPainter>

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    constexpr float MORPHOLOGY_AREA_FRACTION = 0.25f;
    constexpr float STANDARD_MIN_CELL_WIDTH = 0.6f;
    constexpr float CORTICAL_MIN_CELL_WIDTH = 0.3f;
    constexpr float MORPHOLOGY_WIDTH_PADDING = 1.0f;
    constexpr float TRACE_WIDTH = 1.0f;
    constexpr float MISSING_TRACE_IMAGE_SIZE = 1.0f;
    constexpr float SLOT_PADDING = 0.03f;
    constexpr float RIGHT_MARGIN = 0.3f;
    constexpr float LABEL_WIDTH = 100.0f;
    constexpr float ACQUISITION_HEIGHT = 0.6f;
    constexpr float STIMULUS_HEIGHT = 0.3f;

    float SafeSpan(float minValue, float maxValue)
    {
        const float span = maxValue - minValue;
        return std::abs(span) > std::numeric_limits<float>::epsilon() ? span : 1.0f;
    }

    int HighestPriorityStimulus(const CellRenderObject& cell, StimulusType type)
    {
        float bestPriority = -std::numeric_limits<float>::max();
        int bestIndex = -1;

        for (int i = 0; i < static_cast<int>(cell.stimulusObjects.size()); ++i)
        {
            const TraceRenderObject& stimulus = cell.stimulusObjects[i];
            if (stimulus.stimulusType != type || stimulus.priority <= bestPriority) continue;

            bestPriority = stimulus.priority;
            bestIndex = i;
        }

        return bestIndex;
    }
}

MERenderer::MERenderer() : _scene(Scene::getInstance()), _renderObjectBuilder(this)
{
}

void MERenderer::Init()
{
    initializeOpenGLFunctions();

    bool loaded = true;
    loaded &= _lineShader.loadShaderFromFile(":me_view/shaders/PassThrough.vert", ":me_view/shaders/Lines.frag");
    loaded &= _somaShader.loadShaderFromFile(":me_view/shaders/Soma.vert", ":me_view/shaders/Soma.frag");
    loaded &= _traceShader.loadShaderFromFile(":me_view/shaders/Trace.vert", ":me_view/shaders/Trace.frag");
    loaded &= _texShader.loadShaderFromFile(":me_view/shaders/Texture.vert", ":me_view/shaders/Texture.frag");

    if (!loaded) qCritical() << "Failed to load one of the morphology shaders";

    _noSweepsTex.loadFromFile(":me_view/images/SweepNotAvailable.png");
    _somaVAO = _renderObjectBuilder.BuildCellSoma();
    glGenVertexArrays(1, &_imageVAO);
    glEnable(GL_LINE_SMOOTH);
}

void MERenderer::Resize(int width, int height, float pixelRatio)
{
    _widgetWidth = std::max(width, 1);
    _widgetHeight = std::max(height, 1);
    _pixelRatio = pixelRatio;

    const int left = static_cast<int>(48.0f * pixelRatio);
    const int morphologyBottom = static_cast<int>(_widgetHeight * MORPHOLOGY_AREA_FRACTION);
    const int morphologyTop = _widgetHeight - static_cast<int>(32.0f * pixelRatio);
    const int traceBottom = static_cast<int>(8.0f * pixelRatio);
    const int traceTop = morphologyBottom - static_cast<int>(8.0f * pixelRatio);

    _morphologyBand = {left, morphologyBottom, std::max(1, _widgetWidth - left), std::max(1, morphologyTop - morphologyBottom)};
    _traceBand = {left, traceBottom, std::max(1, _widgetWidth - left), std::max(1, traceTop - traceBottom)};

    RebuildLayout();
}

void MERenderer::Update(float rotation, QPainter& painter)
{
    Q_UNUSED(painter);

    glViewport(0, 0, _widgetWidth, _widgetHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    RenderMorphologies(rotation);
    RenderEphys();

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
}

void MERenderer::SetCortical(bool cortical)
{
    if (_isCortical == cortical) return;

    _isCortical = cortical;
    RebuildLayout();
    RequestWidgetWidth();
}

void MERenderer::SetEnabledProcesses(const QStringList& enabledProcesses)
{
    if (_enabledProcesses == enabledProcesses) return;

    _enabledProcesses = enabledProcesses;
    RebuildLayout();
    RequestWidgetWidth();
}

void MERenderer::SetCurrentStimType(const QString& stimulusType)
{
    const StimulusType type = StimulusTypeFromString(stimulusType);
    if (_currentStimType == type) return;

    _currentStimType = type;
    RecalculateTraceRanges();
}

void MERenderer::BuildRenderObjects(const std::vector<Cell>& cells)
{
    _renderObjectBuilder.BuildCellRenderObjects(cells, _cellRenderObjects);
    RebuildLayout();
}

void MERenderer::SetSelectedCellIds(const std::vector<uint32_t>& indices)
{
    Q_UNUSED(indices);

    RebuildLayout();
    RecalculateTraceRanges();
    RequestWidgetWidth();
}

std::vector<float> MERenderer::GetHorizontalCellLocations() const
{
    std::vector<float> centers;
    centers.reserve(_slots.size());

    for (const CellSlot& slot : _slots) centers.push_back(slot.centerPx);
    return centers;
}

std::vector<CellMorphology::Type> MERenderer::IgnoredMorphologyTypes() const
{
    std::vector<CellMorphology::Type> ignored;

    if (!_enabledProcesses.contains("Axon")) ignored.push_back(CellMorphology::Type::Axon);
    if (!_enabledProcesses.contains("Apical Dendrite")) ignored.push_back(CellMorphology::Type::ApicalDendrite);
    if (!_enabledProcesses.contains("Basal Dendrite")) ignored.push_back(CellMorphology::Type::BasalDendrite);

    return ignored;
}

QMatrix4x4 MERenderer::ProjectionFor(const Band& band) const
{
    QMatrix4x4 pixelProjection;
    pixelProjection.ortho(0.0f, static_cast<float>(_widgetWidth), 0.0f, static_cast<float>(_widgetHeight), -3.0f, 3.0f);

    QMatrix4x4 bandToPixels;
    bandToPixels.translate(static_cast<float>(band.left), static_cast<float>(band.bottom), 0.0f);
    bandToPixels.scale(static_cast<float>(band.height), static_cast<float>(band.height), 1.0f);

    return pixelProjection * bandToPixels;
}

void MERenderer::RebuildLayout()
{
    _slots.clear();

    const std::vector<CellMorphology::Type> ignoredTypes = IgnoredMorphologyTypes();
    float maximumMorphologyHeight = 0.0f;

    _slots.reserve(_scene.selectedCells.size());

    for (int i = 0; i < static_cast<int>(_scene.selectedCells.size()); ++i)
    {
        const Cell& cell = _scene.selectedCells[i];
        auto it = _cellRenderObjects.find(cell.cellId);
        CellRenderObject* renderObject = it == _cellRenderObjects.end() ? nullptr : &(*it);

        if (!renderObject) qDebug() << "[MERenderer] cellId wasn't found in _cellRenderObjects";

        if (renderObject && renderObject->hasMorphology)
        {
            renderObject->morphologyObject.ComputeExtents(ignoredTypes);
            const CellMorphology::Extent& extent = renderObject->morphologyObject.totalExtent;
            maximumMorphologyHeight = std::max(maximumMorphologyHeight, (extent.emax - extent.emin).y);
        }

        _slots.push_back({i, renderObject});
    }

    _morphologyReferenceHeight = _isCortical ? _scene.getCortexStructure().getDepthRange() : maximumMorphologyHeight;
    _morphologyReferenceHeight = std::max(_morphologyReferenceHeight, 1.0f);

    const float baseWidthPx = (_isCortical ? CORTICAL_MIN_CELL_WIDTH : STANDARD_MIN_CELL_WIDTH) * _morphologyBand.height;
    const float traceWidthPx = TRACE_WIDTH * _traceBand.height;
    const float imageWidthPx = MISSING_TRACE_IMAGE_SIZE * _traceBand.height;
    const float labelWidthPx = LABEL_WIDTH * _pixelRatio;
    const float paddingPx = SLOT_PADDING * _morphologyBand.height;
    const float minimumWidthPx = std::max({baseWidthPx, traceWidthPx, imageWidthPx, labelWidthPx}) + 2.0f * paddingPx;

    float x = static_cast<float>(_morphologyBand.left);

    for (CellSlot& slot : _slots)
    {
        float morphologyWidthPx = 0.0f;

        if (slot.renderObject && slot.renderObject->hasMorphology)
        {
            const CellMorphology::Extent& extent = slot.renderObject->morphologyObject.totalExtent;
            const mv::Vector3f dimensions = extent.emax - extent.emin;
            morphologyWidthPx = std::hypot(dimensions.x, dimensions.z) / _morphologyReferenceHeight * MORPHOLOGY_WIDTH_PADDING * _morphologyBand.height;
        }

        const float widthPx = std::max(minimumWidthPx, morphologyWidthPx);
        slot.leftPx = x;
        slot.centerPx = x + widthPx * 0.5f;
        slot.rightPx = x + widthPx;
        x += widthPx;
    }

    _contentRightPx = x;
}

void MERenderer::RecalculateTraceRanges()
{
    float stimulusMin = std::numeric_limits<float>::max();
    float stimulusMax = -std::numeric_limits<float>::max();
    float acquisitionMin = std::numeric_limits<float>::max();
    float acquisitionMax = -std::numeric_limits<float>::max();

    bool hasStimulus = false;
    bool hasAcquisition = false;

    for (const CellSlot& slot : _slots)
    {
        if (!slot.renderObject || slot.cellIndex < 0 || slot.cellIndex >= static_cast<int>(_scene.selectedCells.size())) continue;

        const Cell& cell = _scene.selectedCells[slot.cellIndex];
        if (!cell.ephysTraces) continue;

        CellRenderObject& renderObject = *slot.renderObject;
        renderObject._stimChartDomainMin = std::numeric_limits<float>::max();
        renderObject._stimChartDomainMax = -std::numeric_limits<float>::max();
        renderObject._acqChartDomainMin = std::numeric_limits<float>::max();
        renderObject._acqChartDomainMax = -std::numeric_limits<float>::max();

        for (const Sweep& sweep : cell.ephysTraces->GetSweeps())
        {
            if (sweep.stimulus.GetType() != _currentStimType) continue;

            hasStimulus = true;
            renderObject._stimChartDomainMin = std::min(renderObject._stimChartDomainMin, sweep.stimulus.GetWindowStart());
            renderObject._stimChartDomainMax = std::max(renderObject._stimChartDomainMax, sweep.stimulus.GetWindowEnd());
            stimulusMin = std::min(stimulusMin, sweep.stimulus.GetYMin());
            stimulusMax = std::max(stimulusMax, sweep.stimulus.GetYMax());

            const Recording& acquisition = sweep.acquisition.GetRecording();
            hasAcquisition = true;
            renderObject._acqChartDomainMin = std::min(renderObject._acqChartDomainMin, acquisition.GetData().xMin);
            renderObject._acqChartDomainMax = std::max(renderObject._acqChartDomainMax, acquisition.GetData().xMax);
            acquisitionMin = std::min(acquisitionMin, acquisition.GetData().yMin);
            acquisitionMax = std::max(acquisitionMax, acquisition.GetData().yMax);
        }
    }

    _stimulusRange = hasStimulus ? Range{stimulusMin, stimulusMax} : Range{};
    _acquisitionRange = hasAcquisition ? Range{acquisitionMin, acquisitionMax} : Range{};
}

void MERenderer::RequestWidgetWidth()
{
    if (!isInitialized() || _slots.empty() || _widgetHeight <= 0) return;

    float requestedWidthPx = _contentRightPx + RIGHT_MARGIN * _morphologyBand.height;

    GLint maxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    requestedWidthPx = std::min(requestedWidthPx, static_cast<float>(maxTextureSize));

    emit RequestNewAspectRatio(requestedWidthPx / _widgetHeight);
}

void MERenderer::RenderMorphologies(float rotation)
{
    glEnable(GL_SCISSOR_TEST);
    glScissor(_morphologyBand.left, _morphologyBand.bottom, _morphologyBand.width, _morphologyBand.height);

    const QMatrix4x4 projection = ProjectionFor(_morphologyBand);
    const std::vector<CellMorphology::Type> ignoredTypes = IgnoredMorphologyTypes();

    _somaPositions.clear();
    _lineShader.bind();
    _lineShader.uniformMatrix4f("projMatrix", projection.constData());

    for (const CellSlot& slot : _slots)
    {
        if (!slot.renderObject || slot.cellIndex < 0 || slot.cellIndex >= static_cast<int>(_scene.selectedCells.size())) continue;

        const Cell& selectedCell = _scene.selectedCells[slot.cellIndex];
        CellRenderObject& cell = *slot.renderObject;

        for (const Cluster& cluster : _scene.currentClusterDataset->getClusters())
        {
            if (cluster.getName() != selectedCell.cluster) continue;

            const QColor color = cluster.getColor();
            cell.cellTypeColor = mv::Vector3f(color.redF(), color.greenF(), color.blueF());
            break;
        }

        if (!cell.hasMorphology) continue;

        const CellMorphology::Extent& extent = cell.morphologyObject.totalExtent;
        const float centerX = (slot.centerPx - _morphologyBand.left) / _morphologyBand.height;

        QMatrix4x4 model;
        model.translate(centerX, 0.0f, 0.0f);
        model.rotate(rotation, 0.0f, 1.0f, 0.0f);

        if (_isCortical) model *= _scene.getCortexStructure().mapCellToStructure(cell.morphologyObject.somaPosition, extent.center);
        else
        {
            model.scale(1.0f / _morphologyReferenceHeight);
            model.translate(-extent.center.x, -extent.emin.y, -extent.center.z);
        }

        _lineShader.uniformMatrix4f("modelMatrix", model.constData());
        _lineShader.uniform3f("cellTypeColor", cell.cellTypeColor);
        _lineShader.uniform1f("axonTransparency", _axonTransparency);

        for (auto it = cell.morphologyObject.processes.begin(); it != cell.morphologyObject.processes.end(); ++it)
        {
            if (std::find(ignoredTypes.begin(), ignoredTypes.end(), it.key()) != ignoredTypes.end()) continue;

            const MorphologyProcessRenderObject& process = it.value();
            _lineShader.uniform1i("type", static_cast<int>(it.key()));
            glBindVertexArray(process.vao);
            glDrawArrays(GL_LINES, 0, process.numVertices);
        }

        const QVector4D soma = model * QVector4D(cell.morphologyObject.somaPosition.x, cell.morphologyObject.somaPosition.y, cell.morphologyObject.somaPosition.z, 1.0f);
        _somaPositions.emplace_back(soma.x(), soma.y(), soma.z());
    }

    _lineShader.release();

    _somaShader.bind();
    _somaShader.uniformMatrix4f("projMatrix", projection.constData());
    glBindVertexArray(_somaVAO);

    for (const mv::Vector3f& soma : _somaPositions)
    {
        _somaShader.uniform3f("somaPosition", soma);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindVertexArray(0);
    _somaShader.release();
}

void MERenderer::RenderEphys()
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glScissor(_traceBand.left, _traceBand.bottom, _traceBand.width, _traceBand.height);

    const QMatrix4x4 projection = ProjectionFor(_traceBand);

    _traceShader.bind();
    _traceShader.uniformMatrix4f("projMatrix", projection.constData());

    for (const CellSlot& slot : _slots)
    {
        if (!slot.renderObject) continue;

        CellRenderObject& cell = *slot.renderObject;
        const float centerX = (slot.centerPx - _traceBand.left) / _traceBand.height;
        const int highlightedIndex = HighestPriorityStimulus(cell, _currentStimType);

        auto drawTrace = [&](int traceIndex)
        {
            if (traceIndex < 0 || traceIndex >= static_cast<int>(cell.stimulusObjects.size()) || traceIndex >= static_cast<int>(cell.acquisitionsObjects.size())) return;

            TraceRenderObject& stimulus = cell.stimulusObjects[traceIndex];
            if (stimulus.stimulusType != _currentStimType) return;

            TraceRenderObject& acquisition = cell.acquisitionsObjects[traceIndex];
            const bool highlighted = traceIndex == highlightedIndex;

            QMatrix4x4 model;
            model.translate(centerX - TRACE_WIDTH * 0.5f, 0.5f, 0.0f);
            model.scale(TRACE_WIDTH / SafeSpan(cell._acqChartDomainMin, cell._acqChartDomainMax), ACQUISITION_HEIGHT / SafeSpan(_acquisitionRange.min, _acquisitionRange.max), 1.0f);
            model.translate(-cell._acqChartDomainMin, -_acquisitionRange.min, 0.0f);

            _traceShader.uniformMatrix4f("modelMatrix", model.constData());
            _traceShader.uniform3f("lineColor", highlighted ? cell.cellTypeColor : mv::Vector3f(0.7f));
            _traceShader.uniform1f("alpha", highlighted ? 1.0f : 0.05f);

            glBindVertexArray(acquisition.vao);
            glDrawArrays(GL_LINE_STRIP, 0, acquisition.numVertices);

            model.setToIdentity();
            model.translate(centerX - TRACE_WIDTH * 0.5f, 0.0f, 0.0f);
            model.scale(TRACE_WIDTH / SafeSpan(cell._stimChartDomainMin, cell._stimChartDomainMax), STIMULUS_HEIGHT / SafeSpan(_stimulusRange.min, _stimulusRange.max), 1.0f);
            model.translate(-cell._stimChartDomainMin, -_stimulusRange.min, 0.0f);

            _traceShader.uniformMatrix4f("modelMatrix", model.constData());
            _traceShader.uniform3f("lineColor", highlighted ? mv::Vector3f(0.2f) : mv::Vector3f(0.5f));
            _traceShader.uniform1f("alpha", highlighted ? 1.0f : 0.05f);

            glBindVertexArray(stimulus.vao);
            glDrawArrays(GL_LINE_STRIP, 0, stimulus.numVertices);
        };

        for (int i = 0; i < static_cast<int>(cell.stimulusObjects.size()); ++i)
        {
            if (i != highlightedIndex) drawTrace(i);
        }

        if (highlightedIndex >= 0) drawTrace(highlightedIndex);
    }

    glBindVertexArray(0);
    _traceShader.release();

    _texShader.bind();
    _texShader.uniformMatrix4f("projMatrix", projection.constData());
    _noSweepsTex.bind(0);
    _texShader.uniform1i("tex", 0);

    for (const CellSlot& slot : _slots)
    {
        if (!slot.renderObject) continue;

        CellRenderObject& cell = *slot.renderObject;
        const bool hasSweep = std::any_of(cell.stimulusObjects.begin(), cell.stimulusObjects.end(), [this](const TraceRenderObject& stimulus) { return stimulus.stimulusType == _currentStimType; });
        if (hasSweep) continue;

        const float centerX = (slot.centerPx - _traceBand.left) / _traceBand.height;
        const float halfSize = MISSING_TRACE_IMAGE_SIZE * 0.5f;

        QMatrix4x4 model;
        model.translate(centerX, 0.0f, 0.0f);
        model.scale(halfSize, halfSize, 1.0f);
        model.translate(0.0f, 1.0f, 0.0f);

        _texShader.uniformMatrix4f("modelMatrix", model.constData());

        glBindVertexArray(_imageVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindVertexArray(0);
    _texShader.release();
}

void MERenderer::RenderLabels(QPainter& painter)
{
    constexpr int LABEL_TOP = 4;
    constexpr int LABEL_HEIGHT = 28;

    QPen pen(QColor(100, 100, 100, 255), 2, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin);

    for (const CellSlot& slot : _slots)
    {
        if (!slot.renderObject || slot.cellIndex < 0 || slot.cellIndex >= static_cast<int>(_scene.selectedCells.size())) continue;

        const Cell& cell = _scene.selectedCells[slot.cellIndex];
        const int centerX = static_cast<int>(slot.centerPx / _pixelRatio);
        const mv::Vector3f color = slot.renderObject->cellTypeColor;

        pen.setColor(QColor(static_cast<int>(color.x * 0.65f * 255.0f), static_cast<int>(color.y * 0.65f * 255.0f), static_cast<int>(color.z * 0.65f * 255.0f), 255));
        painter.setPen(pen);
        painter.drawText(QRect(centerX - static_cast<int>(LABEL_WIDTH * 0.5f), LABEL_TOP, static_cast<int>(LABEL_WIDTH), LABEL_HEIGHT), Qt::AlignCenter | Qt::AlignTop | Qt::TextWordWrap, cell.cluster);
    }
}

void MERenderer::RenderSeparations(QPainter& painter)
{
    if (_slots.size() < 2) return;

    const float logicalHeight = _widgetHeight / _pixelRatio;
    constexpr int topMargin = 32;
    const int bottomMargin = static_cast<int>(logicalHeight * MORPHOLOGY_AREA_FRACTION);
    const int lineHeight = static_cast<int>(logicalHeight) - topMargin - bottomMargin;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(QColor(200, 200, 200, 255), 2, Qt::DashLine));

    for (int i = 0; i + 1 < static_cast<int>(_slots.size()); ++i)
    {
        const CellSlot& left = _slots[i];
        const CellSlot& right = _slots[i + 1];

        if (left.cellIndex < 0 || right.cellIndex < 0) continue;
        if (left.cellIndex >= static_cast<int>(_scene.selectedCells.size()) || right.cellIndex >= static_cast<int>(_scene.selectedCells.size())) continue;
        if (_scene.selectedCells[left.cellIndex].cluster == _scene.selectedCells[right.cellIndex].cluster) continue;

        const float x = left.rightPx / _pixelRatio;
        painter.drawLine(QPointF(x, topMargin), QPointF(x, topMargin + lineHeight));
    }

    painter.restore();
}

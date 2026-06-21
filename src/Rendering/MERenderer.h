#pragma once

#include "Rendering/RenderContext.h"
#include "Rendering/RenderObjectBuilder.h"
#include "Rendering/RenderRegion.h"

#include "graphics/Shader.h"

#include <QOpenGLFunctions_3_3_Core>

#include <QHash>

class QPainter;

struct Range
{
    float min;
    float max;
};

class MERenderer : public QObject, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    MERenderer();

    void Init();
    void Resize(int w, int h, float pixelRatio);
    void Update(float t, QPainter& painter);

    void SetCortical(bool isCortical);
    void SetEnabledProcesses(const QStringList& enabledProcesses);
    void SetCurrentStimType(const QString& stimset);
    void SetAxonTransparency(float alpha) { _axonTransparency = alpha; }

    void BuildRenderObjects(const std::vector<Cell>& cells);
    void ComputeRenderLocations(const std::vector<CellRenderObject*>& cellRenderObjects);
    std::vector<float> GetHorizontalCellLocations();
    void CompileSelectedCellRenderObjects(const std::vector<Cell>& cells);
    void SetSelectedCellIds(const std::vector<uint32_t>& indices);
    void RecalculateTraceBounds();
    void RequestNewWidgetWidth();

public: // Rendering
    void RenderLabels(QPainter& painter);
    void RenderVerticalLine(QPainter& painter, float x);
    void RenderSeparations(QPainter& painter);

private:
    void RenderMorphologies(float t);
    void RenderSomas();
    void RenderTraces();

signals:
    void RequestNewAspectRatio(float aspectRatio);

private:
    Scene& _scene;

    RenderContext _context;
    float _axonTransparency = 0.2f;

private: // Shaders
    /** Renders cell morphologies as a series of lines */
    mv::ShaderProgram _lineShader;

    /** Renders cell soma as a dot */
    mv::ShaderProgram _somaShader;

    /** Renders electrophysiology sweeps */
    mv::ShaderProgram _traceShader;

private: // Objects
    GLuint _somaVAO = 0;

    RenderObjectBuilder _renderObjectBuilder;

    QHash<QString, CellRenderObject>    _cellRenderObjects;
    std::vector<CellRenderObject*>      _selectedCellRenderObjects;

private: // Render regions
    RenderRegion _fullViewport;
    RenderRegion _morphologyViewport;
    RenderRegion _traceViewport;
    float        _pixelRatio;

    bool _isCortical = false;

private: // Render bounds
    Range _stimChartRange;
    Range _acqChartRange;

private: // UI state
    StimulusType _currentStimType = StimulusType::Unknown;
    QStringList _enabledProcesses;
};

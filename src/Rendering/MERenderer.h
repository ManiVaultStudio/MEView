#pragma once

#include "Rendering/RenderObjectBuilder.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"

#include <QHash>
#include <QMatrix4x4>
#include <QOpenGLFunctions_3_3_Core>

class QPainter;

class MERenderer : public QObject, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    MERenderer();

    void Init();
    void Resize(int width, int height, float pixelRatio);
    void Update(float rotation, QPainter& painter);

    void SetCortical(bool cortical);
    void SetEnabledProcesses(const QStringList& enabledProcesses);
    void SetCurrentStimType(const QString& stimulusType);
    void SetAxonTransparency(float alpha) { _axonTransparency = alpha; }

    void BuildRenderObjects(const std::vector<Cell>& cells);
    void SetSelectedCellIds(const std::vector<uint32_t>& indices);

    void RenderLabels(QPainter& painter);
    void RenderSeparations(QPainter& painter);

    std::vector<float> GetHorizontalCellLocations() const;

signals:
    void RequestNewAspectRatio(float aspectRatio);

private:
    struct Range
    {
        float min = 0.0f;
        float max = 1.0f;
    };

    struct Band
    {
        int left = 0;
        int bottom = 0;
        int width = 1;
        int height = 1;
    };

    struct CellSlot
    {
        int cellIndex = -1;
        CellRenderObject* renderObject = nullptr;
        float leftPx = 0.0f;
        float centerPx = 0.0f;
        float rightPx = 0.0f;
    };

    void RebuildLayout();
    void RecalculateTraceRanges();
    void RequestWidgetWidth();

    std::vector<CellMorphology::Type> IgnoredMorphologyTypes() const;
    QMatrix4x4 ProjectionFor(const Band& band) const;

    void RenderMorphologies(float rotation);
    void RenderEphys();

    Scene& _scene;
    RenderObjectBuilder _renderObjectBuilder;

    QHash<QString, CellRenderObject> _cellRenderObjects;
    std::vector<CellSlot> _slots;
    std::vector<mv::Vector3f> _somaPositions;

    mv::ShaderProgram _lineShader;
    mv::ShaderProgram _somaShader;
    mv::ShaderProgram _traceShader;
    mv::ShaderProgram _texShader;
    mv::Texture2D _noSweepsTex;

    GLuint _somaVAO = 0;
    GLuint _imageVAO = 0;

    Band _morphologyBand;
    Band _traceBand;

    Range _stimulusRange;
    Range _acquisitionRange;

    int _widgetWidth = 1;
    int _widgetHeight = 1;

    float _morphologyReferenceHeight = 1.0f;
    float _contentRightPx = 0.0f;
    float _pixelRatio = 1.0f;
    float _axonTransparency = 0.2f;

    bool _isCortical = false;
    StimulusType _currentStimType = StimulusType::Unknown;
    QStringList _enabledProcesses;
};

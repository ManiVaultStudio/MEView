#pragma once

#include "Scene.h"

#include "Rendering/RenderState.h"
#include "Rendering/RenderObjectBuilder.h"
#include "Rendering/CellRenderObject.h"

#include "graphics/Shader.h"
#include "graphics/Vector3f.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>

class MorphologyLineSegments
{
public:
    std::vector<mv::Vector3f>   segments;
    std::vector<float>          segmentRadii;
    std::vector<int>            segmentTypes;
};

class EMRenderer : public QObject, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    EMRenderer() :
        _scene(Scene::getInstance()),
        _renderObjectBuilder(this, &_renderState)
    {

    }

    void init();
    void resize(int w, int h);
    void update(float t);

    void BuildRenderObjects(const std::vector<Cell>& cells);

public: // UI State
    void showAxons(bool enabled);
    void setCurrentStimset(const QString& stimset);

private:
    void buildRenderObject(const Cell& cell, CellRenderObject& cellRenderObject);
    void buildTraceRenderObject(TraceRenderObject& ro, const Recording& trace, bool isStim);
    //void Rebuild();
    void RebuildMorphologies();
    void RebuildTraces();

signals:
    void requestNewAspectRatio(float aspectRatio);

private:
    Scene& _scene;

    mv::ShaderProgram _lineShader;
    mv::ShaderProgram _traceShader;
    //mv::ShaderProgram _quadShader;

    QMatrix4x4 _projMatrix;
    QMatrix4x4 _viewMatrix;
    QMatrix4x4 _modelMatrix;

    int vx, vy, vw, vh;

    std::vector<CellRenderObject> _cellRenderObjects;

    float _stimChartRangeMin = -1;
    float _stimChartRangeMax = 1;

    float _acqChartRangeMin = -1;
    float _acqChartRangeMax = 1;

    RenderObjectBuilder _renderObjectBuilder;
    RenderState _renderState;

    // UI State
    bool _showAxons = true;
    QString _currentStimset = "";
};

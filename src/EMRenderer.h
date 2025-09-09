#pragma once

#include "Scene.h"

#include "Rendering/RenderState.h"
#include "Rendering/RenderObjectBuilder.h"
#include "Rendering/CellRenderObject.h"
#include "Rendering/RenderRegion.h"

#include "graphics/Shader.h"
#include "graphics/Vector3f.h"

#include <CellMorphologyData/CellMorphology.h>

#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QStringList>

class EMRenderer : public QObject, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    EMRenderer() :
        _scene(Scene::getInstance()),
        _renderObjectBuilder(this, &_renderState),
        _fullViewport(this),
        _morphologyViewport(this),
        _traceViewport(this)
    {

    }

    void init();
    void resize(int w, int h, float pixelRatio);
    void update(float t);

    void SetCortical(bool isCortical);
    void BuildRenderObjects(const std::vector<Cell>& cells);
    void SetSelectedCellIds(const std::vector<Cell>& cells);
    std::vector<float> GetHorizontalCellLocations();

public: // UI State
    void SetEnabledProcesses(const QStringList& enabledProcesses);
    void setCurrentStimset(const QString& stimset);

private:
    void BuildListOfCellRenderObjects(const std::vector<Cell>& cells, std::vector<CellRenderObject*>& cellRenderObjects);
    void RequestNewWidgetWidth();

signals:
    void requestNewAspectRatio(float aspectRatio);

private:
    Scene& _scene;

    mv::ShaderProgram _lineShader;
    mv::ShaderProgram _traceShader;
    //mv::ShaderProgram _quadShader;

    QMatrix4x4 _morphProjMatrix;
    QMatrix4x4 _traceProjMatrix;
    QMatrix4x4 _modelMatrix;

    bool _isCortical = false;

    std::vector<float> _horizontalCellLocations;

    RenderRegion _fullViewport;
    RenderRegion _morphologyViewport;
    RenderRegion _traceViewport;

    RenderObjectBuilder _renderObjectBuilder;
    RenderState _renderState;
    QStringList _enabledProcesses;

    // UI State
    QString _currentStimset = "";
};

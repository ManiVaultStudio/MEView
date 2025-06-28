#pragma once

#include "Scene.h"

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

class CellRenderObject
{
public:
    GLuint vao = 0; // Vertex array object
    GLuint vbo = 0; // Vertex buffer object
    GLuint rbo = 0; // Radius buffer object
    GLuint tbo = 0; // Type buffer object

    int numVertices = 0;

    mv::Vector3f ranges;
    float maxExtent = 0;
    mv::Vector3f anchorPoint;
    mv::Vector3f cellTypeColor;

    GLuint traceVAO = 0; // Trace vao
    GLuint traceVBO = 0;
    int numTraceVertices = 0;
};

class EMRenderer : public QObject, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    EMRenderer() :
        _scene(Scene::getInstance())
    {

    }

    void init();
    void resize(int w, int h);
    void update(float t);

    void showAxons(bool enabled);
    void buildRenderObjects(const std::vector<Cell>& cells);

private:
    void buildRenderObject(const Cell& cell, CellRenderObject& cellRenderObject);

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

    bool _showAxons = true;
};

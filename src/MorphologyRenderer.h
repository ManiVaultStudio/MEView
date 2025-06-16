#pragma once

#include "Scene.h"

#include "LRUCache.h"

#include "graphics/Shader.h"
#include "graphics/Vector3f.h"

#include <QOpenGLFunctions_3_3_Core>

#include <QMatrix4x4>
#include <QString>

class CellMorphology;

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
    mv::Vector3f somaPosition;
    mv::Vector3f cellTypeColor;
};

class MorphologyRenderer : protected QOpenGLFunctions_3_3_Core
{
public:
    MorphologyRenderer(Scene* scene) :
        _scene(scene),
        _aspectRatio(1),
        quadVao(0)
    {

    }

    virtual void init() = 0;
    void resize(int w, int h, int xMargin, int yMargin);
    void update(float t);

    virtual void render(int index, float t) = 0;

    void buildRenderObjects();

    int getNumRenderObjects() { return (int) _cellRenderObjects.size(); }

protected:
    virtual void buildRenderObject(const CellMorphology& cellMorphology, CellRenderObject& cellRenderObject) = 0;

protected:
    Scene* _scene;

    QMatrix4x4 _projMatrix;
    QMatrix4x4 _viewMatrix;
    QMatrix4x4 _modelMatrix;

    int vx, vy, vw, vh;

    float _aspectRatio;

    std::vector<CellRenderObject> _cellRenderObjects;

    unsigned int quadVao;
};

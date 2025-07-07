#pragma once

#include <graphics/Vector3f.h>

#include <QOpenGLFunctions_3_3_Core>

class CellMorphology;

struct MorphologyRenderObject
{
    GLuint vao = 0; // Vertex array object
    GLuint vbo = 0; // Buffer for vertices of the morphology
    GLuint rbo = 0; // Buffer for the radius at each vertex
    GLuint tbo = 0; // Buffer for the type of structure the vertex is a part of

    /* Number of vertices in the morphology */
    int numVertices = 0;

    /* Centerpoint of the morphology, around which it will rotate */
    mv::Vector3f anchorPoint;

    /* Dimensions of the morphology */
    mv::Vector3f dimensions = mv::Vector3f(1, 1, 1);
    /* The largest of the 3 dimensions */
    float maxExtent = 0;
};

struct TraceRenderObject
{
    GLuint vao = 0;
    GLuint vbo = 0;
    int numVertices = 0;
};

class CellRenderObject
{
public:
    void BuildMorphologyObject(const CellMorphology& cellMorphology, bool showAxons);

public:
    // Base color of the whole cell
    mv::Vector3f            cellTypeColor;

    // Sub-render objects
    MorphologyRenderObject  morphologyObject;
    TraceRenderObject       stimulusObject;
    TraceRenderObject       acquisitionObject;
};

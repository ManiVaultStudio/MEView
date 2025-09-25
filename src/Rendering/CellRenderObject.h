#pragma once

#include <graphics/Bounds.h>
#include <graphics/Vector3f.h>

#include <CellMorphologyData/CellMorphology.h>

#include <QOpenGLFunctions_3_3_Core>
#include <QHash>

class CellMorphology;

struct MorphologyProcessRenderObject
{
    GLuint vao = 0; // Vertex array object
    GLuint vbo = 0; // Buffer for vertices of the morphology
    GLuint rbo = 0; // Buffer for the radius at each vertex

    /* Number of vertices in the vbo */
    int numVertices = 0;

    /* Extents of the vertices in this object */
    CellMorphology::Extent extents;
};

class MorphologyRenderObject
{
public:
    void ComputeExtents(std::vector<CellMorphology::Type> ignoredTypes);

    QHash<CellMorphology::Type, MorphologyProcessRenderObject> processes;

    mv::Vector3f somaPosition;
    float somaRadius;

    CellMorphology::Extent totalExtent;

    ///* Centerpoint of the morphology, around which it will rotate */
    //mv::Vector3f _anchorPoint;
};

struct TraceRenderObject
{
    GLuint vao = 0;
    GLuint vbo = 0;
    int numVertices = 0;

    mv::Bounds extents;

    float priority;

    QString stimulusDescription;
};

class CellRenderObject
{
public:
    void Cleanup(QOpenGLFunctions_3_3_Core* f);

public:
    // Base color of the whole cell
    mv::Vector3f            cellTypeColor;

    // Sub-render objects
    MorphologyRenderObject  morphologyObject;

    std::vector<TraceRenderObject> stimulusObjects;
    std::vector<TraceRenderObject> acquisitionsObjects;

    bool hasMorphology = false;
};

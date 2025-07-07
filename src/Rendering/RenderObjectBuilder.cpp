#include "Rendering/RenderObjectBuilder.h"

#include <CellMorphologyData/CellMorphology.h>

#define ROB RenderObjectBuilder

namespace
{
    class MorphologyLineSegments
    {
    public:
        std::vector<mv::Vector3f>   segments;
        std::vector<float>          segmentRadii;
        std::vector<int>            segmentTypes;
    };
}

ROB::ROB(
    QOpenGLFunctions_3_3_Core* f,
    RenderState* renderState
) :
    _f(f),
    _renderState(renderState)
{

}

void ROB::BuildCellRenderObject(CellRenderObject& out, Cell& cell)
{

}

void ROB::BuildMorphologyObject(MorphologyRenderObject& mro, const CellMorphology& cellMorphology, bool showAxons)
{
    MorphologyLineSegments lineSegments;

    mv::Vector3f                somaPosition;
    float                       somaRadius;

    // Generate line segments
    try
    {
        for (int i = 0; i < cellMorphology.ids.size(); i++)
        {
            mv::Vector3f position = cellMorphology.positions.at(i);
            float radius = cellMorphology.radii.at(i);

            if (cellMorphology.types.at(i) == (int)CellMorphology::Type::Soma)
            {
                somaPosition = position;
                somaRadius = radius;
                break;
            }
        }

        for (int i = 1; i < cellMorphology.parents.size(); i++)
        {
            if (cellMorphology.parents[i] == -1) // New root found, there is no line segment here so skip it
                continue;

            int id = cellMorphology.idMap.at(cellMorphology.ids[i]);
            int parent = cellMorphology.idMap.at(cellMorphology.parents[i]);

            int type = cellMorphology.types[id];

            // Hide axons if so indicated
            if (!showAxons && type == (int)CellMorphology::Type::Axon)
                continue;

            float radius = cellMorphology.radii[id];

            lineSegments.segments.push_back(cellMorphology.positions[parent]);
            lineSegments.segments.push_back(cellMorphology.positions[id]);
            lineSegments.segmentRadii.push_back(radius);
            lineSegments.segmentRadii.push_back(radius);
            lineSegments.segmentTypes.push_back(type);
            lineSegments.segmentTypes.push_back(type);
        }
    }
    catch (std::out_of_range& oor)
    {
        qWarning() << "Out of range error in setCellMorphology(): " << oor.what();
        return;
    }

    // Initialize VAO and VBOs
    _f->glGenVertexArrays(1, &mro.vao);
    _f->glBindVertexArray(mro.vao);

    _f->glGenBuffers(1, &mro.vbo);
    _f->glBindBuffer(GL_ARRAY_BUFFER, mro.vbo);
    _f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    _f->glEnableVertexAttribArray(0);

    _f->glGenBuffers(1, &mro.rbo);
    _f->glBindBuffer(GL_ARRAY_BUFFER, mro.rbo);
    _f->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    _f->glEnableVertexAttribArray(1);

    _f->glGenBuffers(1, &mro.tbo);
    _f->glBindBuffer(GL_ARRAY_BUFFER, mro.tbo);
    _f->glVertexAttribIPointer(2, 1, GL_INT, 0, 0);
    _f->glEnableVertexAttribArray(2);

    // Store data on GPU
    _f->glBindVertexArray(mro.vao);

    _f->glBindBuffer(GL_ARRAY_BUFFER, mro.vbo);
    _f->glBufferData(GL_ARRAY_BUFFER, lineSegments.segments.size() * sizeof(mv::Vector3f), lineSegments.segments.data(), GL_STATIC_DRAW);
    qDebug() << "VBO size: " << (lineSegments.segments.size() * sizeof(mv::Vector3f)) / 1000 << "kb";
    _f->glBindBuffer(GL_ARRAY_BUFFER, mro.rbo);
    _f->glBufferData(GL_ARRAY_BUFFER, lineSegments.segmentRadii.size() * sizeof(float), lineSegments.segmentRadii.data(), GL_STATIC_DRAW);
    qDebug() << "RBO size: " << (lineSegments.segmentRadii.size() * sizeof(float)) / 1000 << "kb";
    _f->glBindBuffer(GL_ARRAY_BUFFER, mro.tbo);
    _f->glBufferData(GL_ARRAY_BUFFER, lineSegments.segmentTypes.size() * sizeof(int), lineSegments.segmentTypes.data(), GL_STATIC_DRAW);
    qDebug() << "TBO size: " << (lineSegments.segmentTypes.size() * sizeof(int)) / 1000 << "kb";

    mro.numVertices = (int)lineSegments.segments.size();

    if (showAxons)
        mro.anchorPoint = (cellMorphology.minRange + cellMorphology.maxRange) / 2;
    else
        mro.anchorPoint = (cellMorphology.noAxonMinRange + cellMorphology.noAxonMaxRange) / 2;

    mv::Vector3f range = showAxons ? (cellMorphology.maxRange - cellMorphology.minRange) : (cellMorphology.noAxonMaxRange - cellMorphology.noAxonMinRange);
    float maxExtent = std::max(std::max(range.x, range.y), range.z);
    mro.dimensions = range;
    mro.maxExtent = maxExtent;
}

void ROB::BuildTraceObject(TraceRenderObject& tro, const Recording& recording, bool isStim)
{
    // Generate line segments
    const TimeSeries& ts = recording.GetData();

    std::vector<Vector3f> vertices;
    for (int i = 0; i < ts.xSeries.size(); i++)
    {
        vertices.emplace_back(ts.xSeries[i], ts.ySeries[i], 0);
    }
    float xMin = std::numeric_limits<float>::max(), xMax = -std::numeric_limits<float>::max(), yMin = std::numeric_limits<float>::max(), yMax = -std::numeric_limits<float>::max();
    for (int i = 0; i < vertices.size(); i++)
    {
        Vector3f& v = vertices[i];
        if (v.x < xMin) xMin = v.x;
        if (v.x > xMax) xMax = v.x;
        if (v.y < yMin) yMin = v.y;
        if (v.y > yMax) yMax = v.y;
    }
    float xRange = xMax - xMin;
    float yRange = yMax - yMin;
    for (int i = 0; i < vertices.size(); i++)
    {
        vertices[i].x = (vertices[i].x - xMin) / xRange;
        if (isStim)
            vertices[i].y = (vertices[i].y - _renderState->_stimChartRangeMin) / (_renderState->_stimChartRangeMax - _renderState->_stimChartRangeMin);
        else
            vertices[i].y = (vertices[i].y - _renderState->_acqChartRangeMin) / (_renderState->_acqChartRangeMax - _renderState->_acqChartRangeMin);
    }

    // Initialize VAO and VBOs
    _f->glGenVertexArrays(1, &tro.vao);
    _f->glBindVertexArray(tro.vao);

    _f->glGenBuffers(1, &tro.vbo);
    _f->glBindBuffer(GL_ARRAY_BUFFER, tro.vbo);
    _f->glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vector3f), vertices.data(), GL_STATIC_DRAW);
    _f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    _f->glEnableVertexAttribArray(0);

    tro.numVertices = vertices.size();
}

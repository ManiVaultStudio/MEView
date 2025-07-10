#include "Rendering/RenderObjectBuilder.h"

#include "Rendering/RenderState.h"

#include <CellMorphologyData/CellMorphology.h>
#include <EphysData/EphysData.h>

namespace
{
    class MorphologyLineSegments
    {
    public:
        std::vector<mv::Vector3f>   segments;
        std::vector<float>          segmentRadii;
    };
}

RenderObjectBuilder::RenderObjectBuilder(
    QOpenGLFunctions_3_3_Core* f,
    RenderState* renderState
) :
    _f(f),
    _renderState(renderState)
{

}

void RenderObjectBuilder::BuildCellRenderObjects(const std::vector<Cell>& cells)
{
    for (const Cell& cell : cells)
    {
        CellRenderObject cro;
        BuildCellRenderObject(cro, cell);

        _renderState->_cellRenderObjects[cell.cellId] = cro;
    }
    qDebug() << "Built all render objects" << _renderState->_cellRenderObjects.size();
}

void RenderObjectBuilder::BuildCellRenderObject(CellRenderObject& cro, const Cell& cell)
{
    // Morphology
    if (cell.morphology != nullptr)
    {
        const CellMorphology& cellMorphology = *cell.morphology;

        BuildMorphologyObject(cro.morphologyObject, cellMorphology);

        cro.cellTypeColor = cellMorphology.cellTypeColor;
    }
    if (cell.ephysTraces != nullptr)
    {
        const Experiment& experiment = *cell.ephysTraces;

        if (experiment.getAcquisitions().empty() || experiment.getStimuli().empty())
            return;

        for (int i = 0; i < experiment.getStimuli().size(); i++)
        {
            const Recording& recording = experiment.getStimuli()[i];
            TraceRenderObject stimTRO;
            TraceRenderObject acqTRO;

            BuildTraceObject(stimTRO, experiment.getStimuli()[i], true);
            BuildTraceObject(acqTRO, experiment.getAcquisitions()[i], false);

            cro.stimulusObjects.push_back(stimTRO);
            cro.acquisitionsObjects.push_back(acqTRO);
        }
    }
}

void RenderObjectBuilder::BuildMorphologyObject(MorphologyRenderObject& mro, const CellMorphology& cellMorphology)
{
    bool showAxons = true; // FIXME Redo morphology storage

    QHash<CellMorphology::Type, MorphologyLineSegments> lineSegmentsHash;

    mv::Vector3f                somaPosition;
    float                       somaRadius;

    // Generate line segments
    try
    {
        // Find the soma position and radius
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

            int itype = cellMorphology.types[id];
            CellMorphology::Type type = CellMorphology::TypeFromInt(itype);

            // Hide axons if so indicated
            if (!showAxons && type == CellMorphology::Type::Axon)
                continue;

            float radius = cellMorphology.radii[id];

            MorphologyLineSegments& ls = lineSegmentsHash[type];

            ls.segments.push_back(cellMorphology.positions[parent]);
            ls.segments.push_back(cellMorphology.positions[id]);
            ls.segmentRadii.push_back(radius);
            ls.segmentRadii.push_back(radius);
        }
    }
    catch (std::out_of_range& oor)
    {
        qWarning() << "Out of range error in setCellMorphology(): " << oor.what();
        return;
    }


    for (auto it = lineSegmentsHash.begin(); it != lineSegmentsHash.end(); ++it)
    {
        CellMorphology::Type type = it.key();
        MorphologyLineSegments& ls = it.value();

        MorphologyProcessRenderObject& mpro = mro.processes[type];

        // Initialize VAO and VBOs
        _f->glGenVertexArrays(1, &mpro.vao);
        _f->glBindVertexArray(mpro.vao);

        _f->glGenBuffers(1, &mpro.vbo);
        _f->glBindBuffer(GL_ARRAY_BUFFER, mpro.vbo);
        _f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        _f->glEnableVertexAttribArray(0);

        _f->glGenBuffers(1, &mpro.rbo);
        _f->glBindBuffer(GL_ARRAY_BUFFER, mpro.rbo);
        _f->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
        _f->glEnableVertexAttribArray(1);

        // Store data on GPU
        _f->glBindVertexArray(mpro.vao);

        _f->glBindBuffer(GL_ARRAY_BUFFER, mpro.vbo);
        _f->glBufferData(GL_ARRAY_BUFFER, ls.segments.size() * sizeof(mv::Vector3f), ls.segments.data(), GL_STATIC_DRAW);
        //qDebug() << "VBO size: " << (ls.segments.size() * sizeof(mv::Vector3f)) / 1000 << "kb";
        _f->glBindBuffer(GL_ARRAY_BUFFER, mpro.rbo);
        _f->glBufferData(GL_ARRAY_BUFFER, ls.segmentRadii.size() * sizeof(float), ls.segmentRadii.data(), GL_STATIC_DRAW);
        //qDebug() << "RBO size: " << (ls.segmentRadii.size() * sizeof(float)) / 1000 << "kb";

        mpro.numVertices = (int)ls.segments.size();
        mpro.extents = cellMorphology.extents[type];
    }
    qDebug() << "Number of line segments in hash " << lineSegmentsHash.size();
}

void RenderObjectBuilder::BuildTraceObject(TraceRenderObject& tro, const Recording& recording, bool isStim)
{
    Bounds bounds(recording.GetData().xMin, recording.GetData().xMax, recording.GetData().yMin, recording.GetData().yMax);
    tro.extents = bounds;
    tro.stimulusDescription = recording.GetStimulusDescription();

    // Generate line segments
    const TimeSeries& ts = recording.GetData();

    std::vector<Vector3f> vertices;
    for (int i = 0; i < ts.xSeries.size(); i++)
    {
        vertices.emplace_back(ts.xSeries[i], ts.ySeries[i], 0);
    }

    //qDebug() << "Trace VBO size: " << (vertices.size() * sizeof(mv::Vector3f)) / 1000 << "kb";
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

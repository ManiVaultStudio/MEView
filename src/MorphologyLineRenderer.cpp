#include "MorphologyLineRenderer.h"

#include "CellMorphologyData/CellMorphology.h"

#define TEMP_MARGIN 1000

void MorphologyLineRenderer::init()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    loaded &= _lineShader.loadShaderFromFile(":me_viewer/shaders/PassThrough.vert", ":me_viewer/shaders/Lines.frag");
    loaded &= _quadShader.loadShaderFromFile(":me_viewer/shaders/Quad.vert", ":me_viewer/shaders/Quad.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }

    glEnable(GL_LINE_SMOOTH);
}

void MorphologyLineRenderer::render(float t)
{
    glViewport(vx, vy, vw, vh);

    //_quadShader.bind();
    //glBindVertexArray(quadVao);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //glClear(GL_DEPTH_BUFFER_BIT);

    float depthRange = _scene.getCortexStructure().getMaxDepth() - _scene.getCortexStructure().getMinDepth();

    _lineShader.bind();

    _viewMatrix.setToIdentity();
    _viewMatrix.scale(1.0f / depthRange);

    float xOffset = 0;
    qDebug() << "Rendering " << _cellRenderObjects.size() << " objects.";
    for (int i = 0; i < _cellRenderObjects.size(); i++)
    {
        CellRenderObject& cellRenderObject = _cellRenderObjects[i];

        mv::Vector3f somaPosition = cellRenderObject.somaPosition;

        float maxWidth = sqrtf(powf(cellRenderObject.ranges.x, 2) + powf(cellRenderObject.ranges.z, 2)) * 1.2f;
        
        //qDebug() << "YOffset" << yOffset;
        _modelMatrix.setToIdentity();
        _modelMatrix.translate(TEMP_MARGIN + xOffset, depthRange + cellRenderObject.somaPosition.y, 0);
        _modelMatrix.rotate(t, 0, 1, 0);
        _modelMatrix.translate(-somaPosition.x, -somaPosition.y, -somaPosition.z);

        _lineShader.uniformMatrix4f("projMatrix", _projMatrix.constData());
        _lineShader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());
        _lineShader.uniformMatrix4f("modelMatrix", _modelMatrix.constData());

        _lineShader.uniform3f("cellTypeColor", cellRenderObject.cellTypeColor);

        glBindVertexArray(cellRenderObject.vao);
        glDrawArrays(GL_LINES, 0, cellRenderObject.numVertices);
        glBindVertexArray(0);

        xOffset += 200;//_maxRowWidth;
    }

    _lineShader.release();
}

void MorphologyLineRenderer::showAxons(bool enabled)
{
    _showAxons = enabled;
}

void MorphologyLineRenderer::getCellMetadataLocations(std::vector<float>& locations)
{
    float depthRange = _scene.getCortexStructure().getMaxDepth() - _scene.getCortexStructure().getMinDepth();

    locations.resize(_cellRenderObjects.size());

    float xOffset = 0;
    for (int i = 0; i < locations.size(); i++)
    {
        CellRenderObject& cellRenderObject = _cellRenderObjects[i];
        float maxWidth = sqrtf(powf(cellRenderObject.ranges.x, 2) + powf(cellRenderObject.ranges.z, 2)) * 1.2f;

        QMatrix4x4 viewMatrix;
        viewMatrix.scale(1.0f / depthRange);
        QMatrix4x4 modelMatrix;
        modelMatrix.translate(TEMP_MARGIN + xOffset, 0, 0);
        QVector4D v = _projMatrix * viewMatrix * modelMatrix * QVector4D(0, 0, 0, 1);
        locations[i] = (v.x() / v.w()) * 0.5f + 0.5f;

        //xOffset += _maxRowWidth;
    }
}

void MorphologyLineRenderer::buildRenderObject(const CellMorphology& cellMorphology, CellRenderObject& cellRenderObject)
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

            if (cellMorphology.types.at(i) == (int) CellMorphology::Type::Soma)
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
            if (!_showAxons && type == (int)CellMorphology::Type::Axon)
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
    glGenVertexArrays(1, &cellRenderObject.vao);
    glBindVertexArray(cellRenderObject.vao);

    glGenBuffers(1, &cellRenderObject.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &cellRenderObject.rbo);
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.rbo);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &cellRenderObject.tbo);
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.tbo);
    glVertexAttribIPointer(2, 1, GL_INT, 0, 0);
    glEnableVertexAttribArray(2);

    // Store data on GPU
    glBindVertexArray(cellRenderObject.vao);

    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.vbo);
    glBufferData(GL_ARRAY_BUFFER, lineSegments.segments.size() * sizeof(mv::Vector3f), lineSegments.segments.data(), GL_STATIC_DRAW);
    qDebug() << "VBO size: " << (lineSegments.segments.size() * sizeof(mv::Vector3f)) / 1000 << "kb";
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.rbo);
    glBufferData(GL_ARRAY_BUFFER, lineSegments.segmentRadii.size() * sizeof(float), lineSegments.segmentRadii.data(), GL_STATIC_DRAW);
    qDebug() << "RBO size: " << (lineSegments.segmentRadii.size() * sizeof(float)) / 1000 << "kb";
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.tbo);
    glBufferData(GL_ARRAY_BUFFER, lineSegments.segmentTypes.size() * sizeof(int), lineSegments.segmentTypes.data(), GL_STATIC_DRAW);
    qDebug() << "TBO size: " << (lineSegments.segmentTypes.size() * sizeof(int)) / 1000 << "kb";

    cellRenderObject.numVertices = (int) lineSegments.segments.size();

    cellRenderObject.somaPosition = somaPosition;
    mv::Vector3f range = cellMorphology.maxRange - cellMorphology.minRange;
    float maxExtent = std::max(std::max(range.x, range.y), range.z);
    cellRenderObject.ranges = range;
    cellRenderObject.maxExtent = maxExtent;
    cellRenderObject.cellTypeColor = cellMorphology.cellTypeColor;
}

#include "MorphologyTubeRenderer.h"

#include "CellMorphologyData/CellMorphology.h"

#include "util/Math.h"

#include <QMatrix4x4>
#include <QVector3D>
#include <QtMath>

std::vector<mv::Vector3f> sphereVertices = {
    mv::Vector3f(0.000000f, -1.000000f, 0.000000f),
    mv::Vector3f(0.723607f, -0.447220f, 0.525725f),
    mv::Vector3f(-0.276388f, -0.447220f, 0.850649f),
    mv::Vector3f(-0.894426f, -0.447216f, 0.000000f),
    mv::Vector3f(-0.276388f, -0.447220f, -0.850649f),
    mv::Vector3f(0.723607f, -0.447220f, -0.525725f),
    mv::Vector3f(0.276388f, 0.447220f, 0.850649f),
    mv::Vector3f(-0.723607f, 0.447220f, 0.525725f),
    mv::Vector3f(-0.723607f, 0.447220f, -0.525725f),
    mv::Vector3f(0.276388f, 0.447220f, -0.850649f),
    mv::Vector3f(0.894426f, 0.447216f, 0.000000f),
    mv::Vector3f(0.000000f, 1.000000f, 0.000000f),
    mv::Vector3f(-0.162456f, -0.850654f, 0.499995f),
    mv::Vector3f(0.425323f, -0.850654f, 0.309011f),
    mv::Vector3f(0.262869f, -0.525738f, 0.809012f),
    mv::Vector3f(0.850648f, -0.525736f, 0.000000f),
    mv::Vector3f(0.425323f, -0.850654f, -0.309011f),
    mv::Vector3f(-0.525730f, -0.850652f, 0.000000f),
    mv::Vector3f(-0.688189f, -0.525736f, 0.499997f),
    mv::Vector3f(-0.162456f, -0.850654f, -0.499995f),
    mv::Vector3f(-0.688189f, -0.525736f, -0.499997f),
    mv::Vector3f(0.262869f, -0.525738f, -0.809012f),
    mv::Vector3f(0.951058f, 0.000000f, 0.309013f),
    mv::Vector3f(0.951058f, 0.000000f, -0.309013f),
    mv::Vector3f(0.000000f, 0.000000f, 1.000000f),
    mv::Vector3f(0.587786f, 0.000000f, 0.809017f),
    mv::Vector3f(-0.951058f, 0.000000f, 0.309013f),
    mv::Vector3f(-0.587786f, 0.000000f, 0.809017f),
    mv::Vector3f(-0.587786f, 0.000000f, -0.809017f),
    mv::Vector3f(-0.951058f, 0.000000f, -0.309013f),
    mv::Vector3f(0.587786f, 0.000000f, -0.809017f),
    mv::Vector3f(0.000000f, 0.000000f, -1.000000f),
    mv::Vector3f(0.688189f, 0.525736f, 0.499997f),
    mv::Vector3f(-0.262869f, 0.525738f, 0.809012f),
    mv::Vector3f(-0.850648f, 0.525736f, 0.000000f),
    mv::Vector3f(-0.262869f, 0.525738f, -0.809012f),
    mv::Vector3f(0.688189f, 0.525736f, -0.499997f),
    mv::Vector3f(0.162456f, 0.850654f, 0.499995f),
    mv::Vector3f(0.525730f, 0.850652f, 0.000000f),
    mv::Vector3f(-0.425323f, 0.850654f, 0.309011f),
    mv::Vector3f(-0.425323f, 0.850654f, -0.309011f),
    mv::Vector3f(0.162456f, 0.850654f, -0.499995f)
};

std::vector<int> sphereFaces = {
    1, 14, 13, 2, 14, 16, 1, 13, 18, 1, 18, 20, 1, 20, 17, 2, 16, 23, 3, 15, 25, 4, 19, 27, 5, 21, 29, 6, 22, 31, 2, 23, 26, 3, 25, 28, 4, 27, 30, 5, 29, 32, 6, 31, 24, 7, 33, 38, 8, 34, 40, 9, 35, 41, 10, 36, 42, 11, 37, 39, 39, 42, 12, 39, 37, 42, 37, 10, 42, 42, 41, 12, 42, 36, 41, 36, 9, 41, 41, 40, 12, 41, 35, 40, 35, 8, 40, 40, 38, 12, 40, 34, 38, 34, 7, 38, 38, 39, 12, 38, 33, 39, 33, 11, 39, 24, 37, 11, 24, 31, 37, 31, 10, 37, 32, 36, 10, 32, 29, 36, 29, 9, 36, 30, 35, 9, 30, 27, 35, 27, 8, 35, 28, 34, 8, 28, 25, 34, 25, 7, 34, 26, 33, 7, 26, 23, 33, 23, 11, 33, 31, 32, 10, 31, 22, 32, 22, 5, 32, 29, 30, 9, 29, 21, 30, 21, 4, 30, 27, 28, 8, 27, 19, 28, 19, 3, 28, 25, 26, 7, 25, 15, 26, 15, 2, 26, 23, 24, 11, 23, 16, 24, 16, 6, 24, 17, 22, 6, 17, 20, 22, 20, 5, 22, 20, 21, 5, 20, 18, 21, 18, 4, 21, 18, 19, 4, 18, 13, 19, 13, 3, 19, 16, 17, 6, 16, 14, 17, 14, 1, 17, 13, 15, 3, 13, 14, 15, 14, 2, 15
};

void MorphologyTubeRenderer::init()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    loaded &= _shader.loadShaderFromFile(":me_viewer/shaders/Model.vert", ":me_viewer/shaders/Tubes.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }
}

void MorphologyTubeRenderer::render(float t)
{
    glEnable(GL_DEPTH_TEST);

    CellRenderObject& cellRenderObject = _cellRenderObjects[0];

    _projMatrix.setToIdentity();
    mv::Vector3f somaPosition = cellRenderObject.somaPosition;
    float maxExtent = cellRenderObject.maxExtent / 1.5f;

    _projMatrix.ortho(-maxExtent * _aspectRatio, maxExtent * _aspectRatio, -maxExtent, maxExtent, -maxExtent, maxExtent);

    _viewMatrix.setToIdentity();
    _viewMatrix.rotate(t, 0, 1, 0);
    _viewMatrix.translate(-somaPosition.x, -somaPosition.y, -somaPosition.z);

    _shader.bind();
    _shader.uniformMatrix4f("projMatrix", _projMatrix.constData());
    _shader.uniformMatrix4f("viewMatrix", _viewMatrix.constData());

    glBindVertexArray(cellRenderObject.vao);
    glDrawArrays(GL_TRIANGLES, 0, cellRenderObject.numVertices);
    glBindVertexArray(0);

    _shader.release();
}

void MorphologyTubeRenderer::reloadShaders()
{
    // Load shaders
    bool loaded = true;
    loaded &= _shader.loadShaderFromFile(":shaders/Model.vert", ":shaders/Tubes.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }
}

void MorphologyTubeRenderer::buildRenderObject(const CellMorphology& cellMorphology, CellRenderObject& cellRenderObject)
{
    mv::Vector3f                somaPosition;
    float                       somaRadius;

    // Generate tube vertices
    std::vector<mv::Vector3f>   tubeVertices;
    int numVertices = 5;
    for (int i = 0; i < numVertices; i++)
    {
        int j = (i + 1) % numVertices;

        float degRoti = ((float)i / numVertices) * 360;
        float radRoti = qDegreesToRadians(degRoti);
        float degRotj = ((float)j / numVertices) * 360;
        float radRotj = qDegreesToRadians(degRotj);

        mv::Vector3f v0(cos(radRoti), 0, sin(radRoti));
        mv::Vector3f v1(cos(radRotj), 0, sin(radRotj));
        mv::Vector3f v2(cos(radRoti), 1, sin(radRoti));
        mv::Vector3f v3(cos(radRotj), 1, sin(radRotj));

        tubeVertices.push_back(v0);
        tubeVertices.push_back(v1);
        tubeVertices.push_back(v2);
        tubeVertices.push_back(v2);
        tubeVertices.push_back(v1);
        tubeVertices.push_back(v3);
    }

    for (int i = 0; i < tubeVertices.size(); i++)
    {
        std::cout << tubeVertices[i].str() << std::endl;
    }

    // Generate tube segments
    std::vector<mv::Vector3f> allTubeVertices;
    std::vector<int> types;

    try
    {
        for (int i = 0; i < cellMorphology.ids.size(); i++)
        {
            mv::Vector3f position = cellMorphology.positions.at(i);
            float radius = cellMorphology.radii.at(i);

            if (cellMorphology.types.at(i) == 1) // Soma
            {
                somaPosition = position;
                somaRadius = radius;
            }

            if (cellMorphology.parents[i] == -1) // New root found, there is no line segment here so skip it
                continue;

            int parentIdx = cellMorphology.idMap.at(cellMorphology.parents[i]);
            mv::Vector3f parentPosition = cellMorphology.positions[parentIdx];
            float parentType = cellMorphology.types[parentIdx];
            float parentRadius = cellMorphology.radii[parentIdx];
            if (parentType == 1)
                parentRadius *= 0.5f;

            std::vector<mv::Vector3f> transformedTubeVertices(tubeVertices.size());

            mv::Vector3f direction = (position - parentPosition);
            float length = direction.length();
            if (std::isinf(length))
            {
                std::cout << position.str() << "  " << parentPosition.str() << std::endl;
            }
            direction.set(direction.x / length, direction.y / length, direction.z / length);
            QVector3D qdir(direction.x, direction.y, direction.z);
            QVector3D y(0, 1, 0);

            float dot = QVector3D::dotProduct(y, qdir);
            float angle = acos(dot);
            QVector3D perp = QVector3D::crossProduct(y, qdir);
            if (dot == -1)
            {
                perp = QVector3D(1, 0, 0);
            }

            QMatrix4x4 T;
            T.translate(parentPosition.x, parentPosition.y, parentPosition.z);

            QMatrix4x4 R;
            R.rotate(qRadiansToDegrees(angle), perp);

            QMatrix4x4 Su;
            Su.scale(radius, length, radius);

            QMatrix4x4 Sl;
            Sl.scale(parentRadius, length, parentRadius);

            QMatrix4x4 Tru = T * R * Su;
            QMatrix4x4 Trl = T * R * Sl;

            for (int j = 0; j < transformedTubeVertices.size(); j++)
            {
                QVector3D v(tubeVertices[j].x, tubeVertices[j].y, tubeVertices[j].z);
                QVector3D tv;
                if (tubeVertices[j].y > 0.5)
                    tv = Tru.map(v);
                else
                    tv = Trl.map(v);
                transformedTubeVertices[j] = mv::Vector3f(tv.x(), tv.y(), tv.z());
                if (j == 0 && std::isnan(transformedTubeVertices[j].x)) qDebug() << "trans: " << Tru << T << R << Su;
            }

            allTubeVertices.insert(allTubeVertices.end(), transformedTubeVertices.begin(), transformedTubeVertices.end());
            std::vector<int> segmentTypes(transformedTubeVertices.size(), cellMorphology.types[parentIdx]);
            for (int j = 0; j < segmentTypes.size(); j++)
            {
                if (tubeVertices[j].y > 0.5)
                    segmentTypes[j] = cellMorphology.types[i];
            }
            types.insert(types.end(), segmentTypes.begin(), segmentTypes.end());
        }
    }
    catch (std::out_of_range& oor)
    {
        qWarning() << "Out of range error in setCellMorphology(): " << oor.what();
        return;
    }

    // Add soma sphere
    for (int i = 0; i < sphereFaces.size(); i++)
    {
        int vertexId = sphereFaces[i] - 1;

        mv::Vector3f v = sphereVertices[vertexId];
        v = v * somaRadius * 0.5f;
        v = v + mv::Vector3f(somaPosition);
        allTubeVertices.push_back(v);
        types.push_back(1);
    }

    // Initialize VAO and VBOs
    glGenVertexArrays(1, &cellRenderObject.vao);
    glBindVertexArray(cellRenderObject.vao);

    glGenBuffers(1, &cellRenderObject.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //glGenBuffers(1, &cellRenderObject.rbo);
    //glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.rbo);
    //glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    //glEnableVertexAttribArray(1);

    glGenBuffers(1, &cellRenderObject.tbo);
    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.tbo);
    glVertexAttribIPointer(2, 1, GL_INT, 0, 0);
    glEnableVertexAttribArray(2);

    // Store data on GPU
    glBindVertexArray(cellRenderObject.vao);

    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.vbo);
    glBufferData(GL_ARRAY_BUFFER, allTubeVertices.size() * sizeof(mv::Vector3f), allTubeVertices.data(), GL_STATIC_DRAW);

    //glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.rbo);
    //glBufferData(GL_ARRAY_BUFFER, segmentRadii.size() * sizeof(float), segmentRadii.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, cellRenderObject.tbo);
    glBufferData(GL_ARRAY_BUFFER, types.size() * sizeof(int), types.data(), GL_STATIC_DRAW);

    cellRenderObject.numVertices = (int) allTubeVertices.size();
    qDebug() << ">>>>>>>>>>>>>>>>>>>>> Num vertices: " << cellRenderObject.numVertices;

    cellRenderObject.somaPosition = somaPosition;
    mv::Vector3f range = cellMorphology.maxRange - cellMorphology.minRange;
    float maxExtent = std::max(std::max(range.x, range.y), range.z);
    cellRenderObject.maxExtent = maxExtent;
}

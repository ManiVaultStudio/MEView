#pragma once

#include "MorphologyRenderer.h"

class MorphologyLineSegments
{
public:
    std::vector<mv::Vector3f>   segments;
    std::vector<float>          segmentRadii;
    std::vector<int>            segmentTypes;
};

class MorphologyLineRenderer : public MorphologyRenderer
{
public:
    MorphologyLineRenderer(Scene* scene) :
        MorphologyRenderer(scene),
        _maxRowWidth(0),
        _cellCache(100)
    {

    }

    void init() override;
    //void update(float t) override;

    virtual void render(int index, float t) override;

    void showAxons(bool enabled);

    void setRowWidth(float rowWidth) { _maxRowWidth = rowWidth; }

    void getCellMetadataLocations(std::vector<float>& locations);

private:
    virtual void buildRenderObject(const CellMorphology& cellMorphology, CellRenderObject& cellRenderObject) override;

private:
    mv::ShaderProgram _lineShader;
    mv::ShaderProgram _quadShader;

    float _maxRowWidth; // Maximum width that can be filled by displayed cells, if exceeded, next cells are rendered below

    LRUCache<QString, CellRenderObject> _cellCache;

    bool _showAxons = true;
};

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
    MorphologyLineRenderer() :
        MorphologyRenderer(),
        _cellCache(100)
    {

    }

    void init() override;
    //void update(float t) override;

    virtual void render(float t) override;

    void showAxons(bool enabled);

    void getCellMetadataLocations(std::vector<float>& locations);

private:
    virtual void buildRenderObject(const CellMorphology& cellMorphology, CellRenderObject& cellRenderObject) override;

private:
    mv::ShaderProgram _lineShader;
    mv::ShaderProgram _quadShader;

    LRUCache<QString, CellRenderObject> _cellCache;

    bool _showAxons = true;
};

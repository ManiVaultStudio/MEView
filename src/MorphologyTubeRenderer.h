#pragma once

#include "MorphologyRenderer.h"

class MorphologyTubeRenderer : public MorphologyRenderer
{
public:
    MorphologyTubeRenderer() :
        MorphologyRenderer()
    {

    }

    void init() override;
    //void update(float t) override;

    virtual void render(float t) override;

    void reloadShaders();

private:
    virtual void buildRenderObject(const CellMorphology& cellMorphology, CellRenderObject& cellRenderObject) override;

private:
    mv::ShaderProgram _shader;
};

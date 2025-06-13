#pragma once

#include "MorphologyRenderer.h"

class MorphologyTubeRenderer : public MorphologyRenderer
{
public:
    MorphologyTubeRenderer(Scene* scene) :
        MorphologyRenderer(scene)
    {

    }

    void init() override;
    //void update(float t) override;

    virtual void render(int index, float t) override;

    void reloadShaders();

private:
    virtual void buildRenderObject(const CellMorphology& cellMorphology, CellRenderObject& cellRenderObject) override;

private:
    mv::ShaderProgram _shader;
};

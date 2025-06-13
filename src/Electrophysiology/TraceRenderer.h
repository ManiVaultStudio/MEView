#pragma once

#include "graphics/Shader.h"

#include <QOpenGLFunctions_3_3_Core>

class Scene;

class TraceRenderer : protected QOpenGLFunctions_3_3_Core
{
public:
    TraceRenderer(Scene* scene) :
        _scene(scene)
    {}

    void init();

    void resize(int w, int h);

    void render();

    //void reloadShaders();

private:
    Scene* _scene;

    mv::ShaderProgram _shader;
};

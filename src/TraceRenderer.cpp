#include "TraceRenderer.h"

void TraceRenderer::init()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    //loaded &= _lineShader.loadShaderFromFile(":me_viewer/shaders/PassThrough.vert", ":me_viewer/shaders/Lines.frag");
    //loaded &= _quadShader.loadShaderFromFile(":me_viewer/shaders/Quad.vert", ":me_viewer/shaders/Quad.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the morphology shaders";
    }

    glEnable(GL_LINE_SMOOTH);
}

void TraceRenderer::resize(int w, int h, int xMargin, int yMargin)
{

}

void TraceRenderer::update(float t)
{

}

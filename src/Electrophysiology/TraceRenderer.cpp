#include "TraceRenderer.h"

void TraceRenderer::init()
{
    initializeOpenGLFunctions();

    // Load shaders
    bool loaded = true;
    loaded &= _shader.loadShaderFromFile(":shaders/Trace.vert", ":shaders/Trace.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the trace rendering shaders";
    }

    glEnable(GL_LINE_SMOOTH);
}

void TraceRenderer::resize(int w, int h)
{
    glViewport(0, 0, w, h);
}

void TraceRenderer::render()
{

}

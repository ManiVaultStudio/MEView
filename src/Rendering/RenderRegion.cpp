#include "Rendering/RenderRegion.h"

RenderRegion::RenderRegion(QOpenGLFunctions_3_3_Core* f) :
    _f(f)
{

}

void RenderRegion::Set(int x, int y, int w, int h)
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
}

float RenderRegion::GetAspectRatio()
{
    return (float)_w / _h;
}

void RenderRegion::Begin()
{
    _f->glGetIntegerv(GL_VIEWPORT, prevViewport);

    _f->glViewport(_x, _y, _w, _h);
}

void RenderRegion::End()
{
    _f->glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

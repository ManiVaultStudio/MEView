#include "Rendering/RenderRegion.h"

#include <QMatrix4x4>

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

QVector4D RenderRegion::GetScreenCoordinates(QVector4D ndc)
{
    float hw = _w / 2.0f;
    float hh = _h / 2.0f;

    QMatrix4x4 M = QMatrix4x4(hw, 0, 0, _x + hw, 0, hh, 0, _y + hh, 0, 0, 1, 0, 0, 0, 0, 1);

    return M * ndc;
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

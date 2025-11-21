#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QVector4D>

namespace mv
{
    class Bounds;
}

class RenderRegion
{
public:
    RenderRegion(QOpenGLFunctions_3_3_Core* f);

    void Set(int x, int y, int w, int h);
    void Set(mv::Bounds bounds);

    int GetWidth() { return _w; }
    int GetHeight() { return _h; }
    float GetAspectRatio();

    QMatrix4x4& GetProjectionMatrix();
    QVector4D GetScreenCoordinates(QVector4D ndc);

    void Begin();
    void End();

private:
    QOpenGLFunctions_3_3_Core* _f;

    int prevViewport[4] = { 0, 0, 1, 1 };

    int _x = 0;
    int _y = 0;
    int _w = 1;
    int _h = 1;

    QMatrix4x4 _projMatrix;
};

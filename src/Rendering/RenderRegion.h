#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QVector4D>

class RenderRegion
{
public:
    RenderRegion(QOpenGLFunctions_3_3_Core* f);

    void Set(int x, int y, int w, int h);
    float GetAspectRatio();
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
};

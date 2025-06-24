#pragma once

#include <QOpenGLFunctions_3_3_Core>

class TraceRenderer : protected QOpenGLFunctions_3_3_Core
{
public:
    void init();
    void resize(int w, int h, int xMargin, int yMargin);
    void update(float t);

private:

};

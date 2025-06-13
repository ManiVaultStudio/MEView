#pragma once

#include "widgets/OpenGLWidget.h"

#include "Scene.h"

#include "TraceRenderer.h"

class EphysView;

class EphysWidget : public mv::gui::OpenGLWidget
{
    Q_OBJECT
public:
    EphysWidget(EphysView* plugin, Scene* scene);
    ~EphysWidget();

protected:
    void onWidgetInitialized()          override;
    void onWidgetResized(int w, int h)  override;
    void onWidgetRendered()             override;
    void onWidgetCleanup()              override;

    bool eventFilter(QObject* target, QEvent* event);

private:
    bool isInitialized = false;

    Scene* _scene;

    TraceRenderer _traceRenderer;
};

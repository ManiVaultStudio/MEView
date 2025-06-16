#pragma once

#include "Scene.h"

#include "widgets/OpenGLWidget.h"

#include "MorphologyLineRenderer.h"
#include "MorphologyTubeRenderer.h"

#include "LayerDrawing.h"

#include "graphics/Vector3f.h"

#include <vector>
#include <unordered_map>

class MEView;
class CellMorphology;

enum class RenderMode
{
    LINE, REAL
};

class MorphologyWidget : public mv::gui::OpenGLWidget
{
    Q_OBJECT
public:
    MorphologyWidget(MEView* plugin, Scene* scene);
    ~MorphologyWidget();

    void setRenderMode(RenderMode renderMode)
    {
        _renderMode = renderMode;
    }

    void showAxons(bool enabled);
    void setRowWidth(float rowWidth);
    void uploadMorphologies();

protected:
    virtual void onWidgetInitialized() override;
    virtual void onWidgetResized(int w, int h) override;
    virtual void onWidgetRendered() override;
    virtual void onWidgetCleanup() override;

    bool eventFilter(QObject* target, QEvent* event);

private:
    bool isInitialized = false;

    Scene* _scene;

    float t = 0;

    MorphologyLineRenderer _lineRenderer;
    MorphologyTubeRenderer _tubeRenderer;
    RenderMode _renderMode;

    LayerDrawing _layerDrawing;
};

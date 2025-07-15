#pragma once

#include "Scene.h"

#include "LayerDrawing.h"
#include "EMRenderer.h"

#include "widgets/OpenGLWidget.h"

#include <vector>

class MEWidget : public mv::gui::OpenGLWidget
{
public:
    MEWidget();

    EMRenderer& GetRenderer() { return _emRenderer; }

    /** Set the indices of which morphologies should be shown */
    void setCells(const std::vector<Cell>& cells);

    void setSelectedCells(const std::vector<Cell>& cells);

    void SetCortical(bool isCortical);
    void showAxons(bool enabled);

protected: // mv::gui::OpenGLWidget overrides
    virtual void onWidgetInitialized() override;
    virtual void onWidgetResized(int w, int h) override;
    virtual void onWidgetRendered() override;
    virtual void onWidgetCleanup() override;

public slots:
    void onNewAspectRatioRequested(float aspectRatio);

private:
    int _width, _height;

    std::vector<Cell> _cells;

    bool _isCortical;
    LayerDrawing _layerDrawing;
    EMRenderer _emRenderer;

    float t = 0;
};

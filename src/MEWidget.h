#pragma once

#include "Scene.h"
#include "MorphologyLineRenderer.h"

#include "widgets/OpenGLWidget.h"

#include <vector>

class MEWidget : public mv::gui::OpenGLWidget
{
public:
    MEWidget();

    /** Set the indices of which morphologies should be shown */
    void setCells(const std::vector<Cell>& cells);

protected: // mv::gui::OpenGLWidget overrides
    virtual void onWidgetInitialized() override;
    virtual void onWidgetResized(int w, int h) override;
    virtual void onWidgetRendered() override;
    virtual void onWidgetCleanup() override;

private:
    std::vector<Cell> _cells;

    MorphologyLineRenderer _lineRenderer;

    float t = 0;
};

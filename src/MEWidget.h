#pragma once

#include "Scene.h"

#include "EMRenderer.h"

#include "widgets/OpenGLWidget.h"

#include <vector>

class MEWidget : public mv::gui::OpenGLWidget
{
public:
    MEWidget();

    /** Set the indices of which morphologies should be shown */
    void setCells(const std::vector<Cell>& cells);

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

    EMRenderer _emRenderer;

    float t = 0;
};

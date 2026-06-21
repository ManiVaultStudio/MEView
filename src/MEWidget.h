#pragma once

#include "Scene.h"

#include "LayerDrawing.h"
#include "Rendering/MERenderer.h"
//#include "HoverPopup.h"
#include "RoundedPopup.h"
#include "CellCard/CellCard.h"

#include "widgets/OpenGLWidget.h"

#include <vector>

class MEWidget : public mv::gui::OpenGLWidget
{
public:
    MEWidget();

    MERenderer& GetRenderer() { return _meRenderer; }

    /** Set the indices of which morphologies should be shown */
    void setCells(const std::vector<Cell>& cells);

    void setSelectedCells(const std::vector<uint32_t>& indices);

    void SetCortical(bool isCortical);
    void SetAxonTransparency(float alpha) { _meRenderer.SetAxonTransparency(alpha); }

protected: // mv::gui::OpenGLWidget overrides
    virtual void onWidgetInitialized() override;
    virtual void onWidgetResized(int w, int h) override;
    virtual void onWidgetRendered() override;
    virtual void onWidgetCleanup() override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

public slots:
    void onNewAspectRatioRequested(float aspectRatio);

private:
    Scene& _scene;

    int _width, _height;

    bool _isCortical;
    LayerDrawing _layerDrawing;
    MERenderer _meRenderer;

    float t = 0;

    RoundedPopup* popup = nullptr;
    CellCard* cellCard = nullptr;
};

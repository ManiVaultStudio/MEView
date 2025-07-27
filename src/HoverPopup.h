#pragma once

#include "Scene.h"

#include "Electrophysiology/EphysWebWidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

class HoverPopup : public QWidget
{
public:
    HoverPopup(QWidget* parent = nullptr);

    void setCell(Cell& cell);

protected:
    void leaveEvent(QEvent* event) override;

private:
    QVBoxLayout* _layout;

    QLabel* _cellLabel;
    QLabel* _clusterLabel;
    EphysWebWidget* _ephysWebWidget;
};

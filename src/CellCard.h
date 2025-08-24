#pragma once

#include "Scene.h"

#include "Electrophysiology/EphysWebWidget.h"

#include <actions/IntegralAction.h>

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

class CellCard : public QWidget
{
public:
    CellCard();

    void SetCell(Cell& cell);

private:
    QVBoxLayout* _layout;

    QLabel* _cellLabel;
    QLabel* _clusterLabel;
    EphysWebWidget* _ephysWebWidget;
    mv::gui::IntegralAction* _spinnerWidget;
};

#pragma once

#include "Scene.h"

#include "CellCard/CellCardWidget.h"

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

    CellCardWidget* _widget;
    //mv::gui::IntegralAction* _spinnerWidget;
};

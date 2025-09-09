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

    CellCardWidget* _widget;
};

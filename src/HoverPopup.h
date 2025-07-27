#pragma once

#include "Scene.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

class HoverPopup : public QWidget
{
public:
    HoverPopup(QWidget* parent = nullptr) : QWidget(parent, Qt::ToolTip) {
        setAttribute(Qt::WA_ShowWithoutActivating);
        QLabel* label = new QLabel("This is a custom popup", this);
        _cellLabel = new QLabel();

        setFixedSize(400, 400);
        _layout = new QVBoxLayout(this);
        _layout->addWidget(label);
        _layout->addWidget(_cellLabel);
        setLayout(_layout);
    }

    void setCell(Cell& cell)
    {
        _cellLabel->setText("Cell: " + cell.cellId);
    }

private:
    QVBoxLayout* _layout;
    QLabel* _cellLabel;
};

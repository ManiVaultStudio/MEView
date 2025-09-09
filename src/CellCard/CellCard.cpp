#include "CellCard.h"

CellCard::CellCard()
{
    _widget = new CellCardWidget();
    _widget->setPage(":me_view/web/cellcard/cellcard.html", "qrc:/me_view/web/cellcard/");

    setObjectName("card");
    setStyleSheet("#card { background: white; border-radius: 12px; border: 1px solid #999; }");

    _layout = new QVBoxLayout(this);
    _layout->addWidget(_widget);
    setLayout(_layout);

    // Soft shadow
    //auto* shadow = new QGraphicsDropShadowEffect(this);
    //shadow->setBlurRadius(24);
    //shadow->setOffset(0, 6);
    //shadow->setColor(QColor(0, 0, 0, 90));
    //setGraphicsEffect(shadow);
}

void CellCard::SetCell(Cell& cell)
{
    qDebug() << "Set Cell" << cell.cellId;

    _widget->setCell(cell);
}

#include "RoundedPopup.h"

#include <QVBoxLayout>

RoundedPopup::RoundedPopup(QWidget* parent) :
    QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);

    setFocusPolicy(Qt::FocusPolicy::StrongFocus); // The widget accepts focus by both tabbing and clicking
    setFixedWidth(400);

    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(16, 16, 16, 16); // space for the card + shadow
}

void RoundedPopup::SetWidget(QWidget* widget)
{
    _layout->addWidget(widget);
    cardWidget = widget;
}

void RoundedPopup::paintEvent(QPaintEvent*)
{

}

void RoundedPopup::leaveEvent(QEvent* event)
{
    //hide();

    QWidget::leaveEvent(event);
}

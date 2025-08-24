#pragma once

#include <QWidget>

#include <QVBoxLayout>

class RoundedPopup : public QWidget
{
public:
    RoundedPopup(QWidget* parent = nullptr);

    void SetWidget(QWidget* widget);

protected:
    void leaveEvent(QEvent* event) override;

    void paintEvent(QPaintEvent*) override;

private:
    QVBoxLayout* _layout;

    QWidget* cardWidget = nullptr;
};

#include "HoverPopup.h"

#include <QPainterPath>

HoverPopup::HoverPopup(QWidget* parent) :
    QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    //setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus); // The widget accepts focus by both tabbing and clicking
    setFixedWidth(400);
    setStyleSheet("background-color: #FFF;");

    ////QLabel* label = new QLabel("This is a custom popup", this);
    //_cellLabel = new QLabel();
    //_clusterLabel = new QLabel();
    //_ephysWebWidget = new EphysWebWidget();
    //_ephysWebWidget->setPage(":me_viewer/ephys_viewer/trace_view.html", "qrc:/me_viewer/ephys_viewer/");

    //_spinnerWidget = new mv::gui::IntegralAction(this, "Sweep #", 0, 5, 0);

    ////QFrame* frame = new QFrame(this);

    //_layout = new QVBoxLayout(this);
    ////_layout->addWidget(label);
    ////_layout->addWidget(_cellLabel);
    ////_layout->addWidget(_clusterLabel);
    //_layout->addWidget(_ephysWebWidget);
    ////_layout->addWidget(_spinnerWidget->createWidget(this, mv::gui::IntegralAction::WidgetFlag::SpinBox));
    ////frame->setLayout(_layout);
    //setLayout(_layout);

    //QPainterPath path;
    //path.addRoundedRect(rect(), 20, 20); // 20px radius
    //setMask(QRegion(path.toFillPolygon().toPolygon()));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16); // space for the  + shadow

    //// inner  widget that well round & shadow
    //card = new QWidget(this);
    //card->setObjectName("card");
    //card->setStyleSheet("#card { background: white; border-radius: 12px; }");
    //auto* cardLayout = new QVBoxLayout(card);
    //cardLayout->addWidget(new QLabel("This is a smooth rounded popup", card));
    //auto* btn = new QPushButton("Close", card);
    //cardLayout->addWidget(btn);
    //layout->addWidget(card);

    //// soft shadow
    //auto* shadow = new QGraphicsDropShadowEffect(card);
    //shadow->setBlurRadius(24);
    //shadow->setOffset(0, 6);
    //shadow->setColor(QColor(0, 0, 0, 90));
    //card->setGraphicsEffect(shadow);
}

void HoverPopup::paintEvent(QPaintEvent*)
{

}

void HoverPopup::setCell(Cell& cell)
{
    _cellLabel->setText("Cell: " + cell.cellId);
    _clusterLabel->setText("Cluster: " + cell.cluster);

    if (cell.ephysTraces != nullptr)
    {
        const std::vector<Recording>& stimuli = cell.ephysTraces->getStimuli();

        std::vector<uint32_t> sweeps = cell.ephysTraces->getStimsetSweeps("X4PS_SupraThresh");

        std::sort(sweeps.begin(), sweeps.end(), [&](uint32_t a, uint32_t b) {
            return stimuli[a].GetSweepNumber() < stimuli[b].GetSweepNumber();
        });

        std::vector<Recording> recordings;
        for (int i = 0; i < sweeps.size(); i++)
        {
            qDebug() << "Sweep index: " << sweeps[i];
            int sweepNumber = cell.ephysTraces->getStimuli()[sweeps[i]].GetSweepNumber();
            auto attributes = cell.ephysTraces->getStimuli()[sweeps[i]].GetAttributes();
            qDebug() << "Cell ID: " << cell.cellId;
            qDebug() << "Sweep number: " << sweepNumber;
        }

        _ephysWebWidget->setNumSweeps(sweeps.size());
        //_ephysWebWidget->setData(cell, sweeps);
    }
}

void HoverPopup::leaveEvent(QEvent* event)
{
    //hide();

    QWidget::leaveEvent(event);
}

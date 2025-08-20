#include "HoverPopup.h"

HoverPopup::HoverPopup(QWidget* parent) :
    QWidget(parent, Qt::Popup)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus); // The widget accepts focus by both tabbing and clicking

    //QLabel* label = new QLabel("This is a custom popup", this);
    _cellLabel = new QLabel();
    _clusterLabel = new QLabel();
    _ephysWebWidget = new EphysWebWidget();
    _ephysWebWidget->setPage(":me_viewer/ephys_viewer/trace_view.html", "qrc:/me_viewer/ephys_viewer/");

    _spinnerWidget = new mv::gui::IntegralAction(this, "Sweep #", 0, 5, 0);

    
    setFixedSize(400, 500);
    _layout = new QVBoxLayout(this);
    //_layout->addWidget(label);
    _layout->addWidget(_cellLabel);
    _layout->addWidget(_clusterLabel);
    _layout->addWidget(_ephysWebWidget);
    //_layout->addWidget(_spinnerWidget->createWidget(this, mv::gui::IntegralAction::WidgetFlag::SpinBox));
    setLayout(_layout);
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
        _ephysWebWidget->setData(*cell.ephysTraces, sweeps);
    }
}

void HoverPopup::leaveEvent(QEvent* event)
{
    //hide();

    QWidget::leaveEvent(event);
}

#include "HoverPopup.h"

HoverPopup::HoverPopup(QWidget* parent) :
    QWidget(parent, Qt::ToolTip)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    QLabel* label = new QLabel("This is a custom popup", this);
    _cellLabel = new QLabel();
    _clusterLabel = new QLabel();
    _ephysWebWidget = new EphysWebWidget();
    _ephysWebWidget->setPage(":me_viewer/ephys_viewer/trace_view.html", "qrc:/me_viewer/ephys_viewer/");

    setFixedSize(400, 400);
    _layout = new QVBoxLayout(this);
    _layout->addWidget(label);
    _layout->addWidget(_cellLabel);
    _layout->addWidget(_clusterLabel);
    _layout->addWidget(_ephysWebWidget);
    setLayout(_layout);
}

void HoverPopup::setCell(Cell& cell)
{
    _cellLabel->setText("Cell: " + cell.cellId);
    _clusterLabel->setText("Cluster: " + cell.cluster);

    if (cell.ephysTraces != nullptr)
    {
        std::vector<Experiment> experiments;
        experiments.push_back(*cell.ephysTraces);
        _ephysWebWidget->setData(experiments, std::vector<uint32_t>{ 0 });
    }
}

void HoverPopup::leaveEvent(QEvent* event)
{
    hide();

    QWidget::leaveEvent(event);
}

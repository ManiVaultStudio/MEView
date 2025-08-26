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
    //_cellLabel->setText("Cell: " + cell.cellId);
    //_clusterLabel->setText("Cluster: " + cell.cluster);
    qDebug() << "Set Cell" << cell.cellId;

    //_widget->setNumSweeps(sweeps.size());
    _widget->setCell(cell);

    //if (cell.ephysTraces != nullptr)
    //{
    //    qDebug() << "Set Cell Ephys";
    //    //const std::vector<Recording>& stimuli = cell.ephysTraces->getStimuli();

    //    //std::vector<uint32_t> sweeps = cell.ephysTraces->getStimsetSweeps("X4PS_SupraThresh");

    //    //std::sort(sweeps.begin(), sweeps.end(), [&](uint32_t a, uint32_t b) {
    //    //    return stimuli[a].GetSweepNumber() < stimuli[b].GetSweepNumber();
    //    //});

    //    //std::vector<Recording> recordings;
    //    //for (int i = 0; i < sweeps.size(); i++)
    //    //{
    //    //    qDebug() << "Sweep index: " << sweeps[i];
    //    //    int sweepNumber = cell.ephysTraces->getStimuli()[sweeps[i]].GetSweepNumber();
    //    //    auto attributes = cell.ephysTraces->getStimuli()[sweeps[i]].GetAttributes();
    //    //    qDebug() << "Cell ID: " << cell.cellId;
    //    //    qDebug() << "Sweep number: " << sweepNumber;
    //    //}


    //}
}

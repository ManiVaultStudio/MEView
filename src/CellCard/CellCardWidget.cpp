#include "CellCardWidget.h"

#include "MEView.h"

#include "Scene.h"

#include <util/Timer.h>

#include <QLayout>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QHash>

#include <iostream>

QStringList newFormatStims = { "X1PS_SubThresh", "X3LP_Rheo", "X4PS_SupraThresh" };
QStringList oldFormatStims = { "C1LSFINEST150112", "C1LSCOARSE150216", "C1LSFINESTMICRO", "C1LSCOARSEMICRO" };

QStringList subStims = { "X1PS_SubThresh" };
QStringList includedStimsets = { "C1LSFINEST150112", "C1LSCOARSE150216", "C1LSFINESTMICRO", "C1LSCOARSEMICRO", "X3LP_Rheo", "X4PS_SupraThresh" };

/**
* JSON Structure
* 
* - cell
*    - cellId
*    - cluster
*    - ephys
*       - stimset
*       - bounds
*       - recordings[]
*           - sweepNumber
*           - stimulus
*               - xData[]
*               - yData[]
*           - acquisition
*               - xData[]
*               - yData[]
*/

namespace
{
    void addSweepToArray(QJsonArray& sweepArray, const Sweep& sweep, const ActionPotential* ap)
    {
        QJsonArray acqXData, acqYData, stimXData, stimYData;
        QJsonObject acquisitionObj, stimulusObj;

        for (float x : sweep.acquisition.GetData().xSeries)
            acqXData.append(x);
        for (float y : sweep.acquisition.GetData().ySeries)
            acqYData.append(y);
        for (float x : sweep.stimulus.GetRecording().GetData().xSeries)
            stimXData.append(x);
        for (float y : sweep.stimulus.GetRecording().GetData().ySeries)
            stimYData.append(y);

        acquisitionObj["xData"] = acqXData;
        acquisitionObj["yData"] = acqYData;
        stimulusObj["xData"] = stimXData;
        stimulusObj["yData"] = stimYData;

        QJsonObject sweepObj;
        sweepObj.insert("acquisition", acquisitionObj);
        sweepObj.insert("stimulus", stimulusObj);
        sweepObj.insert("sweepNumber", sweep.GetSweepNumber());
        //recordingObj.insert("title", acquisition.GetStimulusDescription());

        sweepArray.append(sweepObj);
    }
}

// =============================================================================
// JSCommunicationObject
// =============================================================================

JSCommunicationObject::JSCommunicationObject()
{

}

// =============================================================================
// EphysWebWidget
// =============================================================================

CellCardWidget::CellCardWidget() :
    _commObject(),
    _scene(Scene::getInstance())
{
    connect(this, &WebWidget::webPageFullyLoaded, this, &CellCardWidget::onWebPageFullyLoaded);
    qDebug() << "Connect to event";
    // For more info on drag&drop behavior, see the ExampleViewPlugin project
    setAcceptDrops(true);

    // Ensure linking to the resources defined in res/ephys_viewer_resources.qrc
    //Q_INIT_RESOURCE(ephys_viewer_resources);

    // ManiVault and Qt create a "QtBridge" object on the js side which represents _comObject
    // there, we can connect the signals qt_js_* and call the slots js_qt_* from our communication object
    init(&_commObject);

    setContentsMargins(0, 0, 0, 0);
    layout()->setContentsMargins(0, 0, 0, 0);

    setMinimumHeight(240);
}

CellCardWidget::~CellCardWidget()
{

}

void CellCardWidget::setNumSweeps(int numSweeps)
{
    //_commObject.setNumSweeps(numSweeps);
}

void CellCardWidget::setCell(const Cell& cell)
{
    Timer t("SetData");

    QJsonObject cellObj;
    cellObj["cellId"] = cell.cellId;
    cellObj["cellName"] = cell.cellName;
    cellObj["cluster"] = cell.cluster;
    //cellObj["title"] = "Long Square";

    if (cell.ephysTraces != nullptr)
    {
        const Experiment& experiment = *cell.ephysTraces;

        const std::vector<Sweep>& sweeps = experiment.GetSweeps();

        std::vector<uint32_t> stimSweeps = experiment.GetStimTypeSweeps(Scene::getInstance().GetCurrentStimType());
        qDebug() << "Get current stim type: " << ToString(Scene::getInstance().GetCurrentStimType());
        qDebug() << "Stim sweeps length: " << stimSweeps.size();
        std::sort(stimSweeps.begin(), stimSweeps.end(), [&](uint32_t a, uint32_t b) {
            return sweeps[a].GetSweepNumber() < sweeps[b].GetSweepNumber();
        });

        // Build list of sweeps that should be included in the cell's graph
        QJsonArray sweepArray;

        float axMin = std::numeric_limits<float>::max();
        float axMax = -std::numeric_limits<float>::max();
        float ayMin = std::numeric_limits<float>::max();
        float ayMax = -std::numeric_limits<float>::max();

        float sxMin = std::numeric_limits<float>::max();
        float sxMax = -std::numeric_limits<float>::max();
        float syMin = std::numeric_limits<float>::max();
        float syMax = -std::numeric_limits<float>::max();

        for (uint32_t sweepIndex : stimSweeps)
        {
            // Per cell get its acquisitions and stimuli and determine what to render
            const Sweep& sweep = experiment.GetSweeps()[sweepIndex];

            addSweepToArray(sweepArray, sweep, experiment.getActionPotential());

            if (sweep.acquisition.GetData().xMin < axMin) axMin = sweep.acquisition.GetData().xMin;
            if (sweep.acquisition.GetData().xMax > axMax) axMax = sweep.acquisition.GetData().xMax;
            if (sweep.acquisition.GetData().yMin < ayMin) ayMin = sweep.acquisition.GetData().yMin;
            if (sweep.acquisition.GetData().yMax > ayMax) ayMax = sweep.acquisition.GetData().yMax;

            if (sweep.stimulus.GetRecording().GetData().xMin < sxMin) sxMin = sweep.stimulus.GetRecording().GetData().xMin;
            if (sweep.stimulus.GetRecording().GetData().xMax > sxMax) sxMax = sweep.stimulus.GetRecording().GetData().xMax;
            if (sweep.stimulus.GetRecording().GetData().yMin < syMin) syMin = sweep.stimulus.GetRecording().GetData().yMin;
            if (sweep.stimulus.GetRecording().GetData().yMax > syMax) syMax = sweep.stimulus.GetRecording().GetData().yMax;
        }

        // Action potential
        const ActionPotential* ap = experiment.getActionPotential();
        QJsonObject actionPotentialObj;
        if (ap)
        {
            QJsonArray apXData, apYData;
            for (float x : ap->getTimeSeries())
                apXData.append(x);
            for (float y : ap->getVoltageSeries())
                apYData.append(y);

            actionPotentialObj["xData"] = apXData;
            actionPotentialObj["yData"] = apYData;
            actionPotentialObj["peakIndex"] = ap->getPeakIndex();
        }

        QJsonObject ephysObj;

        ephysObj["stimset"] = Scene::getInstance().GetCurrentStimset();
        ephysObj["recordings"] = recordingArray;
        if (ap) ephysObj["actionPotential"] = actionPotentialObj;

        // Store graph extents
        ephysObj["stimExtentX"] = QJsonArray{ sxMin, sxMax };
        ephysObj["stimExtentY"] = QJsonArray{ syMin, syMax };
        ephysObj["acqExtentX"] = QJsonArray{ axMin, axMax };
        ephysObj["acqExtentY"] = QJsonArray{ ayMin, ayMax };

        cellObj.insert("ephys", ephysObj);
    }

    QJsonObject rootObj;
    rootObj.insert("cell", cellObj);

    QJsonDocument doc(rootObj);
    QString strJson(doc.toJson(QJsonDocument::Indented));

    t.printElapsedTime("SetData", true);
    _commObject.setData(strJson);
}

void JSCommunicationObject::js_partitionHovered(const QString& data) {
    if (!data.isEmpty())
    {
        qDebug() << "PARTITION SIGNAL" << data;
        emit partitionHovered(data);
    }
}

void CellCardWidget::onWebPageFullyLoaded()
{
    qDebug() << "EphysWebWidget::onWebPageFullyLoaded: Web page completely loaded.";
    //emit webPageLoaded();

    qDebug() << "EphysWebWidget size: " << width() << height();
}

void CellCardWidget::onPartitionHovered(QString name)
{
    qDebug() << "You hovered over partition: " << name;
}

void CellCardWidget::resizeEvent(QResizeEvent* event)
{
    (void)event;
    //applyAspectRatio();
}

void CellCardWidget::applyAspectRatio()
{
    int w = this->width();
    int h = this->height();
    double aspect = static_cast<double>(h) / static_cast<double>(w);

    if (aspect < 1.0f)
    {
        int targetSize = std::max(w, h);
        setMinimumWidth(targetSize);
        setMinimumHeight(targetSize);
        setMaximumWidth(targetSize);
        setMaximumHeight(targetSize);
    }
}

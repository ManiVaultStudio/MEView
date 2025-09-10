#include "CellCardWidget.h"

#include "MEView.h"

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
    void addRecordingToArray(QJsonArray& recordingArray, const Recording& acquisition, const Recording& stimulus)
    {
        QJsonArray acqXData, acqYData, stimXData, stimYData;
        QJsonObject acquisitionObj, stimulusObj;

        for (float x : acquisition.GetData().xSeries)
            acqXData.append(x);
        for (float y : acquisition.GetData().ySeries)
            acqYData.append(y);
        for (float x : stimulus.GetData().xSeries)
            stimXData.append(x);
        for (float y : stimulus.GetData().ySeries)
            stimYData.append(y);

        acquisitionObj["xData"] = acqXData;
        acquisitionObj["yData"] = acqYData;
        stimulusObj["xData"] = stimXData;
        stimulusObj["yData"] = stimYData;

        if (stimulus.GetSweepNumber() != acquisition.GetSweepNumber())
            qWarning() << "Showing mismatching sweeps, stim: " << stimulus.GetSweepNumber() << " acq: " << acquisition.GetSweepNumber();

        QJsonObject recordingObj;
        recordingObj.insert("acquisition", acquisitionObj);
        recordingObj.insert("stimulus", stimulusObj);
        recordingObj.insert("sweepNumber", stimulus.GetSweepNumber());
        //recordingObj.insert("title", acquisition.GetStimulusDescription());

        recordingArray.append(recordingObj);
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

        const std::vector<Recording>& stimuli = experiment.getStimuli();

        std::vector<uint32_t> sweeps = experiment.getStimsetSweeps("X4PS_SupraThresh");

        std::sort(sweeps.begin(), sweeps.end(), [&](uint32_t a, uint32_t b) {
            return stimuli[a].GetSweepNumber() < stimuli[b].GetSweepNumber();
        });

        // Build list of recordings that should be included in the cell's graph
        QJsonArray recordingArray;

        float axMin = std::numeric_limits<float>::max();
        float axMax = -std::numeric_limits<float>::max();
        float ayMin = std::numeric_limits<float>::max();
        float ayMax = -std::numeric_limits<float>::max();

        float sxMin = std::numeric_limits<float>::max();
        float sxMax = -std::numeric_limits<float>::max();
        float syMin = std::numeric_limits<float>::max();
        float syMax = -std::numeric_limits<float>::max();

        for (uint32_t sweepIndex : sweeps)
        {
            // Per cell get its acquisitions and stimuli and determine what to render
            const Recording& stimulus = experiment.getStimuli()[sweepIndex];
            const Recording& acquisition = experiment.getAcquisitions()[sweepIndex];

            addRecordingToArray(recordingArray, acquisition, stimulus);

            if (acquisition.GetData().xMin < axMin) axMin = acquisition.GetData().xMin;
            if (acquisition.GetData().xMax > axMax) axMax = acquisition.GetData().xMax;
            if (acquisition.GetData().yMin < ayMin) ayMin = acquisition.GetData().yMin;
            if (acquisition.GetData().yMax > ayMax) ayMax = acquisition.GetData().yMax;

            if (stimulus.GetData().xMin < sxMin) sxMin = stimulus.GetData().xMin;
            if (stimulus.GetData().xMax > sxMax) sxMax = stimulus.GetData().xMax;
            if (stimulus.GetData().yMin < syMin) syMin = stimulus.GetData().yMin;
            if (stimulus.GetData().yMax > syMax) syMax = stimulus.GetData().yMax;
        }

        QJsonObject ephysObj;

        ephysObj["stimset"] = "X4PS_SupraThresh";
        ephysObj["recordings"] = recordingArray;

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

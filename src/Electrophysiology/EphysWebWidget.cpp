#include "EphysWebWidget.h"

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
QStringList supraStims = { "C1LSFINEST150112", "C1LSCOARSE150216", "C1LSFINESTMICRO", "C1LSCOARSEMICRO", "X3LP_Rheo", "X4PS_SupraThresh" };

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

        QJsonObject recordingObj;
        recordingObj.insert("acquisition", acquisitionObj);
        recordingObj.insert("stimulus", stimulusObj);
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

EphysWebWidget::EphysWebWidget(MEView* plugin, EphysScene* scene) :
    _commObject(),
    _scene(scene)
{
    connect(this, &WebWidget::webPageFullyLoaded, this, &EphysWebWidget::onWebPageFullyLoaded);
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

EphysWebWidget::~EphysWebWidget()
{

}

void EphysWebWidget::setData(std::vector<Experiment>& experiments, const std::vector<uint32_t>& selectionIndices)
{
    Timer t("SetData");

    std::vector<Recording> cellAcquisitions;
    std::vector<Recording> cellStimuli;

    QJsonArray graphArray;

    for (uint32_t index : selectionIndices)
    {
        // Per cell get its acquisitions and stimuli and determine what to render
        const std::vector<Recording>& acquisitions = experiments[index].getAcquisitions();
        const std::vector<Recording>& stimuli = experiments[index].getStimuli();

        if (acquisitions.size() != stimuli.size())
        {
            qWarning() << "[EphysViewer] Num acquisitions not same as num stimuli, skipping drawing..";
            return;
        }

        // Find out which stimsets to draw
        //bool include = false;
        //for (const Recording& acquisition : acquisitions)
        //{
        //    if (newFormatStims.contains(acquisition.GetStimulusDescription()))
        //        newFormat = true;
        //}

        // Build list of recordings that should be included in the cell's graph
        QJsonArray recordingArray;
        QStringList& includedStims = supraStims;

        float axMin = std::numeric_limits<float>::max();
        float axMax = -std::numeric_limits<float>::max();
        float ayMin = std::numeric_limits<float>::max();
        float ayMax = -std::numeric_limits<float>::max();

        float sxMin = std::numeric_limits<float>::max();
        float sxMax = -std::numeric_limits<float>::max();
        float syMin = std::numeric_limits<float>::max();
        float syMax = -std::numeric_limits<float>::max();

        std::vector<float> lengths(acquisitions.size(), 0);

        for (int i = 0; i < acquisitions.size(); i++)
        {
            const Recording& acquisition = acquisitions[i];
            for (int j = 0; j < acquisition.GetData().ySeries.size(); j++)
            {
                lengths[i] += acquisition.GetData().ySeries[j];
            }
            lengths[i] /= acquisition.GetData().xSeries.size();
            //lengths[i] = acquisition.GetData().xMax - acquisition.GetData().xMin;
            //lengths[i] = acquisition.GetData().xSeries.size();
        }

        std::vector<size_t> idx(lengths.size());
        iota(idx.begin(), idx.end(), 0);

        stable_sort(idx.begin(), idx.end(),
            [&lengths](size_t i1, size_t i2) {return lengths[i1] > lengths[i2]; });

        qDebug() << "Beep";
        qDebug() << lengths[0] << lengths[1];
        qDebug() << idx[0] << idx[1];

        for (int i = 0; i < acquisitions.size(); i++)
        {
            int index = idx[i];
            const Recording& acquisition = acquisitions[index];
            const Recording& stimulus = stimuli[index];

            if (includedStims.contains(acquisition.GetStimulusDescription()))
            {
                if (recordingArray.count() >= 3)
                    break;

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
        }

        QJsonObject graphObj;
        graphObj["title"] = "Long Square";
        graphObj["recordings"] = recordingArray;

        // Store graph extents
        graphObj["stimExtentX"] = QJsonArray{ sxMin, sxMax };
        graphObj["stimExtentY"] = QJsonArray{ syMin, syMax };
        graphObj["acqExtentX"] = QJsonArray{ axMin, axMax };
        graphObj["acqExtentY"] = QJsonArray{ ayMin, ayMax };

        graphArray.append(graphObj);
    }

    QJsonObject rootObj;
    rootObj.insert("graphs", graphArray);

    QJsonDocument doc(rootObj);
    QString strJson(doc.toJson(QJsonDocument::Indented));

    t.printElapsedTime("SetData", true);
    _commObject.setData(strJson);
}

void EphysWebWidget::setData(const std::vector<Recording>& acquisitions, const std::vector<Recording>& stimuli)
{
    Timer t("SetData");

    if (acquisitions.size() != stimuli.size())
    {
        qWarning() << "[EphysViewer] Num acquisitions not same as num stimuli, skipping drawing..";
        return;
    }
    qDebug() << "Number in setData" << acquisitions.size();
    QJsonArray graphArray;
    
    int numRecordings = acquisitions.size(); // Should be the same as stimuli.size()
    for (int i = 0; i < numRecordings; i++)
    {
        QJsonArray acqXData, acqYData, stimXData, stimYData;
        QJsonObject acquisitionObj, stimulusObj;

        const Recording& acquisition = acquisitions[i];
        const Recording& stimulus = stimuli[i];

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

        QJsonObject graphObj;
        graphObj.insert("acquisition", acquisitionObj);
        graphObj.insert("stimulus", stimulusObj);
        graphObj.insert("title", acquisition.GetStimulusDescription());

        graphArray.append(graphObj);
    }

    QJsonObject rootObj;
    rootObj.insert("graphs", graphArray);

    QJsonDocument doc(rootObj);
    QString strJson(doc.toJson(QJsonDocument::Indented));
    //std::cout << strJson.toStdString() << std::endl;

    t.printElapsedTime("SetData", true);
    _commObject.setData(strJson);
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>> Set Data >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
}

void JSCommunicationObject::js_partitionHovered(const QString& data) {
    if (!data.isEmpty())
    {
        qDebug() << "PARTITION SIGNAL" << data;
        emit partitionHovered(data);
    }
}

void EphysWebWidget::onWebPageFullyLoaded()
{
    qDebug() << "EphysWebWidget::onWebPageFullyLoaded: Web page completely loaded.";
    //emit webPageLoaded();

    qDebug() << "EphysWebWidget size: " << width() << height();
}

void EphysWebWidget::onPartitionHovered(QString name)
{
    qDebug() << "You hovered over partition: " << name;
}

void EphysWebWidget::resizeEvent(QResizeEvent* event)
{
    (void)event;
    //applyAspectRatio();
}

void EphysWebWidget::applyAspectRatio()
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

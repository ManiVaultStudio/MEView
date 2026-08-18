#include "CellCardSerializer.h"

#include "Scene.h"

#include <graphics/Bounds.h>
#include <util/Timer.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QHash>

/**
* JSON Structure
*
* - cell
*    - cellId
*    - cluster
*    - ephys
*       - stimtype
*       - bounds
*       - recordings[]
*           - sweepNumber
*           - stimulus
*               - xData[]
*               - yData[]
*               - stimAmplitude
*               - stimDesc
*           - acquisition
*               - xData[]
*               - yData[]
*/

namespace
{
    void AddSweepToArray(QJsonArray& sweepArray, const Sweep& sweep, const ActionPotential* ap)
    {
        QJsonArray acqXData, acqYData, stimXData, stimYData;
        QJsonObject acquisitionObj, stimulusObj;

        for (float x : sweep.acquisition.GetRecording().GetData().xSeries)
            acqXData.append(x);
        for (float y : sweep.acquisition.GetRecording().GetData().ySeries)
            acqYData.append(y);
        for (float x : sweep.stimulus.GetRecording().GetData().xSeries)
            stimXData.append(x);
        for (float y : sweep.stimulus.GetRecording().GetData().ySeries)
            stimYData.append(y);

        acquisitionObj["xData"] = acqXData;
        acquisitionObj["yData"] = acqYData;

        acquisitionObj["numSpikes"] = QString::number(sweep.GetSweepProperties().spikeIndices.size());

        stimulusObj["xData"] = stimXData;
        stimulusObj["yData"] = stimYData;
        stimulusObj["stimAmplitude"] = sweep.stimulus.GetStimulusAmplitude();
        stimulusObj["stimDesc"] = sweep.stimulus.GetStimulusDescription();

        QJsonObject sweepObj;
        sweepObj.insert("acquisition", acquisitionObj);
        sweepObj.insert("stimulus", stimulusObj);
        sweepObj.insert("sweepNumber", sweep.GetSweepNumber());
        //recordingObj.insert("title", acquisition.GetStimulusDescription());

        sweepArray.append(sweepObj);
    }
}

void CellCardSerializer::Serialize(const Cell& cell, QJsonDocument& outputDoc)
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

        Bounds acqBounds = Bounds::Max;
        Bounds stimBounds = Bounds::Max;

        for (uint32_t sweepIndex : stimSweeps)
        {
            // Per cell get its acquisitions and stimuli and determine what to render
            const Sweep& sweep = experiment.GetSweeps()[sweepIndex];

            AddSweepToArray(sweepArray, sweep, experiment.getActionPotential());

            const TimeSeries& acquisition = sweep.acquisition.GetRecording().GetData();
            const TimeSeries& stimulus = sweep.stimulus.GetRecording().GetData();

            if (acquisition.xMin < acqBounds.getLeft()) acqBounds.setLeft(acquisition.xMin);
            if (acquisition.xMax > acqBounds.getRight()) acqBounds.setRight(acquisition.xMax);
            if (acquisition.yMin < acqBounds.getBottom()) acqBounds.setBottom(acquisition.yMin);
            if (acquisition.yMax > acqBounds.getTop()) acqBounds.setTop(acquisition.yMax);

            if (stimulus.xMin < stimBounds.getLeft()) stimBounds.setLeft(stimulus.xMin);
            if (stimulus.xMax > stimBounds.getRight()) stimBounds.setRight(stimulus.xMax);
            if (stimulus.yMin < stimBounds.getBottom()) stimBounds.setBottom(stimulus.yMin);
            if (stimulus.yMax > stimBounds.getTop()) stimBounds.setTop(stimulus.yMax);
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

        StimulusType stimType = Scene::getInstance().GetCurrentStimType();
        ephysObj["stimtype"] = ToString(stimType);
        ephysObj["recordings"] = sweepArray;
        if (ap) ephysObj["actionPotential"] = actionPotentialObj;

        // Store graph extents
        ephysObj["stimExtentX"] = QJsonArray{ stimBounds.getLeft(), stimBounds.getRight() };
        ephysObj["stimExtentY"] = QJsonArray{ stimBounds.getBottom(), stimBounds.getTop() };
        ephysObj["acqExtentX"] = QJsonArray{ acqBounds.getLeft(), acqBounds.getRight() };
        ephysObj["acqExtentY"] = QJsonArray{ acqBounds.getBottom(), acqBounds.getTop() };

        cellObj.insert("ephys", ephysObj);
    }

    QJsonObject rootObj;
    rootObj.insert("cell", cellObj);

    outputDoc = QJsonDocument(rootObj);
    //QString strJson(outputDoc.toJson(QJsonDocument::Indented));

    t.printElapsedTime("SetData", true);
}

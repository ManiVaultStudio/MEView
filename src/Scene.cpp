#include "Scene.h"

#include "LayerDepthsReader.h"

namespace
{
    bool isMorphologicalData(mv::Dataset<DatasetImpl> dataset)
    {
        return dataset->hasProperty("PatchSeqType") && dataset->getProperty("PatchSeqType").toString() == "M";
    }

    bool isMorphologies(mv::Dataset<DatasetImpl> dataset)
    {
        return dataset->hasProperty("PatchSeqType") && dataset->getProperty("PatchSeqType").toString() == "Morphologies";
    }

    bool isEphysFeatures(mv::Dataset<DatasetImpl> dataset)
    {
        return dataset->hasProperty("PatchSeqType") && dataset->getProperty("PatchSeqType").toString() == "E";
    }

    bool isEphysTraces(mv::Dataset<DatasetImpl> dataset)
    {
        return dataset->hasProperty("PatchSeqType") && dataset->getProperty("PatchSeqType").toString() == "EphysTraces";
    }

    bool isMetadata(mv::Dataset<DatasetImpl> dataset)
    {
        return dataset->hasProperty("PatchSeqType") && dataset->getProperty("PatchSeqType").toString() == "Metadata";
    }
}

Scene::Scene() :
    _morphologyFeatureDataset(),
    _morphologyDataset(),
    _cellMetadataDataset()
{
    // Load average layer depth
    _cortexStructure = LayerDepthsReader::load(":/me_viewer/human_average_layer_depths.json");
}

bool Scene::hasAllRequiredDatasets(QStringList& missingDatasets)
{
    if (!_morphologyDataset.isValid())
        missingDatasets.push_back(".SWC Morphologies");
    if (!_morphologyFeatureDataset.isValid())
        missingDatasets.push_back("Morphology Features");
    if (!_ephysFeatures.isValid())
        missingDatasets.push_back("Ephys Features");
    if (!_ephysTraces.isValid())
        missingDatasets.push_back("Ephys Traces");
    if (!_cellMetadataDataset.isValid())
        missingDatasets.push_back("Cell Metadata");

    return missingDatasets.empty(); // true if empty
}

void Scene::offerCandidateDataset(Dataset<DatasetImpl> candidateDataset)
{
    if (isMorphologicalData(candidateDataset))
        _morphologyFeatureDataset = candidateDataset;
    else if (isMorphologies(candidateDataset))
        _morphologyDataset = candidateDataset;
    else if (isEphysFeatures(candidateDataset))
        _ephysFeatures = candidateDataset;
    else if (isEphysTraces(candidateDataset))
        _ephysTraces = candidateDataset;
    else if (isMetadata(candidateDataset))
        _cellMetadataDataset = candidateDataset;
    else
        return;

    QStringList missingDatasets;
    if (hasAllRequiredDatasets(missingDatasets))
    {
        emit allRequiredDatasetsLoaded();
    }
}

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

bool Scene::hasAllRequiredDatasets()
{
    if (!_morphologyDataset.isValid())
    {
        qWarning() << "[MEViewer] No cell morphology dataset found.";
        return false;
    }

    if (!_morphologyFeatureDataset.isValid())
    {
        qWarning() << "[MEViewer] No cell morphology feature dataset found.";
        return false;
    }

    if (!_ephysFeatures.isValid())
    {
        qWarning() << "[MEViewer] No electrophysiology feature dataset found.";
        return false;
    }

    if (!_ephysTraces.isValid())
    {
        qWarning() << "[MEViewer] No electrophysiology traces found.";
        return false;
    }

    if (!_cellMetadataDataset.isValid())
    {
        qWarning() << "[MEViewer] No cell metadata dataset set.";
        return false;
    }
}

void Scene::offerCandidateDataset(Dataset<DatasetImpl> candidateDataset)
{
    if (isMorphologicalData(candidateDataset))
        _morphologyFeatureDataset = candidateDataset;
    if (isMorphologies(candidateDataset))
        _morphologyDataset = candidateDataset;
    if (isEphysFeatures(candidateDataset))
        _ephysFeatures = candidateDataset;
    if (isEphysTraces(candidateDataset))
        _ephysTraces = candidateDataset;
    if (isMetadata(candidateDataset))
        _cellMetadataDataset = candidateDataset;
}

#pragma once

#include <PointData/PointData.h>
#include <TextData/TextData.h>
#include <CellMorphologyData/CellMorphologyData.h>

#include <QHash>
#include <QString>

class CortexStructure
{
public:
    CortexStructure() :
        _layerDepths(7, 0)
    {

    }

    float getLayerDepth(int layer) const { return _layerDepths[layer]; }
    float getMinDepth() const { return _layerDepths[0]; }
    float getMaxDepth() const { return _layerDepths[_layerDepths.size()-1]; }

    std::vector<float>  _layerDepths;
};

class Scene
{
public:
    Scene();

    bool hasAllRequiredDatasets();

    mv::Dataset<CellMorphologies>&      getMorphologyDataset()              { return _morphologyDataset; }
    mv::Dataset<Points>&                getMorphologyFeatureDataset()       { return _morphologyFeatureDataset; }
    mv::Dataset<Text>&                  getCellMetadataDataset()            { return _cellMetadataDataset; }
    const CortexStructure&              getCortexStructure()                { return _cortexStructure; }

    void offerCandidateDataset(mv::Dataset<mv::DatasetImpl> candidateDataset);

private:
    mv::Dataset<CellMorphologies>   _morphologyDataset;             /** Morphology data */
    mv::Dataset<Points>             _morphologyFeatureDataset;      /** Morphology feature data */
    mv::Dataset<Text>               _cellMetadataDataset;           /** Cell metadata */

    CortexStructure                 _cortexStructure;
};

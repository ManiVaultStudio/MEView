#pragma once

#include <PointData/PointData.h>
#include <TextData/TextData.h>
#include <CellMorphologyData/CellMorphologyData.h>
#include <EphysData/EphysData.h>

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QMatrix4x4>

class Cell
{
public:
    QString cellId;
    QString cluster;
    const CellMorphology* morphology;
    const Experiment* ephysTraces;
};

class CortexStructure
{
public:
    CortexStructure() :
        _layerDepths(7, 0)
    {

    }

    /**
     * Map cell from local coordinates to cortex coordinates with y:[-1, 1]
     */
    QMatrix4x4 mapCellToStructure(Vector3f somaPosition, Vector3f center) const;

    float getLayerDepth(int layer) const { return _layerDepths[layer]; }
    float getMinDepth() const { return _layerDepths[0]; }
    float getMaxDepth() const { return _layerDepths[_layerDepths.size()-1]; }
    float getDepthRange() const { return fabs(getMaxDepth() - getMinDepth()); }

    /**
     * Depths of the various layer thresholds, these are positive numbers (>0),
     * higher numbers are lower in the cortex.
     */
    std::vector<float>  _layerDepths;
};

class Scene : public QObject
{
    Q_OBJECT
public:
    static Scene& getInstance()
    {
        static Scene instance;
        return instance;
    }
private: // Private constructor
    Scene();
public: // Delete copy constructors
    Scene(Scene const&) = delete;
    void operator=(Scene const&) = delete;

signals:
    void allRequiredDatasetsLoaded();

public:
    bool hasAllRequiredDatasets(QStringList& missingDatasets);

    mv::Dataset<CellMorphologies>&      getMorphologyDataset()              { return _morphologyDataset; }
    mv::Dataset<Points>&                getMorphologyFeatureDataset()       { return _morphologyFeatureDataset; }
    const CortexStructure&              getCortexStructure()                { return _cortexStructure; }

    mv::Dataset<EphysExperiments>&   getEphysTraces() { return _ephysTraces; }

    mv::Dataset<Text>& getCellMetadataDataset() { return _cellMetadataDataset; }

    void offerCandidateDataset(mv::Dataset<mv::DatasetImpl> candidateDataset);

private: // Morphology
    mv::Dataset<CellMorphologies>   _morphologyDataset;             /** Morphology data */
    mv::Dataset<Points>             _morphologyFeatureDataset;      /** Morphology feature data */

    CortexStructure                 _cortexStructure;

private: // Ephys
    mv::Dataset<Points>             _ephysFeatures;                 /** Ephys feature data */
    mv::Dataset<EphysExperiments>   _ephysTraces;                   /** Ephys traces */

private: // Metadata
    mv::Dataset<Text>               _cellMetadataDataset;           /** Cell metadata */
};

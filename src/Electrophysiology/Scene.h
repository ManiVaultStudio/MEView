#pragma once

#include <PointData/PointData.h>
#include <TextData/TextData.h>
#include <EphysData/EphysData.h>

class EphysScene
{
public:
    EphysScene() :
        _ephysFeatures(),
        _ephysTraces(),
        _cellMetadata()
    {

    }

    mv::Dataset<EphysExperiments>   getEphysTraces() { return _ephysTraces; }
    mv::Dataset<Text>               getCellMetadata() { return _cellMetadata; }

private:
    mv::Dataset<Points>             _ephysFeatures;             /** Ephys feature data */
    mv::Dataset<EphysExperiments>   _ephysTraces;               /** Ephys traces */
    mv::Dataset<Text>               _cellMetadata;              /** Cell metadata */

    friend class MEView;
};

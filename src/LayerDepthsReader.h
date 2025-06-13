#pragma once

#include <QString>
#include <QHash>

class CortexStructure;

class LayerDepthsReader
{
public:
    static CortexStructure load(const QString& filePath);
private:

};

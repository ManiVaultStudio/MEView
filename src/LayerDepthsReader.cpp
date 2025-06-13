#include "LayerDepthsReader.h"

#include "Scene.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

CortexStructure LayerDepthsReader::load(const QString& filePath)
{
    CortexStructure cortexStructure;
    QHash<QString, float> hash;

    // Open the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[LayerDepthsReader] Could not open file:" << filePath;
        return cortexStructure;
    }

    // Read the JSON data
    QByteArray jsonData = file.readAll();
    file.close();

    // Parse JSON
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject()) {
        qWarning() << "[LayerDepthsReader] Invalid JSON format";
        return cortexStructure;
    }

    // Convert JSON object to QHash
    QJsonObject jsonObj = jsonDoc.object();
    for (const QString& key : jsonObj.keys()) {
        hash.insert(key, jsonObj.value(key).toDouble());
    }

    // Done with reading hash, now load depths into cortex structure class
    QStringList keys = { "1", "2", "3", "4", "5", "6", "wm" };
    for (int i = 0; i < cortexStructure._layerDepths.size(); i++)
    {
        QString key = keys[i];
        if (!hash.contains(key))
            qWarning() << "[LayerDepthsReader] Could not find layer with key: " << key;

        cortexStructure._layerDepths[i] = hash[key];
    }

    return cortexStructure;
}

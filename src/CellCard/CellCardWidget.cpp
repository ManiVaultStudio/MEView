#include "CellCardWidget.h"

#include "MEView.h"

#include "Scene.h"

#include <util/Timer.h>

#include <QLayout>

#include "CellCardSerializer.h"

#include <iostream>

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

    setMinimumHeight(520);
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
    CellCardSerializer serializer;
    QJsonDocument doc;
    serializer.Serialize(cell, doc);

    QString strDoc(doc.toJson(QJsonDocument::Indented));

    _commObject.setData(strDoc);
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

#pragma once

#include "EphysData/Experiment.h"

#include <widgets/WebWidget.h>

#include "Scene.h"

#include <vector>

class MEView;

class JSCommunicationObject : public mv::gui::WebCommunicationObject
{
    Q_OBJECT
public:
    JSCommunicationObject();

signals:
    // Signals from Qt to JS side
signals:
    void setData(QString data);
    void setFilterInJS(const QVariantList& data);
    void setHeaderOptions(const QVariantList& data);

public slots:
    // Invoked from JS side 
    void js_partitionHovered(const QString& data);

signals:
    // Signals from comm object to web widget
    void partitionHovered(QString name);

private:

};

class EphysWebWidget : public mv::gui::WebWidget
{
    Q_OBJECT
public:
    EphysWebWidget(MEView* plugin, EphysScene* scene);
    ~EphysWebWidget();

    JSCommunicationObject& getCommObject() { return _commObject; }

    void setData(std::vector<Experiment>& experiments, const std::vector<uint32_t>& selectionIndices);
    void setData(const std::vector<Recording>& acquisitions, const std::vector<Recording>& stimuli);

private slots:
    void onWebPageFullyLoaded();
    void onPartitionHovered(QString name);

protected:
    void resizeEvent(QResizeEvent* event);

public slots:
    void applyAspectRatio();

private:
    JSCommunicationObject   _commObject;    // Communication Object between Qt (cpp) and JavaScript

    EphysScene* _scene;
};

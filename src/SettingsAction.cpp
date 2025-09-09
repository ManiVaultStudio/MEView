#include "SettingsAction.h"

#include "MEView.h"

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _plugin(dynamic_cast<MEView*>(parent)),
    _lineRendererButton(this, "Line Renderer"),
    _realRendererButton(this, "True Renderer"),
    _processesOption(this, "Displayed processes", QStringList{ "Apical Dendrite", "Basal Dendrite", "Axon" }, QStringList{"Apical Dendrite", "Basal Dendrite", "Axon" }),
    _stimSetsAction(this, "Stim sets")
{

}

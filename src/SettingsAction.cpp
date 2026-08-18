#include "SettingsAction.h"

#include "MEView.h"

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    HorizontalGroupAction(parent, title),
    _plugin(dynamic_cast<MEView*>(parent)),
    _lineRendererButton(this, "Line Renderer"),
    _realRendererButton(this, "True Renderer"),
    _processesOption(this, "Displayed processes", QStringList{ "Apical Dendrite", "Basal Dendrite", "Axon" }, QStringList{"Apical Dendrite", "Basal Dendrite", "Axon" }),
    _stimSetsAction(this, "Stim sets"),
    _metadataAction(this, "Metadata coloring"),
    _axonTransparency(this, "Axon transparency", 0, 100, 20),
    _showCellsWithoutMorph(this, "Show all cells", false)
{

    _processesOption.setDefaultWidgetFlags(mv::gui::OptionsAction::ComboBox);
    addAction(&getProcessesOption());
    addAction(&getStimSetsAction());
    addAction(&GetMetadataAction());
    addAction(&GetAxonTransparency());
}

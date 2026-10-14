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
    _axonTransparency(this, "Axon transparency", 0, 100, 60),
    _showCellsWithoutMorph(this, "Show all cells", false)
{

    _processesOption.setDefaultWidgetFlags(mv::gui::OptionsAction::ComboBox);
    addAction(&getProcessesOption());
    addAction(&getStimSetsAction());
    addAction(&GetMetadataAction());
    addAction(&GetAxonTransparency());
}

void SettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _processesOption.fromParentVariantMap(variantMap);
    _stimSetsAction.fromParentVariantMap(variantMap);
    _metadataAction.fromParentVariantMap(variantMap);
    _axonTransparency.fromParentVariantMap(variantMap);
}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _processesOption.insertIntoVariantMap(variantMap);
    _stimSetsAction.insertIntoVariantMap(variantMap);
    _metadataAction.insertIntoVariantMap(variantMap);
    _axonTransparency.insertIntoVariantMap(variantMap);

    return variantMap;
}

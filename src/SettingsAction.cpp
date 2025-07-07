#include "SettingsAction.h"

#include "MEView.h"

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _plugin(dynamic_cast<MEView*>(parent)),
    _lineRendererButton(this, "Line Renderer"),
    _realRendererButton(this, "True Renderer"),
    _showAxons(this, "Show Axons", true),
    _stimSetsAction(this, "Stim sets")
{

}

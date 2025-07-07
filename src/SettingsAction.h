#pragma once

#include <actions/GroupAction.h>
#include <actions/TriggerAction.h>
#include <actions/ToggleAction.h>
#include <actions/OptionAction.h>

class MEView;

class SettingsAction : public mv::gui::GroupAction
{
public:
    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE SettingsAction(QObject* parent, const QString& title);

public: // Action getters
    mv::gui::TriggerAction& getLineRendererButton() { return _lineRendererButton; }
    mv::gui::TriggerAction& getRealRendererButton() { return _realRendererButton; }
    mv::gui::ToggleAction& getShowAxonsToggle() { return _showAxons; }
    mv::gui::OptionAction& getStimSetsAction() { return _stimSetsAction; }

private:
    MEView*     _plugin;

    mv::gui::TriggerAction  _lineRendererButton;
    mv::gui::TriggerAction  _realRendererButton;
    mv::gui::ToggleAction   _showAxons;
    mv::gui::OptionAction   _stimSetsAction;
};

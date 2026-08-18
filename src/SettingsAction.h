#pragma once

#include <actions/GroupAction.h>
#include <actions/HorizontalGroupAction.h>
#include <actions/TriggerAction.h>
#include <actions/ToggleAction.h>
#include <actions/OptionAction.h>
#include <actions/OptionsAction.h>
#include <actions/DecimalAction.h>

class MEView;

class SettingsAction : public mv::gui::HorizontalGroupAction
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

    mv::gui::OptionsAction& getProcessesOption() { return _processesOption; }
    mv::gui::OptionAction& getStimSetsAction() { return _stimSetsAction; }
    mv::gui::OptionAction& GetMetadataAction() { return _metadataAction; }
    mv::gui::DecimalAction& GetAxonTransparency() { return _axonTransparency; }
    mv::gui::ToggleAction& getShowNoMorphsAction() { return _showCellsWithoutMorph; }

private:
    MEView*     _plugin;

    mv::gui::TriggerAction  _lineRendererButton;
    mv::gui::TriggerAction  _realRendererButton;

    mv::gui::OptionsAction  _processesOption;
    mv::gui::OptionAction   _stimSetsAction;
    mv::gui::OptionAction   _metadataAction;
    mv::gui::DecimalAction  _axonTransparency;
    mv::gui::ToggleAction   _showCellsWithoutMorph;
};

#pragma once

#include "MEWidget.h"

#include "SettingsAction.h"

#include "NeuronDescriptor.h"

#include "Scene.h"
#include "Electrophysiology/EphysWebWidget.h"

#include <ViewPlugin.h>

#include <Dataset.h>
#include <widgets/DropWidget.h>
#include <actions/StringAction.h>
#include <actions/HorizontalToolbarAction.h>

#include <QWidget>

/** All plugin related classes are in the mv plugin namespace */
using namespace mv::plugin;

/** Drop widget used in this plugin is located in the mv gui namespace */
using namespace mv::gui;

/** Dataset reference used in this plugin is located in the mv util namespace */
using namespace mv::util;

class QLabel;

/**
 * Cell morphology plugin class
 *
 * @authors J. Thijssen
 */
class MEView : public ViewPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param factory Pointer to the plugin factory
     */
    MEView(const PluginFactory* factory);

    /** Destructor */
    ~MEView() override = default;
    
    /** This function is called by the core after the view plugin has been created */
    void init() override;

    void setStimulusSetOptions(const QSet<QString>& stimSets);

    /**
     * Invoked when a data event occurs
     * @param dataEvent Data event which occurred
     */
    void onDataEvent(mv::DatasetEvent* dataEvent);

private:
    void onCellSelectionChanged();

private:
    HorizontalToolbarAction         _primaryToolbarAction;      /** Horizontal toolbar for primary content */
    SettingsAction                  _settingsAction;
    DropWidget*                     _dropWidget;                /** Widget for drag and drop behavior */

    Scene&                          _scene;

    //MorphologyWidget*               _morphologyWidget;
    //EphysWebWidget*                 _ephysWidget;
    MEWidget*                       _meWidget;
};

/**
 * Example view plugin factory class
 *
 * Note: Factory does not need to be altered (merely responsible for generating new plugins when requested)
 */
class CellMorphologyPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "studio.manivault.MEView"
                      FILE  "MEView.json")

public:

    /** Default constructor */
    CellMorphologyPluginFactory() {}

    /** Destructor */
    ~CellMorphologyPluginFactory() override {}
    
    /** Creates an instance of the example view plugin */
    ViewPlugin* produce() override;

    /** Returns the data types that are supported by the example view plugin */
    mv::DataTypes supportedDataTypes() const override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};

#include "MEView.h"

#include "MorphologyDescription.h"

#include <event/Event.h>
#include <SelectionGroup.h>

#include "CellMorphologyData/CellMorphology.h"

#include <DatasetsMimeData.h>

#include <QDebug>
#include <QMimeData>
#include <QScrollArea>
#include <QScrollBar>
#include <fstream>
#include <sstream>

Q_PLUGIN_METADATA(IID "studio.manivault.MEView")

using namespace mv;

MEView::MEView(const PluginFactory* factory) :
    ViewPlugin(factory),
    _dropWidget(nullptr),
    _scene(Scene::getInstance()),
    _meWidget(new MEWidget()),
    _primaryToolbarAction(this, "PrimaryToolbar"),
    _settingsAction(this, "SettingsAction")
{

}

void MEView::init()
{
    // Create layout
    auto layout = new QVBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);

    // Both signals needed to kickstart
    connect(&_scene, &Scene::allRequiredDatasetsLoaded, this, &MEView::onInitialLoad);
    connect(_meWidget, &MEWidget::widgetInitialized, this, &MEView::onInitialLoad);

    _primaryToolbarAction.addAction(&_settingsAction.getProcessesOption());
    _primaryToolbarAction.addAction(&_settingsAction.getStimSetsAction());

    connect(&_settingsAction.getProcessesOption(), &OptionsAction::selectedOptionsChanged, this, [this](const QStringList& selectedOptions) { _meWidget->GetRenderer().SetEnabledProcesses(selectedOptions); });
    connect(&_settingsAction.getStimSetsAction(), &OptionAction::currentIndexChanged, this, [this](const int32_t& index) { _meWidget->GetRenderer().setCurrentStimset(_settingsAction.getStimSetsAction().getCurrentText()); });

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_primaryToolbarAction.createWidget(&getWidget()));

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->verticalScrollBar()->setEnabled(false);
    scrollArea->setWidget(_meWidget);
    scrollArea->setWidgetResizable(true);

    layout->addWidget(scrollArea, 99);

    // Apply the layout
    getWidget().setLayout(layout);

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_scene.getMorphologyDataset(), &Dataset<Text>::guiNameChanged, this, [this]()
    {
        // Only show the drop indicator when nothing is loaded in the dataset reference
        _dropWidget->setShowDropIndicator(_scene.getMorphologyDataset()->getGuiName().isEmpty());
    });

    connect(&_scene.getEphysTraces(), &Dataset<Text>::changed, this, [this]() { connect(&_scene.getEphysTraces(), &Dataset<Text>::dataSelectionChanged, this, &MEView::onCellSelectionChanged); });

    // Alternatively, classes which derive from hdsp::EventListener (all plugins do) can also respond to events
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetAdded));
    _eventListener.registerDataEventByType(PointType, std::bind(&MEView::onDataEvent, this, std::placeholders::_1));
    _eventListener.registerDataEventByType(TextType, std::bind(&MEView::onDataEvent, this, std::placeholders::_1));
    _eventListener.registerDataEventByType(CellMorphologyType, std::bind(&MEView::onDataEvent, this, std::placeholders::_1));
    _eventListener.registerDataEventByType(EphysType, std::bind(&MEView::onDataEvent, this, std::placeholders::_1));

    // Check if any usable datasets are already available, if so, use them
    for (mv::Dataset dataset : mv::data().getAllDatasets())
        _scene.offerCandidateDataset(dataset);
}

void MEView::onDataEvent(mv::DatasetEvent* dataEvent)
{
    // Get smart pointer to dataset that changed
    const auto changedDataSet = dataEvent->getDataset();

    // Get GUI name of the dataset that changed
    const auto datasetGuiName = changedDataSet->getGuiName();

    // The data event has a type so that we know what type of data event occurred (e.g. data added, changed, removed, renamed, selection changes)
    switch (dataEvent->getType()) {

        // A points dataset was added
        case EventType::DatasetAdded:
        {
            // Cast the data event to a data added event
            const auto dataAddedEvent = static_cast<DatasetAddedEvent*>(dataEvent);

            _scene.offerCandidateDataset(changedDataSet);

            // Get the GUI name of the added points dataset and print to the console
            qDebug() << datasetGuiName << "was added";

            break;
        }

        default:
            break;
    }
}

void MEView::setStimulusSetOptions(const QSet<QString>& stimSets)
{
    qDebug() << "setStimulusSetOptions" << stimSets.size();
    QStringList stimSetList = QStringList(stimSets.begin(), stimSets.end());
    _settingsAction.getStimSetsAction().setOptions(stimSetList);

    // Set initially on suprathresh
    for (int i = 0; i < stimSetList.size(); i++)
    {
        if (stimSetList[i] == "X4PS_SupraThresh")
        {
            _settingsAction.getStimSetsAction().setCurrentText(stimSetList[i]);
        }
    }
}

void MEView::onInitialLoad()
{
    qDebug() << "onInitialLoad";

    QStringList missingDatasets;
    if (_scene.hasAllRequiredDatasets(missingDatasets) && _meWidget->isWidgetInitialized())
    {
        // Find out which stimulus sets are available, and set them in the combobox
        mv::Dataset<EphysExperiments> ephysTraces = _scene.getEphysTraces();

        QSet<QString> stimSets;
        for (const Experiment& experiment : ephysTraces->getData())
        {
            const std::vector<Recording>& recordings = experiment.getStimuli();
            for (const Recording& recording : recordings)
                stimSets.insert(recording.GetStimulusDescription());
        }
        qDebug() << stimSets;
        setStimulusSetOptions(stimSets);

        // Compose all cells, send them to renderer for uploading to GPU
        // ...
        mv::Dataset<CellMorphologies> morphologyDataset = _scene.getMorphologyDataset();
        mv::Dataset<EphysExperiments> ephysTraceDataset = _scene.getEphysTraces();
        const std::vector<CellMorphology>& morphologies = morphologyDataset->getData();

        // Find common selected points
        std::vector<uint32_t> morphIndices(morphologies.size());
        std::iota(morphIndices.begin(), morphIndices.end(), 0);

        std::vector<int> ephysIndices;
        std::vector<int> metaIndices;

        for (mv::KeyBasedSelectionGroup& selGroup : mv::events().getSelectionGroups())
        {
            mv::BiMap& morphBiMap = selGroup.getBiMap(_scene.getMorphologyDataset());
            mv::BiMap& ephysBiMap = selGroup.getBiMap(_scene.getEphysTraces());
            mv::BiMap& metaBiMap = selGroup.getBiMap(_scene.getCellMetadataDataset());

            std::vector<QString> keys = morphBiMap.getKeysByValues(morphIndices);

            ephysIndices = ephysBiMap.getValuesByKeysWithMissingValue(keys, -1);
            metaIndices = metaBiMap.getValuesByKeysWithMissingValue(keys, -1);
        }

        auto& cellIdColumn = _scene.getCellMetadataDataset()->getColumn("Cell ID");

        // Construct vector of cells containing all necessary information
        std::vector<Cell> cells(morphIndices.size());
        for (int i = 0; i < cells.size(); i++)
        {
            Cell& cell = cells[i];
            cell.morphology = &morphologies[i];

            int ephysIndex = ephysIndices[i];
            cell.ephysTraces = ephysIndex >= 0 ? &_scene.getEphysTraces()->getData()[ephysIndex] : nullptr;

            int metaIndex = metaIndices[i];
            cell.cellId = metaIndex >= 0 ? cellIdColumn[metaIndex] : "Missing";
        }

        _meWidget->setCells(cells);
    }
}

void MEView::onCellSelectionChanged()
{
    // Check if the morphology, features and metadata datasets are loaded
    QStringList missingDatasets;
    if (!_scene.hasAllRequiredDatasets(missingDatasets))
    {
        qWarning() << "[MEViewer] Missing datasets:" << missingDatasets;
        return;
    }

    // Find appropriate selection indices in the morphology and ephys data
    mv::Dataset<CellMorphologies> morphologyDataset = _scene.getMorphologyDataset();
    const std::vector<CellMorphology>& morphologies = morphologyDataset->getData();

    const auto& selectionIndices = morphologyDataset->getSelectionIndices();
    std::vector<uint32_t> sortedSelectionIndices = selectionIndices;

    // Reorder selection based on soma depth
    std::sort(sortedSelectionIndices.begin(), sortedSelectionIndices.end(), [&morphologies](const uint32_t& a, const uint32_t& b)
    {
        return morphologies[a].somaPosition.y > morphologies[b].somaPosition.y;
    });

    // Find common selected points
    std::vector<int> ephysSelectionIndices;
    std::vector<int> metaSelectionIndices;

    for (mv::KeyBasedSelectionGroup& selGroup : mv::events().getSelectionGroups())
    {
        mv::BiMap& morphBiMap = selGroup.getBiMap(_scene.getMorphologyDataset());
        mv::BiMap& ephysBiMap = selGroup.getBiMap(_scene.getEphysTraces());
        mv::BiMap& metaBiMap = selGroup.getBiMap(_scene.getCellMetadataDataset());

        std::vector<QString> keys = morphBiMap.getKeysByValues(sortedSelectionIndices);

        ephysSelectionIndices = ephysBiMap.getValuesByKeysWithMissingValue(keys, -1);
        metaSelectionIndices = metaBiMap.getValuesByKeysWithMissingValue(keys, -1);
    }

    auto& cellIdColumn = _scene.getCellMetadataDataset()->getColumn("Cell ID");

    // FIXME
    QString columnName = _scene.getCellMetadataDataset()->hasColumn("Cluster") ? "Cluster" : "Group";
    auto& clusterColumn =  _scene.getCellMetadataDataset()->getColumn(columnName);

    // Construct vector of cells containing all necessary information
    std::vector<Cell> cells(sortedSelectionIndices.size());
    for (int i = 0; i < cells.size(); i++)
    {
        Cell& cell = cells[i];
        cell.morphology = &morphologies[sortedSelectionIndices[i]];

        int ephysIndex = ephysSelectionIndices[i];
        cell.ephysTraces = ephysIndex >= 0 ? &_scene.getEphysTraces()->getData()[ephysIndex] : nullptr;

        int metaIndex = metaSelectionIndices[i];
        cell.cellId = metaIndex >= 0 ? cellIdColumn[metaSelectionIndices[i]] : "Missing";
        cell.cluster = metaIndex >= 0 ? clusterColumn[metaSelectionIndices[i]] : "Missing";
    }

    bool isCortical = false;
    if (_scene.getMorphologyDataset()->hasProperty("isCortical"))
        isCortical = true;

    _meWidget->SetCortical(isCortical);
    _meWidget->setSelectedCells(cells);
}

ViewPlugin* CellMorphologyPluginFactory::produce()
{
    return new MEView(this);
}

mv::DataTypes CellMorphologyPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;

    // This example analysis plugin is compatible with points datasets
    supportedTypes.append(TextType);

    return supportedTypes;
}

mv::gui::PluginTriggerActions CellMorphologyPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this]() -> MEView* {
        return dynamic_cast<MEView*>(plugins().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    if (numberOfDatasets >= 1 && PluginFactory::areAllDatasetsOfTheSameType(datasets, TextType)) {
        auto pluginTriggerAction = new PluginTriggerAction(const_cast<CellMorphologyPluginFactory*>(this), this, "Cell Morphology", "View cell morphologies", icon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
            for (auto dataset : datasets)
                getPluginInstance();
        });

        pluginTriggerActions << pluginTriggerAction;
    }

    return pluginTriggerActions;
}

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
    //_morphologyWidget(new MorphologyWidget(this)),
    //_ephysWidget(new EphysWebWidget(this)),
    _meWidget(new MEWidget()),
    _primaryToolbarAction(this, "PrimaryToolbar"),
    _settingsAction(this, "SettingsAction")
{
    // Notice when the cell morphologies dataset changes, so that we can connect to its selection changes
    //connect(&_scene.getMorphologyDataset(), &Dataset<CellMorphologies>::changed, this, [this]() {
    //    if (_scene.getMorphologyDataset().isValid())
    //        connect(&_scene.getMorphologyDataset(), &Dataset<CellMorphologies>::dataSelectionChanged, this, &MEView::onCellSelectionChanged);
    //});
}

void MEView::init()
{
    // Create layout
    auto layout = new QVBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);

    _primaryToolbarAction.addAction(&_settingsAction.getLineRendererButton());
    //_primaryToolbarAction.addAction(&_settingsAction.getRealRendererButton());
    _primaryToolbarAction.addAction(&_settingsAction.getShowAxonsToggle());

    //connect(&_settingsAction.getLineRendererButton(), &TriggerAction::triggered, this, [this]() { _morphologyWidget->setRenderMode(RenderMode::LINE); });
    //connect(&_settingsAction.getRealRendererButton(), &TriggerAction::triggered, this, [this]() { _morphologyWidget->setRenderMode(RenderMode::REAL); });
    connect(&_settingsAction.getShowAxonsToggle(), &ToggleAction::toggled, this, [this](bool toggled) { _meWidget->showAxons(toggled); });

    // Load webpage
    //_ephysWidget->setPage(":me_viewer/ephys_viewer/trace_view.html", "qrc:/me_viewer/ephys_viewer/");

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

void MEView::onCellSelectionChanged()
{
    // Check if the morphology, features and metadata datasets are loaded
    if (!_scene.hasAllRequiredDatasets())
        return;

    //// Compute maximum width of the visualization, i.e. how many cells can we fit on average
    //float totalWidth = 0;
    //for (CellMorphology& cellMorphology : _scene.getMorphologyDataset()->getData())
    //{
    //    Vector3f range = cellMorphology.maxRange - cellMorphology.minRange;

    //    float maxWidth = sqrtf(range.x * range.x + range.z * range.z);

    //    totalWidth += maxWidth;
    //}
    //float averageWidth = (totalWidth / _scene.getMorphologyDataset()->getData().size()) * 1.5f;

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
        cell.ephysTraces = ephysIndex > 0 ? &_scene.getEphysTraces()->getData()[ephysIndex] : nullptr;

        int metaIndex = metaSelectionIndices[i];
        cell.cellId = metaIndex > 0 ? cellIdColumn[metaSelectionIndices[i]] : "Missing";
        cell.cluster = metaIndex > 0 ? clusterColumn[metaSelectionIndices[i]] : "Missing";
    }

    _meWidget->setCells(cells);

    //// Upload cell morphologies
    //_morphologyWidget->uploadMorphologies();

    //_ephysWidget->setData(_scene.getEphysTraces()->getData(), ephysSelectionIndices);
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

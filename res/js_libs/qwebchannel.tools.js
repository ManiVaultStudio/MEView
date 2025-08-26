var isQtAvailable = false;

// Here, we establish the connection to Qt
// Any signals that we want to send from ManiVault to
// the JS side have to be connected here
try {
    new QWebChannel(qt.webChannelTransport, function (channel) {
        // Establish connection
        QtBridge = channel.objects.QtBridge;

        // register signals
        QtBridge.setData.connect(function () {
            const jsonDoc = JSON.parse(arguments[0]);
            
            if ("cell" in jsonDoc)
            {
                let cellObj = jsonDoc["cell"];
                
                setSweepOptions(cellObj);
                drawCellCard(cellObj);
            }
        });
        QtBridge.setFilterInJS.connect(function () { drawChart(arguments[0]); });
        QtBridge.setHeaderOptions.connect(function () { setHeaderOptions(arguments[0]); });
        QtBridge.setNumSweeps.connect(function () { setNumSweeps(arguments[0]); });

        // confirm successful connection
        isQtAvailable = true;
        notifyBridgeAvailable();
    });
} catch (error) {
    log("CellCard: qwebchannel: could not connect qt");
}

// The slot js_available is defined by ManiVault's WebWidget and will
// invoke the initWebPage function of our web widget (here, ChartWidget)
function notifyBridgeAvailable() {
    if (isQtAvailable) {
        QtBridge.js_available();
    }
    else {
        log("CellCard: qwebchannel: QtBridge is not available - something went wrong");
    }
}

function partitionHovered(name)
{
    log("partitionHovered");
    if (isQtAvailable) {
        QtBridge.js_partitionHovered(name);
    }
}

// The QtBridge is connected to our WebCommunicationObject (ChartCommObject)
// and we can call all slots defined therein

// function onJsFilterChanged(data) {
    // if (isQtAvailable) {
        // QtBridge.onJsFilterChanged(data);
    // }
// }

// function onHeaderOptionChecked(data)
// {
    // if (isQtAvailable)
        // QtBridge.onJsHeaderOptionsChecked(data);
// }

// utility function: pipe errors to log
window.onerror = function (msg, url, num) {
    log("TaxonomyView: qwebchannel: Error: " + msg + "\nURL: " + url + "\nLine: " + num);
};

// utility function: auto log for Qt and console
function log(logtext) {
    if (isQtAvailable) {
        QtBridge.js_debug(logtext.toString());
    }
    else {
        console.log(logtext);
    }
}

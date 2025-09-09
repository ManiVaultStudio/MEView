// Simple app state
window.traceState = {
  data: null,         // last jsonDoc used to draw
  selectedSweep: 0,   // 0-based index from the spinner
};

let selectedSweep = 0;

const chartWidth = 240
const acqHeight = 180
const stimHeight = 160

const GRAPH_ELEMENT_NAME = "container";

// Data keys
const KEY_CELL = "cell";
const KEY_CELL_ID = "cellId";
const KEY_CELL_TYPE = "cluster";
const KEY_EPHYS = "ephys";
const KEY_ACQ = "acquisition";
const KEY_STIM = "stimulus";
const KEY_STIMSET = "stimset";
const KEY_STIM_EXTENT_X = "stimExtentX";
const KEY_STIM_EXTENT_Y = "stimExtentY";
const KEY_ACQ_EXTENT_X = "acqExtentX";
const KEY_ACQ_EXTENT_Y = "acqExtentY";

function setSweepOptions(cellObj)
{
    const spinner = document.getElementById("spinner");
    
    // Clear existing options
    spinner.innerHTML = "";
    
    if (KEY_EPHYS in cellObj)
    {
        let ephysObj = cellObj[KEY_EPHYS];
        let recordings = ephysObj["recordings"];
        var numGraphs  = recordings.length;
                
        // Add new options
        for (let i = 0; i < numGraphs; i++) {
            let recording = recordings[i];
            let sweepNum = parseInt(recording["sweepNumber"], 10);
            
            let option = document.createElement("option");
            option.value = i;
            option.text = "Sweep " + sweepNum;
            spinner.appendChild(option);
        }
    }
}

function setLabelContent(elementName, content)
{
    const label = document.getElementById(elementName);
    if (label)
        label.textContent = content;
}

function drawStimulusGraph(title, ephysObj)
{
    // Set dimensions and margins
    var margin = { top: 40, right: 10, bottom: 20, left: 45 },
        width = chartWidth - margin.left - margin.right,
        height = stimHeight - margin.top - margin.bottom;

    // Append the SVG object to the container
    var svg = d3.select("#"+GRAPH_ELEMENT_NAME)
        .append("svg")
        .attr("width", width + margin.left + margin.right)
        .attr("height", height + margin.top + margin.bottom)
        .style("display", "block")
        .append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    let stimExtentX = ephysObj[KEY_STIM_EXTENT_X];
    let stimExtentY = ephysObj[KEY_STIM_EXTENT_Y];

    // Add chart title
    svg.append("text")
        .attr("x", width / 2)
        .attr("y", -margin.top / 2)
        .attr("text-anchor", "middle")
        .style("font-size", "12px")
        .style("font-weight", "bold")
        .text(title || ""); // default to empty string if title not provided

    // Define scales
    var x = d3.scaleLinear()
        .domain(stimExtentX)
        .range([0, width]);

    var y = d3.scaleLinear()
        .domain(stimExtentY)
        .range([height, 0]);

    // Add X axis
    svg.append("g")
        .attr("transform", "translate(0," + height + ")")
        .call(d3.axisBottom(x).ticks(3));

    // Add Y axis
    svg.append("g")
        .call(d3.axisLeft(y).ticks(4));

    // Add X axis label
    svg.append("text")
        .attr("x", width / 2) // Center the label
        .attr("y", height + margin.bottom - 10) // Position below the X-axis
        .style("text-anchor", "middle") // Center the text
        .style("font-size", "10px") // Make text smaller
        .text(""); // Unit

    // Add Y axis label
    svg.append("text")
        .attr("transform", "rotate(-90)") // Rotate the text for Y-axis
        .attr("y", -margin.left + 5) // Adjust positioning
        .attr("x", -height / 2) // Center the label
        .attr("dy", "1em") // Fine-tune spacing
        .style("text-anchor", "middle") // Center align text
        .style("font-size", "10px") // Make text smaller
        .text("pA"); // Unit

    return svg;
}

function drawAcquisitionGraph(ephysObj)
{
    // Set dimensions and margins
    var margin = { top: 20, right: 10, bottom: 40, left: 45 },
        width = chartWidth - margin.left - margin.right,
        height = acqHeight - margin.top - margin.bottom;

    // Append the SVG object to the container
    var svg = d3.select("#"+GRAPH_ELEMENT_NAME)
        .append("svg")
        .attr("width", width + margin.left + margin.right)
        .attr("height", height + margin.top + margin.bottom)
        .style("display", "block")
        .append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    let acqExtentX = ephysObj[KEY_ACQ_EXTENT_X];
    let acqExtentY = ephysObj[KEY_ACQ_EXTENT_Y];
    
    // Define scales
    var x = d3.scaleLinear()
        .domain(acqExtentX)
        .range([0, width]);

    var y = d3.scaleLinear()
        .domain(acqExtentY)
        .range([height, 0]);

    // Add X axis
    svg.append("g")
        .attr("transform", "translate(0," + height + ")")
        .call(d3.axisBottom(x).ticks(3));

    // Add Y axis
    let yTicks = d3.range(acqExtentY[0], acqExtentY[1] + 20, 20);
    svg.append("g")
        .call(d3.axisLeft(y).tickValues(yTicks));

    // Add X axis label
    svg.append("text")
        .attr("x", width / 2) // Center the label
        .attr("y", height + margin.bottom - 10) // Position below the X-axis
        .style("text-anchor", "middle") // Center the text
        .style("font-size", "10px") // Make text smaller
        .text("ms"); // Unit

    // Add Y axis label
    svg.append("text")
        .attr("transform", "rotate(-90)") // Rotate the text for Y-axis
        .attr("y", -margin.left + 5) // Adjust positioning
        .attr("x", -height / 2) // Center the label
        .attr("dy", "1em") // Fine-tune spacing
        .style("text-anchor", "middle") // Center align text
        .style("font-size", "10px") // Make text smaller
        .text("mV"); // Unit

    log("draw acq")
    return svg;
}

function createNewGraphElement()
{
    var div = document.getElementById("traceRow");
    // Clear column divs
    while (div.firstChild)
    {
        div.removeChild(div.lastChild);
    }

    // Generate new one
    var newDiv = document.createElement('div');
    newDiv.className = "chart-div"
    newDiv.setAttribute("id", GRAPH_ELEMENT_NAME);
    div.appendChild(newDiv);
}

function drawEphysGraph(ephysObj)
{
    createNewGraphElement();

    let title = ephysObj["stimset"]

    // Draw graphs
    stimSvg = drawStimulusGraph(title, ephysObj);
    acqSvg = drawAcquisitionGraph(ephysObj);

    // Draw graph lines
    let recordings = ephysObj["recordings"];
    var numGraphs  = recordings.length;
    
    let acqExtentX = ephysObj[KEY_ACQ_EXTENT_X];
    let acqExtentY = ephysObj[KEY_ACQ_EXTENT_Y];
    
    let stimExtentX = ephysObj[KEY_STIM_EXTENT_X];
    let stimExtentY = ephysObj[KEY_STIM_EXTENT_Y];
    
    log("NumGraphs " + numGraphs);
    
    // STIMULI
    for (let i = 0; i < numGraphs; i++)
    {
        let recording = recordings[i];
        
        let stim_xData = recording[KEY_STIM]["xData"];
        let stim_yData = recording[KEY_STIM]["yData"];
        let stim_data = stim_xData.map((x, i) => ({ x: x, y: stim_yData[i] }));
        
        // Set dimensions and margins
        var margin = { top: 40, right: 10, bottom: 20, left: 45 },
            width = chartWidth - margin.left - margin.right,
            height = stimHeight - margin.top - margin.bottom;
            
        // Define scales
        var x = d3.scaleLinear()
            .domain(stimExtentX)
            .range([0, width]);

        var y = d3.scaleLinear()
            .domain(stimExtentY)
            .range([height, 0]);
        
        let color = (i == selectedSweep) ? "orangered" : "grey";
        let opacity = (i == selectedSweep) ? 1.0 : 0.2;
        // Add the acquisition line
        stimSvg.append("path")
            .datum(stim_data)
            .attr("fill", "none")
            .attr("stroke", color)
            .attr("stroke-width", 1.5)
            .attr("stroke-opacity", opacity)
            .attr("d", d3.line()
                .x(d => x(d.x))
                .y(d => y(d.y))
            );
    }
    // ACQUISITIONS
    for (let i = 0; i < numGraphs; i++)
    {
        let recording = recordings[i];
        let acqObj = recording[KEY_ACQ];
        
        let acq_xData = acqObj["xData"];
        let acq_yData = acqObj["yData"];
        let acq_data = acq_xData.map((x, i) => ({ x: x, y: acq_yData[i] }));
        
        var margin = { top: 20, right: 10, bottom: 40, left: 45 },
            width = chartWidth - margin.left - margin.right,
            height = acqHeight - margin.top - margin.bottom;
            
        // Define scales
        var x = d3.scaleLinear()
            .domain(acqExtentX)
            .range([0, width]);

        var y = d3.scaleLinear()
            .domain(acqExtentY)
            .range([height, 0]);
        
        let color = (i == selectedSweep) ? "steelblue" : "grey";
        let opacity = (i == selectedSweep) ? 1.0 : 0.1;
        // Add the acquisition line
        acqSvg.append("path")
            .datum(acq_data)
            .attr("fill", "none")
            .attr("stroke", color)
            .attr("stroke-width", 1.0)
            .attr("stroke-opacity", opacity)
            .attr("d", d3.line()
                .x(d => x(d.x))
                .y(d => y(d.y))
            );
    }
}

function drawCellCard(cellObj)
{
    // If new data is passed, persist it
    window.traceState.data = cellObj;
    
    // Draw cell metadata
    setLabelContent("cellIdLabel", cellObj[KEY_CELL_ID]);
    setLabelContent("clusterLabel", cellObj[KEY_CELL_TYPE]);

    // Initialize spinner
    const spinnerEl = document.getElementById('spinner');
    
    if (spinnerEl)
    {
      // Ensure there's a selection; if not, default to first option when available
      if (spinnerEl.selectedIndex === -1 && spinnerEl.options.length) {
        spinnerEl.selectedIndex = 0;
      }
      selectedSweep = spinnerEl.selectedIndex; // 0..N-1
      // If you prefer the numeric value (1..N) that you set in spinner.js:
      // selectedSweep = Math.max(0, (parseInt(spinnerEl.value, 10) || 1) - 1);
    }
    
    // Draw ephys graph
    if (KEY_EPHYS in cellObj)
        drawEphysGraph(cellObj[KEY_EPHYS])
}

function redrawCellCard()
{
    const cellObj = window.traceState.data;
    if (!cellObj) return;
    
    drawCellCard(cellObj);
}

document.getElementById('spinner')?.addEventListener('change', () => {
    log("Spinner changed");
    redrawCellCard();
});

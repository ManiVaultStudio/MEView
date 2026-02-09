// Simple app state
window.traceState = {
  data: null,         // last jsonDoc used to draw
  hoveredSweep: -1, // index of sweep currently hovered over, or -1 if none
  lockedSweep: -1,  // clicked sweep, -1 when nothing is clicked
};

const chartWidth = 400
const acqHeight = 100
const stimHeight = 100

const GRAPH_ELEMENT_NAME = "trace_container";

// Data keys
const KEY_CELL = "cell";
const KEY_CELL_ID = "cellId";
const KEY_CELL_NAME = "cellName";
const KEY_CELL_TYPE = "cluster";
const KEY_EPHYS = "ephys";
const KEY_ACQ = "acquisition";
const KEY_STIM = "stimulus";
const KEY_STIMTYPE = "stimtype";
const KEY_STIMDESC = "stimDesc";
const KEY_STIM_EXTENT_X = "stimExtentX";
const KEY_STIM_EXTENT_Y = "stimExtentY";
const KEY_ACQ_EXTENT_X = "acqExtentX";
const KEY_ACQ_EXTENT_Y = "acqExtentY";

const SWEEP_COLORS = [
  "#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd",
  "#8c564b", "#e377c2", "#7f7f7f", "#bcbd22", "#17becf",
  "#4e79a7", "#f28e2b", "#59a14f", "#e15759", "#b07aa1",
  "#9c755f", "#edc949", "#76b7b2", "#af7aa1", "#bab0ab"
];

function getSweepColor(i) {
  return SWEEP_COLORS[i % SWEEP_COLORS.length];
}

function getSelectedIndex()
{
  const locked = window.traceState.lockedSweep;
  const hovered = window.traceState.hoveredSweep;
  const selected = (locked >= 0) ? locked : ((hovered >= -1) ? hovered : -1);
  return selected;
}

// Returns { stroke, opacity } for a given sweep index
function getTraceStyle(i)
{
  const selected = getSelectedIndex();

  // Default: all sweeps in their own colors, full opacity
  if (selected == -1)
    return { stroke: getSweepColor(i), opacity: 1.0 };

  // Selected: selected sweep in its color full opacity; others grey + faded
  if (i === selected) {
    return { stroke: getSweepColor(i), opacity: 1.0 };
  }

  return { stroke: "grey", opacity: 0.05 };
}

function setSweepButtons(cellObj) {
  const palette = document.getElementById("sweepPalette");
  if (!palette) return;

  palette.innerHTML = "";

  // Default: no sweeps
  if (!(KEY_EPHYS in cellObj)) {
    palette.textContent = "No sweeps available";
    return;
  }

  const ephysObj = cellObj[KEY_EPHYS];
  const recordings = ephysObj["recordings"] || [];
  const numSweeps = recordings.length;

  if (!numSweeps) {
    palette.textContent = "No sweeps available";
    return;
  }

  for (let i = 0; i < numSweeps; i++) {
    const recording = recordings[i];
    const sweepNum = parseInt(recording["sweepNumber"], 10);

    const btn = document.createElement("button");
    btn.type = "button";
    btn.className = "sweep-btn";
    btn.style.backgroundColor = SWEEP_COLORS[i % SWEEP_COLORS.length];
    btn.dataset.index = String(i);
    btn.title = `Sweep ${sweepNum}`; // hover tooltip

    btn.addEventListener("mouseenter", () => {
      log("Hover, locked state: " + window.traceState.lockedSweep);
      // If locked, hovering does nothing
      if (window.traceState.lockedSweep >= 0) return;

      window.traceState.hoveredSweep = i;
            log("Hover f, hover state: " + window.traceState.hoveredSweep + " i: " + i);
      redrawGraphs();
      updateSweepButtonStyles();
      log("Hover f, hover state: " + window.traceState.hoveredSweep);
    });

    btn.addEventListener("click", () => {
              log("Hover, locked state: " + window.traceState.lockedSweep);
      const locked = window.traceState.lockedSweep;

      if (locked === i) {
        // Unclick => return to default state
        window.traceState.lockedSweep = -1;
        window.traceState.hoveredSweep = -1;
      } else {
        // Click selects/locks this sweep (and replaces any previous lock)
        window.traceState.lockedSweep = i;
        window.traceState.hoveredSweep = -1; // hover no longer active
      }
      log("Hover, locked state: " + window.traceState.lockedSweep);
      redrawGraphs();
      updateSweepButtonStyles();
    });

    palette.appendChild(btn);
  }
  
    palette.addEventListener("mouseleave", () => {
              log("Palette hover end, locked state: " + window.traceState.lockedSweep);
      if (window.traceState.lockedSweep >= 0) return;

      window.traceState.hoveredSweep = -1;
      redrawGraphs();
      updateSweepButtonStyles();
            log("Palette hover endf, hover state: " + window.traceState.hoveredSweep);
    });

    updateSweepButtonStyles();
}

function updateSweepButtonStyles()
{
  const palette = document.getElementById("sweepPalette");
  if (!palette) return;

  const locked = window.traceState.lockedSweep;
  const hovered = window.traceState.hoveredSweep;

  // Selected sweep: locked overrides hover
  const selected = (locked >= 0) ? locked : ((hovered >= -1) ? hovered : -1);

  palette.querySelectorAll(".sweep-btn").forEach((b) => {
    const idx = parseInt(b.dataset.index, 10);

    // Locked outline only when locked
    b.classList.toggle("is-locked", locked === idx);

    // Grey out all others only when a focus exists
    const shouldGrey = (selected != -1 && idx !== selected);
    b.classList.toggle("is-greyed", shouldGrey);

    // When not greyed, show its real palette color
    if (!shouldGrey) {
      b.style.backgroundColor = getSweepColor(idx);
      b.style.opacity = "1.0";
    }
  });
}

function setSweepInfo(ephysObj)
{
  const el = document.getElementById("sweepInfo");
  if (!el) return;

  const recordings = ephysObj?.recordings || [];
  if (!recordings.length) {
    el.textContent = "";
    return;
  }

  const selectedSweep = getSelectedIndex();

  if (selectedSweep < 0 || selectedSweep >= recordings.length) {
    // Default (no hover/lock): show nothing or a summary
    el.innerHTML =
      `Sweep: <span class="sweep-info-value">-</span> ` +
      `&nbsp;&nbsp;Stimulus amplitude: <span class="sweep-info-value">-</span> ` +
      `&nbsp;&nbsp;Number of spikes: <span class="sweep-info-value">-</span>`;
    return;
  }
  
  const rec = recordings[selectedSweep];

  setLabelContent("stimDescLabel", rec[KEY_STIM][KEY_STIMDESC]);

  const sweepNum = parseInt(rec["sweepNumber"], 10);
  const stimAmplitude = parseFloat(rec[KEY_STIM]["stimAmplitude"])
  const spikeCount = 0;

  const ampText = (stimAmplitude == null) ? "—" : `${stimAmplitude} pA`;
  const numSpikesText = (spikeCount == null) ? "—" : `${spikeCount}`;

  el.innerHTML =
    `Sweep: <span class="sweep-info-value">${sweepNum}</span> ` +
    `&nbsp;&nbsp;Stimulus amplitude: <span class="sweep-info-value">${ampText}</span> ` +
    `&nbsp;&nbsp;Number of spikes: <span class="sweep-info-value">${numSpikesText}</span>`;
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
    var margin = { top: 10, right: 10, bottom: 40, left: 45 },
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

    // // Add chart title
    // svg.append("text")
        // .attr("x", width / 2)
        // .attr("y", -margin.top / 2)
        // .attr("text-anchor", "middle")
        // .style("font-size", "12px")
        // .style("font-weight", "bold")
        // .text(title || ""); // default to empty string if title not provided

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
        .call(d3.axisBottom(x).ticks(6));

    // Add Y axis
    svg.append("g")
        .call(d3.axisLeft(y).ticks(4));

    // Add X axis label
    svg.append("text")
        .attr("x", width / 2) // Center the label
        .attr("y", height + margin.bottom - 10) // Position below the X-axis
        .style("text-anchor", "middle") // Center the text
        .style("font-size", "10px") // Make text smaller
        .text("Time (s)"); // Unit

    // Add Y axis label
    svg.append("text")
        .attr("transform", "rotate(-90)") // Rotate the text for Y-axis
        .attr("y", -margin.left + 5) // Adjust positioning
        .attr("x", -height / 2) // Center the label
        .attr("dy", "1em") // Fine-tune spacing
        .style("text-anchor", "middle") // Center align text
        .style("font-size", "10px") // Make text smaller
        .text("pA"); // Unit

    return [svg, x, y];
}

function drawAcquisitionGraph(ephysObj)
{
    // Set dimensions and margins
    var margin = { top: 20, right: 10, bottom: 20, left: 45 },
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
        .style("font-size", "10px"); // Make text smaller

    // Add Y axis label
    svg.append("text")
        .attr("transform", "rotate(-90)") // Rotate the text for Y-axis
        .attr("y", -margin.left + 5) // Adjust positioning
        .attr("x", -height / 2) // Center the label
        .attr("dy", "1em") // Fine-tune spacing
        .style("text-anchor", "middle") // Center align text
        .style("font-size", "10px") // Make text smaller
        .text("mV"); // Unit

    return [svg, x, y];
}

function emptyGraphElement()
{
    var div = document.getElementById("traceDiv");
    // Clear column divs
    while (div.firstChild)
    {
        div.removeChild(div.lastChild);
    }
}

function createNewGraphElement()
{
    var div = document.getElementById("traceDiv");
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

    setLabelContent("stimTypeLabel", ephysObj[KEY_STIMTYPE]);

    setSweepInfo(ephysObj);

    let title = ephysObj[KEY_STIMTYPE]

    // Draw graphs
    let [acqSvg, acqX, acqY] = drawAcquisitionGraph(ephysObj);
    let [stimSvg, stimX, stimY] = drawStimulusGraph(title, ephysObj);

    // Draw graph lines
    let recordings = ephysObj["recordings"];
    var numGraphs  = recordings.length;
    
    let acqExtentX = ephysObj[KEY_ACQ_EXTENT_X];
    let acqExtentY = ephysObj[KEY_ACQ_EXTENT_Y];
    
    let stimExtentX = ephysObj[KEY_STIM_EXTENT_X];
    let stimExtentY = ephysObj[KEY_STIM_EXTENT_Y];
    
    // STIMULI
    for (let i = 0; i < numGraphs; i++)
    {
        let recording = recordings[i];
        
        let stim_xData = recording[KEY_STIM]["xData"];
        let stim_yData = recording[KEY_STIM]["yData"];
        let stim_data = stim_xData.map((x, i) => ({ x: x, y: stim_yData[i] }));
        
        const { stroke: color, opacity } = getTraceStyle(i);

        // Add the stimulus line
        stimSvg.append("path")
            .datum(stim_data)
            .attr("fill", "none")
            .attr("stroke", color)
            .attr("stroke-width", 1.5)
            .attr("stroke-opacity", opacity)
            .attr("d", d3.line()
                .x(d => stimX(d.x))
                .y(d => stimY(d.y))
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
                
        const { stroke: color, opacity } = getTraceStyle(i);
        
        // Add the acquisition line
        acqSvg.append("path")
            .datum(acq_data)
            .attr("fill", "none")
            .attr("stroke", color)
            .attr("stroke-width", 1.0)
            .attr("stroke-opacity", opacity)
            .attr("d", d3.line()
                .x(d => acqX(d.x))
                .y(d => acqY(d.y))
            );
    }
}

// MAIN ENTRY POINT for new data
function drawEphysCard(cellObj)
{
  // Persist data
  window.traceState.data = cellObj;
  window.traceState.lockedSweep = -1;
  window.traceState.hoveredSweep = -1;
  
    // Metadata
  setLabelContent("cellIdLabel", cellObj[KEY_CELL_ID]);
  setLabelContent("clusterLabel", cellObj[KEY_CELL_TYPE]);
  setLabelContent("cellNameLabel", cellObj[KEY_CELL_NAME]);

  setSweepButtons(cellObj);

  redrawGraphs();
}

function redrawGraphs()
{
    emptyGraphElement();

    const cellObj = window.traceState.data;
    if (!cellObj) return;
    
    // Draw ephys
    if (KEY_EPHYS in cellObj) {
        drawEphysGraph(cellObj[KEY_EPHYS]);
    }
}

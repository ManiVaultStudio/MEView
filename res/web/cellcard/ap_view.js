const apChartWidth = 160
const apHeight = 160

function createNewApGraphElement()
{
    var div = document.getElementById("apCol");
    // Clear column divs
    while (div.firstChild)
    {
        div.removeChild(div.lastChild);
    }
    log("Deleting AP graph")
    // Generate new one
    var newDiv = document.createElement('div');
    newDiv.className = "chart-div"
    newDiv.setAttribute("id", "ap_container");
    div.appendChild(newDiv);
}

function emptyApGraphElement()
{
    var div = document.getElementById("apCol");
    // Clear column divs
    while (div.firstChild)
    {
        div.removeChild(div.lastChild);
    }
    log("Emptying AP graph")
}

function drawActionPotentialGraph(apObj)
{
    createNewApGraphElement()
    
    // Set dimensions and margins
    var margin = { top: 40, right: 10, bottom: 40, left: 45 },
        width = apChartWidth - margin.left - margin.right,
        height = apHeight - margin.top - margin.bottom;

    // Append the SVG object to the container
    var svg = d3.select("#ap_container")
        .append("svg")
        .attr("width", width + margin.left + margin.right)
        .attr("height", height + margin.top + margin.bottom)
        .style("display", "block")
        .append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    // let stimExtentX = ephysObj[KEY_STIM_EXTENT_X];
    // let stimExtentY = ephysObj[KEY_STIM_EXTENT_Y];

    // Add chart title
    svg.append("text")
        .attr("x", width / 2)
        .attr("y", -margin.top / 2)
        .attr("text-anchor", "middle")
        .style("font-size", "12px")
        .style("font-weight", "bold")
        .text("Action Potential");

    // Define scales
    var x = d3.scaleLinear()
        .domain([0, 4])
        .range([0, width]);

    var y = d3.scaleLinear()
        .domain([-60, 50])
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
        .text("Time (ms)"); // Unit

    // Add Y axis label
    svg.append("text")
        .attr("transform", "rotate(-90)") // Rotate the text for Y-axis
        .attr("y", -margin.left + 5) // Adjust positioning
        .attr("x", -height / 2) // Center the label
        .attr("dy", "1em") // Fine-tune spacing
        .style("text-anchor", "middle") // Center align text
        .style("font-size", "10px") // Make text smaller
        .text("Voltage (mV)"); // Unit

    let xData = apObj["xData"];
    let yData = apObj["yData"];
    let ap_data = xData.map((x, i) => ({ x: x, y: yData[i] }));
    log("Length: " + xData.length);
    let color = "steelblue";
    let opacity = 1.0;
    // Add the acquisition line
    svg.append("path")
        .datum(ap_data)
        .attr("fill", "none")
        .attr("stroke", color)
        .attr("stroke-width", 1.5)
        .attr("stroke-opacity", opacity)
        .attr("d", d3.line()
            .x(d => x(d.x))
            .y(d => y(d.y))
        );

    return svg;
}

/**
 * Transforms (time, voltage) data into:
 *  x' = voltage (mV)
 *  y' = d(voltage)/d(time) (mV/ms)
 *
 * @param {number[]} xData - time values in ms
 * @param {number[]} yData - voltage values in mV
 * @returns {{ x: number[], y: number[] }}
 */
function transformToVoltageVsSlope(xData, yData)
{
  if (xData.length !== yData.length) {
    throw new Error("xData and yData must have the same length");
  }
  if (xData.length < 2) {
    throw new Error("At least two data points are required");
  }

  const xTransformed = [];
  const yTransformed = [];

  for (let i = 0; i < xData.length - 1; i++) {
    const dx = xData[i + 1] - xData[i]; // ms
    const dy = yData[i + 1] - yData[i]; // mV

    if (dx === 0) continue; // avoid division by zero

    xTransformed.push(yData[i]);      // mV on x-axis
    yTransformed.push(dy / dx);       // mV/ms on y-axis
  }

  return {
    x: xTransformed,
    y: yTransformed
  };
}

function drawActionPotentialPhasePlot(apObj)
{
        // Set dimensions and margins
    var margin = { top: 40, right: 10, bottom: 40, left: 45 },
        width = apChartWidth - margin.left - margin.right,
        height = apHeight - margin.top - margin.bottom;

    // Append the SVG object to the container
    var svg = d3.select("#ap_container")
        .append("svg")
        .attr("width", width + margin.left + margin.right)
        .attr("height", height + margin.top + margin.bottom)
        .style("display", "block")
        .append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    // Add chart title
    svg.append("text")
        .attr("x", width / 2)
        .attr("y", -margin.top / 2)
        .attr("text-anchor", "middle")
        .style("font-size", "12px")
        .style("font-weight", "bold")
        .text("Phase Plot");

    // Get data
    let xData = apObj["xData"];
    let yData = apObj["yData"];
    
    const transformed = transformToVoltageVsSlope(xData, yData)
    
    const xRange = [Math.min(...transformed.x), Math.max(...transformed.x)];
    const yRange = [Math.min(...transformed.y), Math.max(...transformed.y)];

    // Define scales
    var x = d3.scaleLinear()
        .domain(xRange)
        .range([0, width]);

    var y = d3.scaleLinear()
        .domain(yRange)
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
        .text("Voltage (mV)"); // Unit

    // Add Y axis label
    svg.append("text")
        .attr("transform", "rotate(-90)") // Rotate the text for Y-axis
        .attr("y", -margin.left + 5) // Adjust positioning
        .attr("x", -height / 2) // Center the label
        .attr("dy", "1em") // Fine-tune spacing
        .style("text-anchor", "middle") // Center align text
        .style("font-size", "10px") // Make text smaller
        .text("dV/dt (mV/ms)"); // Unit
    
    let phase_data = transformed.x.map((x, i) => ({ x: x, y: transformed.y[i] }));
    log("Length: " + transformed.x.length);
    let color = "steelblue";
    let opacity = 1.0;
    // Add the acquisition line
    svg.append("path")
        .datum(phase_data)
        .attr("fill", "none")
        .attr("stroke", color)
        .attr("stroke-width", 1.5)
        .attr("stroke-opacity", opacity)
        .attr("d", d3.line()
            .x(d => x(d.x))
            .y(d => y(d.y))
        );

    return svg;
}

function drawActionPotential(ephysObj)
{
    if ("actionPotential" in ephysObj)
    {
        let apObj = ephysObj["actionPotential"]
    
        drawActionPotentialGraph(apObj)
        drawActionPotentialPhasePlot(apObj)
    }
    else
        log("boop");
}

function drawAPView(cellObj)
{
    emptyApGraphElement();
    
    // Draw ephys graph
    if ("ephys" in cellObj)
        drawActionPotential(cellObj["ephys"])
    else
        log("beep");
}

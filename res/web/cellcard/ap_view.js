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

function drawActionPotential(ephysObj)
{
    if ("actionPotential" in ephysObj)
    {
        let apObj = ephysObj["actionPotential"]
    
        drawActionPotentialGraph(apObj)
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

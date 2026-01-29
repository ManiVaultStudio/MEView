const apChartWidth = 160
const apHeight = 160

window.AP = window.AP || {};
window.PP = window.PP || {};

AP.properties = {
  margin: { top: 40, right: 10, bottom: 40, left: 45 },
  title: "Action Potential",
  xRange: [0, 4],
  yRange: [-60, 50],
  xUnit: "Time (ms)",
  yUnit: "Voltage (mV)",
  lineColor: "steelblue"
};

PP.properties = {
  margin: { top: 40, right: 10, bottom: 40, left: 45 },
  title: "Phase Plot",
  xRange: 0,
  yRange: 0,
  xUnit: "Voltage (mV)",
  yUnit: "dV/dt (mV/ms)",
  lineColor: "steelblue"
};

function clearApSvgs()
{
    d3.select("#ap_graph").selectAll("svg").remove();
    d3.select("#phase_graph").selectAll("svg").remove();
    log("Emptying AP graph")
}

function drawGraph(containerSelector, props, xData, yData)
{
    // Set dimensions and margins
    var margin = props.margin,
        width = apChartWidth - props.margin.left - props.margin.right,
        height = apHeight - props.margin.top - props.margin.bottom;
    
    // Append the SVG object to the container
    var svg = d3.select(containerSelector)
        .append("svg")
        .attr("width", width + props.margin.left + props.margin.right)
        .attr("height", height + props.margin.top + props.margin.bottom)
        .style("display", "block")
        .append("g")
        .attr("transform", "translate(" + props.margin.left + "," + props.margin.top + ")");
    
    // Add chart title
    svg.append("text")
        .attr("x", width / 2)
        .attr("y", -margin.top / 2)
        .attr("text-anchor", "middle")
        .style("font-size", "12px")
        .style("font-weight", "bold")
        .text(props.title);
    
    // Define scales
    var x = d3.scaleLinear()
        .domain(props.xRange)
        .range([0, width]);

    var y = d3.scaleLinear()
        .domain(props.yRange)
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
        .attr("y", height + props.margin.bottom - 10) // Position below the X-axis
        .style("text-anchor", "middle") // Center the text
        .style("font-size", "10px") // Make text smaller
        .text(props.xUnit); // Unit

    // Add Y axis label
    svg.append("text")
        .attr("transform", "rotate(-90)") // Rotate the text for Y-axis
        .attr("y", -props.margin.left + 5) // Adjust positioning
        .attr("x", -height / 2) // Center the label
        .attr("dy", "1em") // Fine-tune spacing
        .style("text-anchor", "middle") // Center align text
        .style("font-size", "10px") // Make text smaller
        .text(props.yUnit); // Unit

    if (xData.length == 0 || yData.length == 0)
        return;

    let graphData = xData.map((x, i) => ({ x: x, y: yData[i] }));
    log("Length: " + xData.length);
    
    let opacity = 1.0;
    // Add the acquisition line
    svg.append("path")
        .datum(graphData)
        .attr("fill", "none")
        .attr("stroke", props.lineColor)
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
  
  const xTransformed = [];
  const yTransformed = [];
  
  if (xData.length < 2) {
    log("AP data has less than two data points");
      return {
        x: xTransformed,
        y: yTransformed
      };
  }

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

function drawAPView(cellObj)
{
    clearApSvgs();
    
    // Draw ephys graph
    if ("ephys" in cellObj)
    {
        const ephysObj = cellObj["ephys"];
        
        if ("actionPotential" in ephysObj)
        {
            let apObj = ephysObj["actionPotential"]
            const xData = apObj["xData"];
            const yData = apObj["yData"];
            
            // Action Potential Plot
            drawGraph("#ap_graph", AP.properties, xData, yData)
        
            // Phase Plot
            const transformed = transformToVoltageVsSlope(xData, yData)
            const xRange = [Math.min(...transformed.x), Math.max(...transformed.x)];
            const yRange = [Math.min(...transformed.y), Math.max(...transformed.y)];
            
            PP.properties.xRange = xRange
            PP.properties.yRange = yRange
            
            drawGraph("#phase_graph", PP.properties, transformed.x, transformed.y)
        }
        else
        {
            log("No action potential in ephysObj");
        }
    }
    else
        log("No ephys in cellObj");
}

const chartWidth = 180
const acqHeight = 160
const stimHeight = 120

function drawTrace(jsonDoc)
{
    const startTime = performance.now()
    log("Draw trace");
    
    d3.select("svg").remove();
    const s1Time = performance.now()

    // Extract the data
    const keyAcq = "acquisition";
    const keyStim = "stimulus";
    const keyTitle = "title";
    const keyStimExtentX = "stimExtentX";
    const keyStimExtentY = "stimExtentY";
    const keyAcqExtentX = "acqExtentX";
    const keyAcqExtentY = "acqExtentY";

    function drawGraph(selector, graphObj)
    {
        function drawStimulusGraph(selector, graphObj)
        {
            // Set dimensions and margins
            var margin = { top: 40, right: 10, bottom: 20, left: 45 },
                width = chartWidth - margin.left - margin.right,
                height = stimHeight - margin.top - margin.bottom;
            
            // Append the SVG object to the container
            var svg = d3.select(selector)
                .append("svg")
                .attr("width", width + margin.left + margin.right)
                .attr("height", height + margin.top + margin.bottom)
                .style("display", "block")
                .append("g")
                .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
            
            let titleData = graphObj[keyTitle];
            let stimExtentX = graphObj[keyStimExtentX];
            let stimExtentY = graphObj[keyStimExtentY];
            
            // Add chart title
            svg.append("text")
                .attr("x", width / 2)
                .attr("y", -margin.top / 2)
                .attr("text-anchor", "middle")
                .style("font-size", "12px")
                .style("font-weight", "bold")
                .text(titleData || ""); // default to empty string if title not provided
            
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
        
        function drawAcquisitionGraph(selector, graphObj)
        {
            // Set dimensions and margins
            var margin = { top: 20, right: 10, bottom: 40, left: 45 },
                width = chartWidth - margin.left - margin.right,
                height = acqHeight - margin.top - margin.bottom;
            
            const ps = performance.now()
            
            // Append the SVG object to the container
            var svg = d3.select(selector)
                .append("svg")
                .attr("width", width + margin.left + margin.right)
                .attr("height", height + margin.top + margin.bottom)
                .style("display", "block")
                .append("g")
                .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

            const pm1 = performance.now()

            let acqExtentX = graphObj[keyAcqExtentX];
            let acqExtentY = graphObj[keyAcqExtentY];
            
            const pm2 = performance.now()

            const pm3 = performance.now()
            
            // Define scales
            var x = d3.scaleLinear()
                .domain(acqExtentX)
                .range([0, width]);

            var y = d3.scaleLinear()
                .domain(acqExtentY)
                .range([height, 0]);

            const pm4 = performance.now()
            // Add X axis
            svg.append("g")
                .attr("transform", "translate(0," + height + ")")
                .call(d3.axisBottom(x).ticks(3));

            // Add Y axis
            let yTicks = d3.range(acqExtentY[0], acqExtentY[1] + 20, 20);
            svg.append("g")
                .call(d3.axisLeft(y).tickValues(yTicks));

            const pm5 = performance.now()
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

            const pm6 = performance.now()

            const pe = performance.now()
            
            log(`Call to drawGraph took ${pe - pm6} ${pm6 - pm5} ${pm5 - pm4} ${pm4 - pm3} ${pm3 - pm2} ${pm2 - pm1}  ${pm1 - ps} milliseconds`)
                
            return svg;
        }
        
        stimSvg = drawStimulusGraph(selector, graphObj);
        acqSvg = drawAcquisitionGraph(selector, graphObj);
        
        let recordings = graphObj["recordings"];
        var numGraphs  = recordings.length;
        
        let acqExtentX = graphObj[keyAcqExtentX];
        let acqExtentY = graphObj[keyAcqExtentY];
        
        let stimExtentX = graphObj[keyStimExtentX];
        let stimExtentY = graphObj[keyStimExtentY];
        
        log("NumGraphs " + numGraphs);
        // STIMULI
        for (let i = 0; i < numGraphs; i++)
        {
            let recording = recordings[i];
            
            let stim_xData = recording[keyStim]["xData"];
            let stim_yData = recording[keyStim]["yData"];
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
            
            let color = (i == numGraphs-1) ? "orangered" : "grey";
            let opacity = (i == numGraphs-1) ? 1.0 : 0.2;
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
            
            let acq_xData = recording[keyAcq]["xData"];
            let acq_yData = recording[keyAcq]["yData"];
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
            
            let color = (i == 0) ? "steelblue" : "grey";
            let opacity = (i == 0) ? 1.0 : 0.1;
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


    const s2Time = performance.now()
    
    let graphObj = jsonDoc["graph"]
    
    var div = document.getElementById("traceRow");
    // Clear column divs
    while (div.firstChild)
    {
        div.removeChild(div.lastChild);
    }
    const s3Time = performance.now()
    
    // Generate new ones
    let i = 0;
    var newDiv = document.createElement('div');
    newDiv.className = "chart-div"
    newDiv.setAttribute("id", "container" + i);
    div.appendChild(newDiv);

    drawGraph("#container" + i, graphObj)

    const endTime = performance.now()
    
    log(`Call to drawTrace took ${endTime - s3Time} ${s3Time - s2Time} ${s2Time - s1Time} ${s1Time - startTime} milliseconds`)
}

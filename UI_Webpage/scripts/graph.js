/*  *****     *****     *****     *****  LIVE GRAPH SETUP  *****     *****     *****     *****  */

var LiveGraph = {
  init : function() {
    // Create the main elements for the graph
    graphSvg = d3.select("#LiveGraph").append("svg"); // Add an svg to the existing LiveGraph div
    svgG = graphSvg.append("g"); // Adds a 'group' element to the svg; for the main container
    xAxis = svgG.append('g'); // Adds a group for the x axis
    pAxis = svgG.append('g'); // Adds a group for the pressure axis
    dAxis = svgG.append('g'); // Adds a group for the deflection axis
    pPath = svgG.append("path"); // Adds a path to the svg to display the pressure
    d1Path = svgG.append('path'); // Adds a path to the svg to display the d1 data
    d2Path = svgG.append('path'); // Adds a path to the svg to display the d2 data

    // Add axis labels
    xAxisLabel = svgG.append('text').attr('class', 'axis-label').style('font-size', '12px');
    pAxisLabel = svgG.append('text').attr('class', 'axis-label').style('font-size', '12px');
    dAxisLabel = svgG.append('text').attr('class', 'axis-label').style('font-size', '12px');

    // Initialize empty data arrays
    this.pressureData = [];
    this.d1Data = [];
    this.d2Data = [];
  }
};
LiveGraph.init();

// Define the function that will render and re-render the graph
LiveGraph.render = function(){
    // Settings
    var transitionDelay = 400;
    var marginRatio = 0.1;

    // Setting up the bounding box
    var margin = {
        top: 10,
        right: 40,
        bottom: 30,
        left: 50
      };
    var width = $('#mainGrid').width() - 60 - margin.left - margin.right;
    var height = 340 - margin.top - margin.bottom;

    graphSvg
        .attr("width", width + margin.left + margin.right)
        .attr("height", height + margin.top + margin.bottom);
    svgG.attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    // Reneder the time axis
    var xMax = d3.max(this.pressureData, d => d[0]);
    var xMin = d3.min(this.pressureData, d => d[0]);
    var xRange = xMax - xMin;
    var xScale = d3.scaleLinear()
      .domain([xMin, xMax])
      .range([0, width]);
    xAxis
      .transition()
        .duration(transitionDelay)
        .ease(d3.easeCubicOut)
      .attr("transform", "translate(0," + height + ")")
      .call(d3.axisBottom(xScale));

    // X axis label
    xAxisLabel
      .attr("text-anchor", "middle")
      .attr("x", width / 2)
      .attr("y", height + margin.bottom)
      .text("Time (minutes)");

    // Render the pressure axis
    var pMax = d3.max(this.pressureData, d => d[1]);
    var pMin = d3.min(this.pressureData, d => d[1]);
    // Ensure domain includes zero
    pMax = Math.max(pMax, 0);
    pMin = Math.min(pMin, 0);
    var pRange = pMax - pMin;
    var pScale = d3.scaleLinear()
      .domain([pMin - (pRange * marginRatio), pMax + (pRange * marginRatio)])
      .range([height, 0]);
    pAxis
      .transition()
        .duration(transitionDelay)
        .ease(d3.easeCubicOut)
      .call(d3.axisLeft(pScale).tickFormat(d3.format('.0f')));

    // Pressure axis label
    pAxisLabel
      .attr("text-anchor", "middle")
      .attr("transform", "rotate(-90)")
      .attr("x", -height / 2)
      .attr("y", -margin.left + 15)
      .text("Pressure (Pa)");

    // Render the deflection axis
    var d1Max = d3.max(this.d1Data, d => d[1]);
    var d1Min = d3.min(this.d1Data, d => d[1]);
    var d2Max = d3.max(this.d2Data, d => d[1]);
    var d2Min = d3.min(this.d2Data, d => d[1]);
    var dMax = d1Max > d2Max ? d1Max : d2Max;
    var dMin = d1Min < d2Min ? d1Min : d2Min;
    var dRange = dMax - dMin;
    var dScale = d3.scaleLinear()
      .domain([dMin - (dRange * marginRatio), dMax + (dRange * marginRatio)])
      .range([height, 0]);
    dAxis
      .transition()
        .duration(transitionDelay)
        .ease(d3.easeCubicOut)
      .attr("transform", "translate("+width+",0)")
      .call(d3.axisRight(dScale).tickFormat(d3.format('.1f')));

    // Deflection axis label
    dAxisLabel
      .attr("text-anchor", "middle")
      .attr("transform", "rotate(-90)")
      .attr("x", -height / 2)
      .attr("y", width + margin.right)
      .text("Displacement (mm)");

    // Render the data paths
    const  line = d3.line().x(d => xScale(d[0])).y(d => pScale(d[1]));
    const dline = d3.line().x(d => xScale(d[0])).y(d => dScale(d[1]));

    // Update paths directly without creating new data bindings
    pPath
      .datum(this.pressureData)
      .transition()
        .duration(transitionDelay)
        .ease(d3.easeCubicOut)
      .attr("d", line)
      .style("fill", "none")
      .style("stroke", "steelblue")
      .style("stroke-width", "3px");

    d1Path
      .datum(this.d1Data)
      .transition()
        .duration(transitionDelay)
        .ease(d3.easeCubicOut)
      .attr('d', dline)
      .style("fill", "none")
      .style("stroke", "#AA2222")
      .style("stroke-width", "3px");

    d2Path
      .datum(this.d2Data)
      .transition()
        .duration(transitionDelay)
        .ease(d3.easeCubicOut)
      .attr('d', dline)
      .style("fill", "none")
      .style("stroke", "#AA2222")
      .style("stroke-width", "3px");
  }

LiveGraph.render();

$(window).resize(() => LiveGraph.render() );

/*  *****     *****     *****     ***** /LIVE GRAPH SETUP *****     *****     *****     *****  */





/*  *****     *****     *****     ***** LIVE GRAPH HOOKUP / REGISTRATION *****     *****     *****     *****  */

var updateGraph = function(data){
  const maxNumberOfPointsToShow = 180;
  const allRequestedVars = new Set();
  for (const subKey in liveDataSubscribers) {
    liveDataSubscribers[subKey].values.forEach(v => allRequestedVars.add(v));
  }

  var t = 0;

  if('secsToday' in data) t = data.secsToday / 60;
  else {
    console.error("updateGraph: Error: there was no data for 'secsToday'.");
    return;
  }

  var pressure = 0;
  var disp1 = 0;
  var disp2 = 0;

  if('wLowPressure' in data) pressure = data.wLowPressure;
  if('wMedPressure' in data) pressure = data.wMedPressure;
  if('wHighPressure' in data) pressure = data.wHighPressure;

  if('wDisplacement1' in data) disp1 = data.wDisplacement1;
  if('wDisplacement2' in data) disp2 = data.wDisplacement2;

  LiveGraph.pressureData.push([+t, +pressure]);
  LiveGraph.d1Data.push([+t, +disp1]);
  LiveGraph.d2Data.push([+t, +disp2]);

  if( 
    ( LiveGraph.pressureData.length >= maxNumberOfPointsToShow ) ||
    ( LiveGraph.d1Data.length >= maxNumberOfPointsToShow ) ||
    ( LiveGraph.d2Data.length >= maxNumberOfPointsToShow )
    )
  {
    LiveGraph.pressureData.shift();
    LiveGraph.d1Data.shift();
    LiveGraph.d2Data.shift();
  }

  LiveGraph.render();
}

// NOTE: Event subscription is not automatic. When antother value subscribes to live data, that automatically starts graphing.
//       The graph automatically unsubscribes when all other subscribers stop.
//subscribeToLiveDataRequester('graph', ['secsToday'], updateGraph);

/*  *****     *****     *****     ***** /LIVE GRAPH HOOKUP / REGISTRATION *****     *****     *****     *****  */
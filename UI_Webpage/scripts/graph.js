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
          .ease(d3.easeCubicOut)
      .delay(transitionDelay)
      .attr("transform", "translate(0," + height + ")")
      .call(d3.axisBottom(xScale));

    // Render the pressure axis
    var pMax = d3.max(this.pressureData, d => d[1]);
    var pMin = d3.min(this.pressureData, d => d[1]);
    var pRange = pMax - pMin;
    var pScale = d3.scaleLinear()
      .domain([pMin - (pRange * marginRatio), pMax + (pRange * marginRatio)])
      .range([height, 0]);
    pAxis
      .transition()
          .ease(d3.easeCubicOut)
      .delay(transitionDelay)
      .call(d3.axisLeft(pScale));

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
        .ease(d3.easeCubicOut)
      .delay(transitionDelay)
      .attr("transform", "translate("+width+",0)")
      .call(d3.axisRight(dScale));

    // Render the data paths
    const  line = d3.line().x(d => xScale(d[0])).y(d => pScale(d[1]));
    const dline = d3.line().x(d => xScale(d[0])).y(d => dScale(d[1]));

    pPath.data([this.pressureData])
        .transition()
          .ease(d3.easeCubicOut)
          .delay(transitionDelay)
      .attr("d", line)
      .style("fill", "none")
      .style("stroke", "steelblue")
      .style("stroke-width", "3px");

    d1Path.data([this.d1Data])
      .transition()
        .ease(d3.easeCubicOut)
        .delay(transitionDelay)
      .attr('d', dline)
      .style("fill", "none")
      .style("stroke", "#AA2222")
      .style("stroke-width", "3px");

    d2Path.data([this.d2Data])
      .transition()
        .ease(d3.easeCubicOut)
        .delay(transitionDelay)
      .attr('d', dline)
      .style("fill", "none")
      .style("stroke", "#AA2222")
      .style("stroke-width", "3px");
  }

LiveGraph.render();

$(window).resize(() => LiveGraph.render() );

/*  *****     *****     *****     ***** /LIVE GRAPH SETUP *****     *****     *****     *****  */





/*  *****     *****     *****     ***** LIVE GRAPH HOOKUP / REGISTRATION *****     *****     *****     *****  */

var graphUpdater = function(){
  var refreshImage = $('#w6').find('.refresh>img');
  refreshImage.unbind('mouseenter mouseleave');
  refreshImage.attr('src', './assets/spinner.gif');

  $.get(rootUrl + '/get/pressure',
      function(data){
        $('#w6').find('input').val(data.pressure);
        
        LiveGraph.pressureData.push([+data.millis, +data.pressure]);
        LiveGraph.d1Data.push([+data.millis, (+data.pressure)/5-5]);
        LiveGraph.d2Data.push([+data.millis, (+data.pressure)/6]);

        if(LiveGraph.pressureData.length >= 50) LiveGraph.pressureData.shift();
        if(LiveGraph.d1Data.length >= 50) LiveGraph.d1Data.shift();
        if(LiveGraph.d2Data.length >= 50) LiveGraph.d2Data.shift();

        LiveGraph.render();

      }).done(function(){
        refreshImage.attr('src', './assets/Refresh2.png'); 
        refreshImage.hover(refreshHoverOn, refreshHoverOff);
      });
};

/*  *****     *****     *****     ***** /LIVE GRAPH HOOKUP / REGISTRATION *****     *****     *****     *****  */
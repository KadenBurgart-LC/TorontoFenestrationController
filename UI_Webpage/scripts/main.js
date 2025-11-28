/*  *****     *****     *****     *****  UI INTERAVTIVITY  *****     *****     *****     *****  */

var rootUrl = 'http://192.168.1.177';

// LIVE TEXT FIELD
$('.go>img').hover(
    function(){ $(this).attr("src", "./assets/Go2.png"); },
    function(){ $(this).attr("src", "./assets/Go.png"); }
  );


// INDICATORS
var indicatorRefreshHandler = function(refreshImg){
    refreshButtonLoadingMode(refreshImg);

    var w = refreshImg.closest('.widget');
    var id = w.attr('id');
    var img = w.find('.indicator > img');
    
    $.get(rootUrl + '/get/' + id,
        function(data){
          if(data == "1") img.attr('src', './assets/warnGif.gif');
          else img.attr('src', './assets/Warn.png');
        })
        .fail(function(){
          img.attr('src', './assets/warnGif.gif');
        })
        .always(function(){
          refreshButtonReadyMode(refreshImg);
        });
  };
$('.indicator').closest('.widget').find('.refresh>img').click(function(){ indicatorRefreshHandler($(this)); });

// INDIVIDUAL WIDGET BEHAVIORS
$('#currentAction').find('input').val("Ready");
$('#currentActionTimer').find('input').val("0 s");

/*  *****     *****     *****     ***** /UI INTERAVTIVITY  *****     *****     *****     *****  */



/*  *****     *****     *****     *****  PAGE UPDATE INTERVALS  *****     *****     *****     *****  */



var ajaxTester = function(){
  $.get(rootUrl + '/get/pressure',
    function(data){
      console.log(data);
      console.log(data.pressure);
    });
};

var pingServer = function(){
  $.get(rootUrl,
    function(data){
      console.log(data);
    });
};

//var puI = setInterval(pressureUpdater, 500);

/*  *****     *****     *****     ***** /PAGE UPDATE INTERVALS  *****     *****     *****     *****  */

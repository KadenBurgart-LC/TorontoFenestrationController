// NOTE: There is documentation for how this widget is supposed to work. Read the documentation. It's in the README.md within the UI_Webpage folder.


var alarmRefreshHandler = function(refreshImg){
    refreshButtonLoadingMode(refreshImg);

    var w = refreshImg.closest('.widget');
    var id = w.attr('id');
    var img = w.find('.indicator > img');
    
    $.get(rootUrl + '/w/' + id,
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

$('.alarm').closest('.widget').find('.refresh>img').click(function(){ alarmRefreshHandler($(this)); }); // BIND the alarm refresh event

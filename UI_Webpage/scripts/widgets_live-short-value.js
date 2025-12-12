// NOTE: There is documentation for how this widget is supposed to work. Read the documentation. It's in the README.md within the UI_Webpage folder.


// Go button hover
$('.go>img').on('mouseenter', function(){ $(this).attr("src", "./assets/Go2.png"); });
$('.go>img').on('mouseleave', function(){ $(this).attr("src", "./assets/Go.png"); });


// Refresh handler
var liveShortValueRefreshHandler = function(refreshImg){
      refreshButtonLoadingMode(refreshImg);

      var w = refreshImg.closest('.widget');
      var id = w.attr('id');
      var textField = w.find('input');

      $.get(rootUrl + '/' + id,
        function(data){
          textField.val(data);
        })
        .fail(function(){
          textField.val("ERROR");
        })
        .always(function(){
          refreshButtonReadyMode(refreshImg);
        });
  };

// Bind the refresh button click to the refresh handler
$('.live-short-value').find('.refresh>img').on('click', function(){ liveShortValueRefreshHandler($(this)); });



// Go/pause button 
var liveShortValueGoPauseClickHandler = function(goImg){
    var w = goImg.closest('.widget');
    var refreshImg = w.find('.refresh>img');
    var id = w.attr('id');
    var textField = w.find('input');

    var currentButtonImageSrc = goImg.attr('src');

    var alreadyPolling = currentButtonImageSrc.includes('Pause');

    if(alreadyPolling){
        unsubscribeFromLiveDataRequester(id);

        refreshButtonReadyMode(refreshImg);
        refreshImg.on('click', function(){ liveShortValueRefreshHandler(refreshImg) });
        
        goImg.off('mouseenter mouseleave');
        goImg.attr('src', './assets/Go.png');
        goImg.on('mouseenter', function(){ goImg.attr('src', './assets/Go2.png'); });
        goImg.on('mouseleave', function(){ goImg.attr('src', './assets/Go.png'); });
    }
    else{
        refreshButtonLoadingMode(refreshImg);
        refreshImg.off('click');
        
        goImg.off('mouseenter mouseleave');
        goImg.attr('src', './assets/Pause.png');
        goImg.on('mouseenter', function(){ goImg.attr('src', './assets/Pause2.png'); });
        goImg.on('mouseleave', function(){ goImg.attr('src', './assets/Pause.png'); });

        subscribeToLiveDataRequester(id, function(data){
            if(data[id] !== undefined) textField.val(data[id]);
        });
    }
}
$('.live-short-value').find('.go>img').on('click', function(){ liveShortValueGoPauseClickHandler($(this)); } );

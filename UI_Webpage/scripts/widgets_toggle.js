// TOGGLE SWITCHES
var toggleClickHandler = function(toggle){
    var slider = toggle.find(".slide");
    var w = slider.closest(".widget");
    var refreshImage = w.find('.refresh>img');
    var id = w.attr('id');

    refreshButtonLoadingMode(refreshImage);

    // This function runs after the AJAX call completes
    var callbackFunction = function() { 
      refreshButtonReadyMode(refreshImage);

      if(id == "w2") textFieldRefreshHandler($('#w5>div>div>img'));
    };

    // TURN ON 
    if(slider.css("left") == "15px") { 
      slider.animate({left: '35px'}, 150, function(){
          $.post(rootUrl + '/set/' + id,
              { setTo: 1 }
            ).done(function(data){
              if(data == "1") slider.animate({left: '55px'}, 150, callbackFunction);
              else slider.animate({left: '15px'}, 150, callbackFunction);
            })
            .always(function(){
              refreshButtonReadyMode(refreshImage);
            });
        });        
    }
    
    // TURN OFF (if the slider is stuck somewhere, we will treat a click as a command to turn off)
    else /*if($(this).find(".slide").css("left") == "55px")*/ { 
      slider.animate({left: '35px'}, 150, function(){
          $.post(rootUrl + '/set/' + slider.closest(".widget").attr('id'),
              { setTo: 0 }
            ).done(function(data){
              if(data == "1") slider.animate({left: '55px'}, 150, callbackFunction);
              else slider.animate({left: '15px'}, 150, callbackFunction);
            })
            .always(function(){
              refreshButtonReadyMode(refreshImage);
            });
        });
    }
  };
$('.toggle').click(function() { toggleClickHandler($(this)); });

var toggleRefreshHandler = function(refreshImg){
    refreshButtonLoadingMode(refreshImg);

    var w = refreshImg.closest('.widget');
    var id = w.attr('id');
    var slide = w.find('.slide');

    var callbackFunction = function() { 
        refreshButtonReadyMode(refreshImg);
      };

    slide.animate({left: '35px'}, 150, function(){
      $.get(rootUrl + '/get/' + id,
        function(data){
          if(data == "1") slide.animate({left: '55px'}, 150, callbackFunction);
          else slide.animate({left: '15px'}, 150, callbackFunction);
        })
        .always(function(){
          refreshButtonReadyMode(refreshImg);
        });
    });
  };
$('.toggle').closest('.widget').find('.refresh>img').click(function(){ toggleRefreshHandler($(this)); });
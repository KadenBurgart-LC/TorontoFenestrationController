// NOTE: There is documentation for how this widget is supposed to work. Read the documentation. It's in the README.md within the UI_Webpage folder.

var toggleClickHandler = function(toggle){
    var slider = toggle.find(".slide");
    var sit = toggle.find(".sit");
    var w = slider.closest(".widget");
    var refreshImage = w.find('.refresh>img');
    var id = w.attr('id');

    refreshButtonLoadingMode(refreshImage);

    // This function runs after the AJAX call completes
    var callbackFunction = function() { 
      refreshButtonReadyMode(refreshImage);

      //if(id == "w2") textFieldRefreshHandler($('#w5>div>div>img')); // I have no idea what this used to do
    };

    // TURN ON 
    if(slider.css("left") == "15px") { 
      slider.animate({left: '35px'}, 150, function(){
          $.ajax({
              url: rootUrl + '/w/' + id,
              type: 'POST',
              data: { setTo: 1 }
            }).done(function(data){
              if(data == "1") slider.animate({left: '55px'}, 150, callbackFunction);
              else if (data == "0") slider.animate({left: '15px'}, 150, callbackFunction);
              
              slider.removeClass('slideError');
              sit.removeClass('sitError');
            }).fail(function(data){
              slider.addClass('slideError');
              sit.addClass('sitError');
            }).always(function(){
              refreshButtonReadyMode(refreshImage);
            });
        });        
    }
    
    // TURN OFF (if the slider is stuck somewhere, we will treat a click as a command to turn off)
    else { 
      slider.animate({left: '35px'}, 150, function(){
          $.ajax({
              url: rootUrl + '/w/' + id,
              type: 'POST',
              data: { setTo: 0 }
            }).done(function(data){
              if(data == "1") slider.animate({left: '55px'}, 150, callbackFunction);
              else if (data == "0") slider.animate({left: '15px'}, 150, callbackFunction);
              
              slider.removeClass('slideError');
              sit.removeClass('sitError');
            }).fail(function(data){
              slider.addClass('slideError');
              sit.addClass('sitError');
            })
            .always(function(){
              refreshButtonReadyMode(refreshImage);
            });
        });
    }
  };
$('.toggleSlider').click(function() { toggleClickHandler($(this)); });  // BIND THE CLICK EVENT




var toggleRefreshHandler = function(refreshImg){
    refreshButtonLoadingMode(refreshImg);

    var w = refreshImg.closest('.widget');
    var id = w.attr('id');
    var slide = w.find('.slide');

    var callbackFunction = function() { 
        refreshButtonReadyMode(refreshImg);
      };

    slide.animate({left: '35px'}, 150, function(){
      $.get(rootUrl + '/w/' + id,
        function(data){
          if(data == "1") slide.animate({left: '55px'}, 150, callbackFunction);
          else slide.animate({left: '15px'}, 150, callbackFunction);
        })
        .always(function(){
          refreshButtonReadyMode(refreshImg);
        });
    });
  };
$('.toggle').closest('.widget').find('.refresh>img').click(function(){ toggleRefreshHandler($(this)); });  // BIND THE REFRESH EVENT
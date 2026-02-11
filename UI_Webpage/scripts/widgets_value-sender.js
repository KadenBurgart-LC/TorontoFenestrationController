// NOTE: There is documentation for how this widget is supposed to work. Read the documentation. It's in the README.md within the UI_Webpage folder.

// NOTE: Some of the refresh button behavior is in widgets_common.js

$('.send>img').hover(
    function(){ $(this).attr("src", "./assets/Send2.png"); },
    function(){ $(this).attr("src", "./assets/Send.png"); }
  );

$('.send>img').click(function(){
    var w = $(this).closest('.widget');
    var id = w.attr('id');
    var refreshImage = w.find('.refresh>img');
    var textField = w.find('input');

    refreshButtonLoadingMode(refreshImage);

    $.post(rootUrl + '/' + id, { 'value': textField.val() })
      .done(function(data){
        textField.val(data);
      })
      .fail(function(){
        textField.val("ERROR");
      })
      .always(function(){
        refreshButtonReadyMode(refreshImage);
      });
  });

// Trigger send button on Enter key press
$('.value-sender').find('input').on('keypress', function(e){
    if(e.which === 13){ // Enter key
      $(this).closest('.widget').find('.send>img').trigger('click');
    }
  });

// Auto-select text when clicking on input field
$('.value-sender').find('input').on('click focus', function(){
    $(this).select();
  });


// Refresh handler
var valueSenderRefreshHandler = function(refreshImg){
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
$('.value-sender').find('.refresh>img').click(function(){ valueSenderRefreshHandler($(this)); });



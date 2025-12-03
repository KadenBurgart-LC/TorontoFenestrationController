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

    $.post(rootUrl + '/w/' + id, { 'value': textField.val() })    
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


// Refresh handler
var valueSenderRefreshHandler = function(refreshImg){
      refreshButtonLoadingMode(refreshImg);

      var w = refreshImg.closest('.widget');
      var id = w.attr('id');
      var textField = w.find('input');

      $.get(rootUrl + '/w/' + id,
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
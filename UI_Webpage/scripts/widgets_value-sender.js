// VALUE SENDER WIDGET
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

    $.post(rootUrl + '/set/' + id, { 'value': textField.val() })    
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
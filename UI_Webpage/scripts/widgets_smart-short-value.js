// SMART-SHORT-VALUE WIDGETS
var textFieldRefreshHandler = function(refreshImg){
      refreshButtonLoadingMode(refreshImg);

      var w = refreshImg.closest('.widget');
      var id = w.attr('id');
      var textField = w.find('input');

      $.get(rootUrl + '/get/' + id,
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

$('.textField').closest('.widget').find('.refresh>img').click(function(){ textFieldRefreshHandler($(this)); });

// NOTE: There is documentation for how this widget is supposed to work. Read the documentation. It's in the README.md within the UI_Webpage folder.

// SMART-SHORT-VALUE WIDGETS
var smartLabelValueRefreshHandler = function(refreshImg){
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

$('.smart-label-value').find('.refresh>img').click(function(){ smartLabelValueRefreshHandler($(this)); });

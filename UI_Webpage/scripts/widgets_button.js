// NOTE: There is documentation for how this widget is supposed to work. Read the documentation. It's in the README.md within the UI_Webpage folder.

// Hover behavior
$('.buttonDiv').on('mouseenter', function(){ $(this).addClass('hover'); });
$('.buttonDiv').on('mouseleave', function(){ $(this).removeClass('hover'); });

// Loader behavior
var loaderLoadingMode = function(loaderImg){
	loaderImg.attr('src', './assets/spinner.gif');
}
var loaderReadyMode = function(loaderImg){
	loaderImg.attr('src', './assets/Ready.png')
}

// Widget-specific post-AJAX callbacks
// Register callbacks in viewModel_index.js using: buttonWidgetCallbacks_ajaxReceived['widgetId'] = function(data) { ... }
var buttonWidgetCallbacks_ajaxReceived = {};

var buttonClickHandler = function(buttonDiv){
	var w = buttonDiv.closest('.widget');
	var loaderImg = w.find('.loader>img');
	var id = w.attr('id');
	var textField = w.find('input');

	loaderLoadingMode(loaderImg);

	textField.val("... ... ...");

	$.get(rootUrl + '/' + id,
		function(data){
			textField.val(data);
		})
		.fail(function(){
			textField.val("ERROR");
		})
		.always(function(data){
			loaderReadyMode(loaderImg);

			// Execute widget-specific callback if registered
			if(buttonWidgetCallbacks_ajaxReceived[id]){
				buttonWidgetCallbacks_ajaxReceived[id](data);
			}
		});
}
$('.buttonDiv').on('click', function(){ buttonClickHandler($(this)); });
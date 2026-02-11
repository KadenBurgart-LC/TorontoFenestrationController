var vm_index = {
	RefreshAll: function(){
		smartLabelValueRefreshHandler($('#wRTC').find('.refresh>img'));
		valueSenderRefreshHandler($('#wTargetPressure').find('.refresh>img'));
		smartShortValueRefreshHandler($('#wTmpAmb').find('.refresh>img'));
		liveShortValueRefreshHandler($('#wMedPressure').find('.refresh>img'));
		liveShortValueRefreshHandler($('#wHighPressure').find('.refresh>img'));
		liveShortValueRefreshHandler($('#wDisplacement1').find('.refresh>img'));
		liveShortValueRefreshHandler($('#wDisplacement2').find('.refresh>img'));
		
		liveShortValueRefreshHandler($('#wLowPressure').find('.refresh>img'));
		toggleRefreshHandler($('#wHPvalvesDirection').find('.refresh>img'));
		toggleRefreshHandler($('#wLPvalvesDirection').find('.refresh>img'));
		toggleRefreshHandler($('#wWaterPump').find('.refresh>img'));
		toggleRefreshHandler($('#wHpBlower').find('.refresh>img'));
		toggleRefreshHandler($('#wLpBlower').find('.refresh>img'));

		longLiveValueRefreshHandler($('#wLastLogEntry').find('.refresh>img'));

		valueSenderRefreshHandler($('#wV1TargetPos').find('.refresh>img'));
		liveShortValueRefreshHandler($('#wV1Pos').find('.refresh>img'));
	}
};



// Register callback for wSTOP_ALL button widget
// This callback executes after the AJAX response is received
buttonWidgetCallbacks_ajaxReceived['wSTOP_ALL'] = function(data){
	toggleRefreshHandler($('#wWaterPump').find('.refresh>img'));
	toggleRefreshHandler($('#wHpBlower').find('.refresh>img'));
	toggleRefreshHandler($('#wLpBlower').find('.refresh>img'));
};

// When we set the position of this valve, let's start monitoring the position feedback signal
$('#wV1TargetPos .send>img').click(function(){
	var goImg = $('#wV1Pos .go>img');
	var alreadyPolling = goImg.attr('src').includes('Pause');

	if(!alreadyPolling) goImg.trigger('click');
});

vm_index.RefreshAll();

var scheduleRefreshAll = setInterval(vm_index.RefreshAll, 1000 * 60 * 3);
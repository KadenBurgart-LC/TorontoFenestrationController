/*  *****     *****     *****     *****  UI INTERAVTIVITY  *****     *****     *****     *****  */

var rootUrl = 'http://192.168.1.177';


// INDIVIDUAL WIDGET BEHAVIORS
$('#wCurrentAction').find('input').val("Ready");
$('#wCurrentActionTimer').find('input').val("0 s");
$('#wExample_labelValue').find('input').val("Label value example data field");

/*  *****     *****     *****     ***** /UI INTERAVTIVITY  *****     *****     *****     *****  */



/*  *****     *****     *****     *****  PAGE UPDATE INTERVALS  *****     *****     *****     *****  */

/* Fields that need to get live updates register themselves in this object.
 * 
 * STRUCTURE:
 * {
 *   'variableKey1' : callbackFunction1,
 *   'variableKey2' : callbackFunction2
 * }
 *
 * To subscribe:
 * subscribeToLiveDataRequester('myKey', myCallbackFunction);
 *
 * The callback function for subscribers needs to be able to receive and process the response data object.
 *
 * The STRUCTURE of the data object that gets passed to each callback function:
 * {
 *   'variableKey1' : data1fromServer,
 *   'variableKey2' : data2fromServer,
 * }
 *
 * To unsubscribe:
 * unsubscribeFromLiveDataRequester('myKey');
 *
 * TODO: Change the behavior so that multiple subscribers can register to look at the same key, but the server
 *       only gets asked for each key's value once, and the subscribers only get the value of the one key they
 *       subscribed to instead of getting the whole response object.
 */
const liveDataSubscribers = {};

function subscribeToLiveDataRequester(keyStr, callbackFunc){
  if(liveDataSubscribers[keyStr] == undefined){
    liveDataSubscribers[keyStr] = callbackFunc;
  }
  else {
    console.warn('LiveDataRequester: ${keyStr} attempted to subscribe to the LiveDataRequester, but they are already subscribed.');
  }
}

function unsubscribeFromLiveDataRequester(keyStr){
  if(liveDataSubscribers[keyStr] !== undefined){
    delete liveDataSubscribers[keyStr];
  }
  else {
    console.warn('LiveDataRequester: ${keyStr} attempted to unsubscribe from the LiveDataRequester, but they are not in the subscriber list.');
  }
}

var liveDataRequester_TimerEventHandler = function(){
  const keysToRequest = Object.keys(liveDataSubscribers);

  if (keysToRequest.length == 0) return;

  $.post(rootUrl + '/liveDataPacketRequest',
      JSON.stringify(keysToRequest),
      function(data){
        for (const subscriber in liveDataSubscribers) {
          const callbackFunc = liveDataSubscribers[subscriber];

          if(callbackFunc) callbackFunc(data);
        }
      },
      'json'
    )
    .fail(function(jqXHR, textStatus, errorThrown){
      console.error("LiveDataRequester: Live data packet request failed.");
    });
}

var liveDataRequester_TimerEvent = setInterval(liveDataRequester_TimerEventHandler, 1000);

function rtcUpdater_TimerEventHandler(){
  $('#wRTC').find('.refresh>img').trigger('click');
}

var rtcUpdater_TimerEvent = setInterval(rtcUpdater_TimerEventHandler, 30000);

/*  *****     *****     *****     ***** /PAGE UPDATE INTERVALS  *****     *****     *****     *****  */






/*  *****     *****     *****     *****   RANDOM UTILITIES  *****     *****     *****     *****  */

var testData = undefined;

var testLiveDataSubscriber = function(data){
  testData = data;
}

//subscribeToLiveDataRequester("test", testLiveDataSubscriber);
//subscribeToLiveDataRequester("wExample_liveShortValue", testLiveDataSubscriber);
//subscribeToLiveDataRequester("millis", testLiveDataSubscriber);

var pingServer = function(){
  $.get(rootUrl,
    function(data){
      console.log(data);
    });
};

var ajaxTester = function(){
  $.get(rootUrl + '/get/pressure',
    function(data){
      console.log(data);
      console.log(data.pressure);
    });
};

var testToggleOn = function(){
  $.ajax({
      url: rootUrl + '/w/wExample_toggle',
      type: 'POST',
      data: { setTo: 1 }
    }).done(function(data){
      if(data == "1") slider.animate({left: '55px'}, 150, callbackFunction);
      else slider.animate({left: '15px'}, 150, callbackFunction);
    })
    .always(function(){
      alert('done');
    });
};

/*  *****     *****     *****     ***** / RANDOM UTILITIES  *****     *****     *****     *****  */
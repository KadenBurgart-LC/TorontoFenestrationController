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
 * liveDataSubscribers = {
 *   'subscriberKey1': {
 *       'values': [
 *            'variableKey1',
 *            'variableKey2',
 *            .
 *            .
 *            .
 *       ],
 *       'callbackFunc': callbackFunction1
 *   },
 *   'subscriberKey2': {
 *       'values': [
 *            'variableKey1',
 *            'variableKey2',
 *            .
 *            .
 *            .
 *       ],
 *       'callbackFunc': callbackFunction1
 *   },
 *   .
 *   .
 *   .
 * }
 *
 * To subscribe:
 * subscribeToLiveDataRequester('subscriberKey', ['variableKey1, variableKey2, ...'], myCallbackFunction);
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
 * unsubscribeFromLiveDataRequester('subscriberKey');
 *
 */
const liveDataSubscribers = {};

function subscribeToLiveDataRequester(subscriberKey, vals, func){
  if(liveDataSubscribers[subscriberKey] == undefined){
    liveDataSubscribers[subscriberKey] = {
      'values': vals,
      'callbackFunc': func
    };
  }
  else {
    console.warn('LiveDataRequester: ${subscriberKey} attempted to subscribe, but they are already subscribed.');
  }
}

function unsubscribeFromLiveDataRequester(subscriberKey) {
  if (liveDataSubscribers[subscriberKey]) {
    delete liveDataSubscribers[subscriberKey];
  } else {
    console.warn(`LiveDataRequester: Attempted to unsubscribe ${subscriberKey}, but no such subscriber exists.`);
  }
}

var liveDataRequester_TimerEventHandler = function(){
  // The Set() type prevents duplicate entries
  const allRequestedVars = new Set();
  for (const subKey in liveDataSubscribers) {
    liveDataSubscribers[subKey].values.forEach(v => allRequestedVars.add(v));
  }

  if (allRequestedVars.size === 0) return;
  //allRequestedVars.add('secsToday');

  const variableKeysArray = Array.from(allRequestedVars);

  $.post(rootUrl + '/liveDataPacketRequest',
      JSON.stringify(variableKeysArray),
      function(data){
        for (const subscriberKey in liveDataSubscribers) {
          const callback = liveDataSubscribers[subscriberKey].callbackFunc;

          if(callback){
            callback(data);
          }
        }
      },
      'json'
    )
    .fail(function(jqXHR, textStatus, errorThrown){
      console.error("LiveDataRequester: Live data packet request failed.");
    });
};

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
      url: rootUrl + '/wExample_toggle',
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
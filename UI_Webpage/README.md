# Fenestration Conntroller UI WebApp
This web app is made up of a bunch of widgets and a graph. The widgets go in a grid of two columns by however many rows are needed. 

## Installation
The server cannot currently serve this UI web app, so you have to download it from the GitHub and run it locally. Download this folder (https://github.com/KadenBurgart-LC/TorontoFenestrationController/tree/main/UI_Webpage) somewhere on your local machine and run it by opening index.html in the browser of your choice. The web app was developed using FireFox as the test browser, so that's the safest option.

## Page structure
First there is a #pageContainer div that adds 100px of spacing to either side of the main grid, and limits the minimum internal width to 800px while allowing it to expand to fill unused space. Within that is the main grid container #mainGrid, which has two columns and an unspecified number of rows. The 'cells' of this grid are intended to host widgets, which can get hooked up to client-side javascript tools, and often call web requests to the server hosted on the P1AM-200 controller.

## Notes on widgets
Some information common to all widgets...  
The ID of a widget is used to generate the PATH for the HTTP messages of the AJAX calls it makes, so the ID names are important, since the server will only be able to recognize and serve certain specific paths that are coded in. The ID of all widgets should begin with a lower-case "w", and should follow rules for naming c++ functions (e.g. no dashes allowed), since the c++ route handler functions will be named after their associated widget IDs.

## Types of widgets

### Button widgets
These widgets do something when you click on them. There is a dataField next to the button so that the computer can provide feedback on the action it's trying to perform.

### Smart-label-value widgets
These behave the same way as smart-short-value widgets, except the label goes in the little field on the left, and the data goes in the bigger field on the right. Also, the refresh button goes on the right hand side.

```html
<div class="example widget" id="wExample_valueSender">
  <div class="smart-label-value">
    <div class="label">Label Value</div>
    <div class="dataField"><input type="text" /></div>
    <div class="refresh"><img src="./assets/Refresh2.png"/></div>
  </div>
</div>
```

### Live-short-value widgets
These widgets provide fields are used to display things like live sensor values that don't take up too many characters and need to be updated once per second.
These widgets have...
* a refresh button
* a small data field (110px wide) for the server data to be displayed in
* a go/pause button 
* a label field that fills the rest of the widget space.

The **behavior** of this widget is as follows...
* Live short value widgets are dormant when the page loads, and they display a refresh button to the left of the data field and a 'go' (play) button to the right of the data field.
* The refresh button can be clicked to request the current value of the field from the Fenestration Wall, and put the result in the data field. When the refresh button is clicked, it will turn into a spinner to indicate that it's sending the request and waiting for a response. When a response is received, the spinner will turn back into a refresh button and display the received data in the data field.
* When the go button is pressed, the widget will...
  * Turn the go (play) button into a pause button.
  * Turn the refresh button into a spinner to indicate that the field is consistently refreshing automatically.
  * Register itself in the 'polling' event that grabs live data from the Fenestration Wall.
    * This registration adds the fields related data key to the data request that goes to the server.
    * This registration also adds a callback function so that the widget's data field gets updated when the response comes back from the server. If the callback function says that there was an error with the request, the data field will show "ERROR".
* When the pause button is pressed, the widget will...
  * Turn the pause button back into a go button.
  * Turn the spinner back into a refresh button.
  * Unregister from the polling event.
* The refresh button has a hover event to make it clear it's a button.
* The go and pause buttons have hover events to make it clear they are buttons.
* Sometimes some client-side scripting might force this widget to refresh, go live, or go dormant without the user actually clicking one of the widget buttons. For example, if the user clicks a widget button to start a water penetration test, the CurrentPressure widget will go live without the user explicitly asking it to.

The HTML for a live-short-value widget looks like this...
```html
<div class="widget" id="wExample_liveShortValue">
  <div class="live-short-value">
    <div class="refresh"><img src="./assets/Refresh2.png"/></div>
    <div class="dataField"><input type="text" disabled="" /></div>
    <div class="goBtn"><img src="./assets/Go.png"/></div>
    <div class="label">Current Pressure</div>
  </div>
</div>
```

The Javascript for live-short-value widgets...
* The refresh button hover effect is bound in widgets_common.js.
* The tools for turning the refresh button into a spinner and back are in widgets_common.js.
* The refresh button click is handled in widgets_live-short-value.js.
* The go/pause button click event is in widgets_live-short-value.js.
* When the user clicks the GO button, this widget subscribes to a global timer event that consolidates all of the live-update requests into a single JSON POST. When the server replies with a JSON dictionary of values, each subscriber's callback function is called with a copy of the server data. The global timer live data subscription system is in main.js, and the live-short-value's specific implementation is inside the click event for the go/pause button in widgets_live-short-value.js.

### Label-value widgets
These are basic text display fields with no inherant automation behavior. They rely on other parts of the web app to update their values.
These widgets have...
* a small label field (145px)
* a medium data field with text that gets set by other parts of the application.

The HTML for a label-value widget looks like this...
```html
<div class="widget" id="wExample_labelValue">
  <div class="label-value">
    <div class="label">Label Value</div>
    <div class="dataField"><input type="text" disabled="" /></div>
  </div>
</div>
```

### Smart-short-value widgets
These fields are good for requesting numerical or other short values from the Fenestration Wall that don't necessarilly need to be updated each second.
These widgets have...
* a refresh button
* a small (110px) data display field
* a label field that fills the rest of the available space

The **behavior** of this widget is as follows...
* When the refresh button is clicked... 
  * The refresh button will turn into a spinner.
  * A request will be sent to the Fenestration Wall to ask the controller what that field value should be.
  * When the server responds, the data will be written to the data field, or will show "ERROR" if the response comes back with some error. Then, the spinner will be turned back into a refresh button.
* The refresh button has a hover-highlight event to make it clear it's a button.

The HTML for a smart-short-value widget looks like this...
```html
<div class="widget" id="wExample_smartShortValue">
  <div class="smart-short-value">
    <div class="refresh"><img src="./assets/Refresh2.png"/></div>
    <div class="dataField"><input type="text" disabled="" /></div>
    <div class="label">Example smart-short-value Label</div>
  </div>
</div>  
```

### Toggle widgets
Toggle widgets tell the Fenestration Wall to turn things on and off, or check to see what things are on or off.
These widgets have...
* a refresh button
* a toggle slider
* a label field that fills the rest of the available space

The **behavior** of this widget is as follows...
* TODO: write this shit.
* TODO: turn the slider red when there is an error.

The HTML for a toggle widget looks like this...
```html
<div class="example widget" id="wExample_toggle">
  <div class="toggle">
    <div class="refresh"><img src="./assets/Refresh2.png" /></div>
    <div class="toggleSlider"><center><div class="sit"></div><div class="slide"></div></center></div>
    <div class="label">Example toggle Label</div>
  </div>
</div>  
```

### Value-sender widgets
These fields let you send a short data field to the Fenestration Wall, or press a refresh button to retrieve the Fenestration Wall's current RAM value for that field.
These widgets have...
* a refresh button
* a small (110px) data input field
* a 'send' button
* a label field that fills the rest of the available space

The **behavior** of this widget is as follows...
* When the 'send' button is clicked...
  * The refresh button turns into a spinner.
  * A POST request is sent to the Fenestration Wall to update the controller's value.
  * The server responds with the data it received and wrote to its memory. This value gets written to the data field in the widget, and the spinner is turned back into a refresh button.
  * IF the server responds with an error, "ERROR" is written to the widget's data field.
* When the refresh button is clicked...
  * The refresh button is turned into a spinner.
  * A GET request is sent to the Fenestration Wall controller to get the existing value of the field on the controller.
  * When the server responds, the data will be written to the data field, or will show "ERROR" if the response comes back with some error, then the spinner will be turned back into a refresh button.
* The refresh and send buttons have hover-highlight events to make it clear they're buttons.

The HTML for a value-sender widget looks like this...
```html
<div class="widget" id="wExample_valueSender">
  <div class="value-sender">
    <div class="refresh"><img src="./assets/Refresh2.png"/></div>
    <div class="dataField"><input type="text" /></div>
    <div class="send"><img src="./assets/Send.png"/></div>
    <div class="label">Target Pressure (kPa)</div>
  </div>
</div>
```

### Alarm widgets
Alarm widgets let us know if there are any errors showing on the Fenestration Wall controller.


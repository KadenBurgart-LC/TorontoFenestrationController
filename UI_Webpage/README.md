# Fenestration Conntroller UI WebApp
This web app is made up of a bunch of widgets and a graph. The widgets go in a grid of two columns by however many rows are needed. 

## Page structure
First there is a #pageContainer div that adds 100px of spacing to either side of the main grid, and limits the minimum internal width to 800px while allowing it to expand to fill unused space. Within that is the main grid container #mainGrid, which has two columns and an unspecified number of rows. The 'cells' of this grid are intended to host widgets, which can get hooked up to client-side javascript tools, and often call web requests to the server hosted on the P1AM-200 controller.

## Types of widgets

### Live-short-text widgets
These widgets provide fields are used to display things like live sensor values that don't take up too many characters and need to be updated once per second.
These widgets have...
* a refresh button
* a small data field (110px wide) for the server data to be displayed in
* a go/pause button 
* a label field that fills the rest of the widget space.

The **behavior** of this widget is as follows...
* Live short text widgets are dormant when the page loads, and they display a refresh button to the left of the data field and a 'go' (play) button to the right of the data field.
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

The HTML for a live-short-text widget looks like this
```html
<div class="widget" id="CurrentPressure">
  <div class="live-short-text">
    <div class="refresh"><img src="./assets/Refresh2.png"/></div>
    <div class="dataField"><input type="text" disabled="" /></div>
    <div class="goBtn"><img src="./assets/Go.png"/></div>
    <div class="label">Current Pressure</div>
  </div>
</div>
```

### Label-value widgets
These are basic text display fields with no inherant automation behavior. They rely on other parts of the web app to update their values.
These widgets have...
* a small label field (145px)
* a medium data field with text that gets set by other parts of the application.

The HTML for a label-value widget looks like this
```html
<div class="widget" id="CurrentAction">
  <div class="label-value">
    <div class="label">Current Action</div>
    <div class="dataField"><input type="text" disabled="" /></div>
  </div>
</div>
```

### Smart-short-value widgets
These fields are good for requesting numerical or other short values from the Fenestration Wall that don't necessarilly need to be updated each second.
These widgets have...
* a refresh button
* a small (110px) data field
* a label field that fills the rest of the available space

The **behavior** of this widget is as follows...
* When the refresh button is clicked... 
  * The refresh button will turn into a spinner.
  * A request will be sent to the Fenestration Wall to ask the controller what that field value should be.
  * When the server responds, the data will be written to the data field, or will show "ERROR" if the response comes back with some error. Then, the spinner will be turned back into a refresh button.

The HTML for a smart-short-value widget looks like this
```html
<div class="widget" id="ExampleShortText">
  <div class="grid">
    <div class="refresh"><img src="./assets/Refresh2.png"/></div>
    <div class="textField"><input type="text" disabled="" /></div>
    <div class="label">Example Short Text</div>
  </div>
</div>  
```

### Value-sender widgets
These behave the same way as smart-short-value widgets, except the label goes in the little field on the left, and the data goes in the bigger field on the right. Also, the refresh button goes on the right hand side.

### Alarm widgets
Alarm widgets let us know if there are any errors showing on the Fenestration Wall controller.

### Toggle widgets
Toggle widgets tell the Fenestration Wall to turn things on and off, or check to see what things are on or off.
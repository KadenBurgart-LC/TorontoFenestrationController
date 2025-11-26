#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define RESPONSE_BUFFER_SIZE 4000
#define JSON_STRING_SIZE 200
#define LED_PIN 13

#define BLOWER_PIN 14
#define PUMP_PIN 15

#define PRESSURE_MAX 80
#define PRESSURE_MIN -80

typedef void (*Func) (void);

bool BlowerState = false;
bool PumpState = false;
bool NegativePressureState = false;

bool PropValve1Fault = false;
bool PropValve2Fault = true;

float BlowerCurrent = 16.4;
float CurrentPressure = 0;
float TargetPressure = 15.3;
float PropValve1Pos = 80;
float PropValve2Pos = 85;
char BlowerCurrentString[10];
char PressureDirection[10];
char tmpStr[50];

const char* SERVER_URI_NAME = "espPortal";

const char* WIFI_SSID = "LabTest";
const char* WIFI_PASS = "EmptGash164";

const char* ROOT_HTML = "<!DOCTYPE html><meta charset=\"utf-8\"><script src=\"https://code.jquery.com/jquery-3.6.0.min.js\"></script><script src=\"https://d3js.org/d3.v4.js\"></script><div id=\"my_dataviz\"></div><script>var margin={top: 10, right: 40, bottom: 30, left: 30}, width=450 - margin.left - margin.right, height=400 - margin.top - margin.bottom;var svG=d3.select(\"#my_dataviz\") .append(\"svg\") .attr(\"width\", width + margin.left + margin.right) .attr(\"height\", height + margin.top + margin.bottom) .append(\"g\") .attr(\"transform\", \"translate(\" + margin.left + \",\" + margin.top + \")\");%svar xMax=d3.max(dataSet, d=>d[0]);var xMin=d3.min(dataSet, d=>d[0]);var xRange=xMax - xMin;var xScale=d3.scaleLinear() .domain([xMin-(xRange/10), xMax+(xRange/10)]) .range([0, width]);svG .append('g') .attr(\"transform\", \"translate(0,\" + height + \")\") .call(d3.axisBottom(xScale));var yMax=d3.max(dataSet, d=>d[1]);var yMin=d3.min(dataSet, d=>d[1]);var yRange=yMax - yMin;var yScale=d3.scaleLinear() .domain([yMin-(yRange/10), yMax+(yRange/10)]) .range([height, 0]); svG .append('g') .call(d3.axisLeft(yScale));const line=d3.line() .x(d=> xScale(d[0])) .y(d=> yScale(d[1]));svG.append(\"path\") .data([dataSet]) .attr(\"d\", line) .style(\"fill\", \"none\") .style(\"stroke\", \"steelblue\") .style(\"stroke-width\", \"3px\");svG .selectAll(\"whatever\") .data(dataSet) .enter() .append(\"circle\") .attr(\"cx\", d=>xScale(d[0])) .attr(\"cy\", d=>yScale(d[1])) .attr(\"r\", 5) .style(\"fill\", d3.rgb(50,60,180));</script>";

char JsonDataString[JSON_STRING_SIZE];

AsyncWebServer Server(80);

void getString(AsyncWebServerRequest *request, char *str){
  Serial.println(str);
  respond(request, 200, "text/html", str);
}

void getFloat(AsyncWebServerRequest *r, float val, char *formatStr = "%.1f"){
  sprintf(tmpStr, formatStr, val);
  getString(r, tmpStr);
}

void getBool(AsyncWebServerRequest *r, bool val){
  strcpy(tmpStr, val ? "1" : "0");
  getString(r, tmpStr);
}

void setBlowerOn(AsyncWebServerRequest *request){
  digitalWrite(BLOWER_PIN, 1);
  BlowerState = true;
  respond(request, 200, "text/html", "1");
  Serial.println("Blower turned on.");
}

void setBlowerOff(AsyncWebServerRequest *request){
  digitalWrite(BLOWER_PIN, 0);
  BlowerState = false;
  respond(request, 200, "text/html", "0");
  Serial.println("Blower turned off.");
}

void setPumpOn(AsyncWebServerRequest *request){
  digitalWrite(PUMP_PIN, 1);
  PumpState = true;
  respond(request, 200, "text/html", "1");
  Serial.println("Pump turned on.");
}

void setPumpOff(AsyncWebServerRequest *request){
  digitalWrite(PUMP_PIN, 0);
  PumpState = false;
  respond(request, 200, "text/html", "0");
  Serial.println("Pump turned off.");
}

void setPressurePositive(AsyncWebServerRequest *request){
  digitalWrite(BLOWER_PIN, 0);
  switchNegativePressure(0);
  strcpy(PressureDirection, "Positive");
  respond(request, 200, "text/html", "0");
  Serial.println("Pressure direction set to positive.");
}

void setPressureNegative(AsyncWebServerRequest *request){
  digitalWrite(BLOWER_PIN, 1);
  switchNegativePressure(1);
  strcpy(PressureDirection, "Negative");
  respond(request, 200, "text/html", "1");
  Serial.println("Pressure direction set to negative.");
}

void setTargetPressure(AsyncWebServerRequest *r){
  int numParams = r->params();
  int targetParam = -1;

  for(int i=0; i<numParams; i++) if(strcmp(r->getParam(i)->name().c_str(), "targetPressure") == 0) targetParam = i;

  if(targetParam == -1){
    respond(r, 200, "text/html", "Parameter not found at endpoint. Check that the post data is formatted correctly.");
  }
  else{
    TargetPressure = atof(r->getParam(targetParam)->value().c_str());
    getFloat(r, TargetPressure);
  }
}

void getBlowerCurrent(AsyncWebServerRequest *request){
  BlowerCurrent = (float)(rand() % 200) / 10;
  getFloat(request, BlowerCurrent, "%.1f A");
}

void updateCurrentPressure(){
  // float delta = (float)(rand() % 200) / 10;

  // if(CurrentPressure > PRESSURE_MAX) CurrentPressure -= delta;
  // else if(CurrentPressure < PRESSURE_MIN) CurrentPressure += delta;
  // else CurrentPressure += delta - 10;

  CurrentPressure++;
  if(CurrentPressure >= PRESSURE_MAX) CurrentPressure = PRESSURE_MIN;   
}

void getCurrentPressure(AsyncWebServerRequest *r){
  updateCurrentPressure();
  getFloat(r, CurrentPressure, "%.1f kPa");
}

void respond(AsyncWebServerRequest *request, int code, char* type, char* body){ // All responses go through here so that we can handle cookies and such all in one place
  AsyncWebServerResponse *response = request->beginResponse(code, type, body);
  //response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

void switchNegativePressure(bool negativePressureOn){
  NegativePressureState = !NegativePressureState;
}


void toggleHandler(AsyncWebServerRequest *request, void (*turnOn)(AsyncWebServerRequest*), void (*turnOff)(AsyncWebServerRequest*)){
  int numParams = request->params();
  int setToParam = -1;

  int i=0;
  for(i; i<numParams; i++){
    // Serial.println(request->getParam(i)->name().c_str()); // Serial.println(request->getParam(i)->value().c_str());

    if(strcmp(request->getParam(i)->name().c_str(), "setTo") == 0){
      setToParam = i;

      if(strcmp(request->getParam(i)->value().c_str(), "1") == 0) {
        turnOn(request);
      }
      else if(strcmp(request->getParam(i)->value().c_str(), "0") == 0){
        turnOff(request);
      }
      else{
        respond(request, 400, "text/html", "Invalid parameter. Only 0 and 1 are allowed.");
      }
    }
  }

  if(setToParam == -1){
    respond(request, 200, "text/html", "Parameter not found at endpoint. Check that the post data is formatted correctly.");
  }
}

void URI_Root(AsyncWebServerRequest *request) {
  char temp[RESPONSE_BUFFER_SIZE];

  sprintf(temp, ROOT_HTML, RandomDataString(10));
  
  Serial.println(temp);
  request->send(200, "text/html", temp);
}

void URI_NotFound(AsyncWebServerRequest *request) {
  String message = "File Not Found\n\n";
  message += "URI: ";
  //message += Server.uri();
  message += "\nMethod: ";
  //message += (Server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  //message += Server.args();
  message += "\n";

  //for (uint8_t i = 0; i < Server.args(); i++) {
  //  message += " " + Server.argName(i) + ": " + Server.arg(i) + "\n";
  //}

  Serial.print("URL not found :");
  Serial.println(request->url());

  request->send(404, "text/plain", message);
}

void setup(void) {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BLOWER_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0);
  digitalWrite(BLOWER_PIN, 0);
  digitalWrite(PUMP_PIN, 0);

  strcpy(PressureDirection, "Positive");

  Serial.begin(115200);
  
  Serial.print("-------------------- BOOT INITIATED --------------------\nConnecting WiFi..");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\nConnected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //if (MDNS.begin(SERVER_URI_NAME)) { // So that we don't need to use the IP address in the client browser
  //  Serial.println("MDNS responder started");
  //}

  Server.on("/", URI_Root);

  Server.on("/get/pressure", [](AsyncWebServerRequest *r){ 
    updateCurrentPressure(); 
    sprintf(tmpStr, "{\"millis\":\"%i\",\"pressure\":\"%.1f\"}", millis(), CurrentPressure); 
    getString(r, tmpStr); 
  });

  Server.on("/set/w1",  [](AsyncWebServerRequest *r){ toggleHandler(r, setBlowerOn, setBlowerOff); });
  Server.on("/get/w1",  [](AsyncWebServerRequest *r){ getBool(r, BlowerState); });
  Server.on("/set/w2",  [](AsyncWebServerRequest *r){ toggleHandler(r, setPressureNegative, setPressurePositive); });
  Server.on("/get/w2",  [](AsyncWebServerRequest *r){ getBool(r, NegativePressureState); });
  Server.on("/set/w11", [](AsyncWebServerRequest *r){ toggleHandler(r, setPumpOn, setPumpOff); });
  Server.on("/get/w11", [](AsyncWebServerRequest *r){ getBool(r, PumpState); });
  Server.on("/get/w4",  [](AsyncWebServerRequest *r){ getBlowerCurrent(r); });
  Server.on("/get/w5",  [](AsyncWebServerRequest *r){ getString(r, PressureDirection); });
  Server.on("/get/w6",  [](AsyncWebServerRequest *r){ getCurrentPressure(r); });
  Server.on("/get/w12", [](AsyncWebServerRequest *r){ getFloat(r, PropValve1Pos, "%.1f%"); });
  Server.on("/get/w13", [](AsyncWebServerRequest *r){ getFloat(r, Pr0.
    opValve2Pos, "%.1f%"); });
  Server.on("/get/w14", [](AsyncWebServerRequest *r){ PropValve1Fault = !PropValve1Fault; getBool(r, PropValve1Fault); });
  Server.on("/get/w15", [](AsyncWebServerRequest *r){ PropValve2Fault = !PropValve2Fault; getBool(r, PropValve2Fault); });
  Server.on("/get/w3",  [](AsyncWebServerRequest *r){ getFloat(r, TargetPressure); });
  Server.on("/set/w3",  [](AsyncWebServerRequest *r){ setTargetPressure(r); }); 
  Server.onNotFound(URI_NotFound);
  //Server.on("/inline", []() { Server.send(200, "text/plain", "this works as well"); });

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); // THIS LINE IS NEEDED TO PREVENT BROWSER SECURITY 'FEATURES' FROM BLOCKING ALL THE AJAX CALLS WHILE DEBUGING ON LOCALHOST
  Server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  //Server.handleClient();
}

char* RandomDataString(int numPoints){
  strcpy(JsonDataString, "var dataSet = [");

  int y = rand() % 130;
  for (int x = 0; x < numPoints; x++) {
    int y2 = rand() % 130;
    sprintf(JsonDataString, "%s[%d, %d],", JsonDataString, x, y+y2);
  }

  strcat(JsonDataString, "];");

  return JsonDataString;
}

#include "th_WebServer.h"

#include <OSBos.h>      // The custom library that acts as our simple kernel https://github.com/actuvon/OSBos
#include "lib_Eth.h"            // Utilities for the ethernet server
#include "lib_Utils.h"          // General utilities for converting string to float and whatnot
#include "th_test.h"		    // Used for the example button widget
#include <ArduinoJson.h>		// Downloaded and installed through the Arduino IDE. Author: Benoit Blanchon. Version 7.4.2 installed.
#include "MechanicalSystem.h"   // Our business logic for controlling the mechanical system
#include "HAL.h" 		        // Our hardware abstraction layer
#include <functional>           // standard c++ library
#include "th_DataLogger.h"

extern OSBos kernel;
extern Thread th_test::thread;

namespace MechanicalSystem { namespace tk_StopAll { extern Thread Task; } }

// Private members
namespace {
	uint8_t MacAddress[6] = {0xDE, 0xAD, 0xEF, 0xFE, 0xED};
	IPAddress IP(192,168,1,177);

	const uint8_t LIVE_DATA_MAX_NUM_FIELDS = 15;
	const uint8_t LIVE_DATA_JSON_KEY_BUFFER_SIZE = 250;
	const uint8_t LIVE_DATA_JSON_RESP_BUFFER_SIZE = 500;
	
	EthernetServer EthSvr(80);
	lib_Eth::EthernetButler Jarvis(EthSvr);

	bool ExampleAlarmToggleThingy = false;

	float _exampleValueSenderValue = 0;

	// Widgets come here to ask for values from the program
	String GetWidgetStrVal(const char* key, bool& success){
		success = false;

		if(strcmp(key, "millis") == 0){
			success = true;
			return String(millis());
		}
		else if (strcmp(key, "wExample_liveShortValue") == 0){
			success = true;
			return (String("LSV: ") + String(millis()/100%100));
		}
		else if (strcmp(key, "wLowPressure") == 0){ 
			success = true;
			return String(HAL::getAnalogInputFloat(HAL::AnalogInput::PRESSURE_WINDOW_LOW)); 
		}
		else if (strcmp(key, "wMedPressure") == 0){
			success = true;
			return String(HAL::getAnalogInputFloat(HAL::AnalogInput::PRESSURE_WINDOW_MED));
		}
		else if (strcmp(key, "wHighPressure") == 0){
			success = true;
			return String(HAL::getAnalogInputFloat(HAL::AnalogInput::PRESSURE_WINDOW_HIGH));
		}
		else if (strcmp(key, "wDisplacement1") == 0){
			success = true;
			return String(HAL::getAnalogInputFloat(HAL::AnalogInput::DISPLACEMENT_1));
		}
		else if (strcmp(key, "wDisplacement2") == 0){
			success = true;
			return String(HAL::getAnalogInputFloat(HAL::AnalogInput::DISPLACEMENT_2));
		}
		else if(strcmp(key, "wLastLogEntry") == 0){
			success = true;
			return th_DataLogger::getLastLogLine();
		}
		else if (strcmp(key, "wRTC") == 0){
			success = true;
			return HAL::RTC_GetDateTime();
		}
		else if (strcmp(key, "wLPvalvesDirection") == 0){
			int8_t result = MechanicalSystem::GetLowPressureValveConfiguration();
			if(result == 0){
				success = true;
				return "0";
			}
			else if (result == 1){
				success = true;
				return "1";
			}
			else return "ERROR";
		}
		else if (strcmp(key, "wHPvalvesDirection") == 0){
			int8_t result = MechanicalSystem::GetHighPressureValveConfiguration();
			if(result == 0){
				success = true;
				return "0";
			}
			else if (result == 1){
				success = true;
				return "1";
			}
			else return "ERROR";
		}
		else if(strcmp(key, "wTargetPressure") == 0){
			success = true;
			return String(MechanicalSystem::GetTargetPressure());
		}
		else if(strcmp(key, "wWaterPump") == 0){
			success = true;
			return String(HAL::getDigitalOutputState(HAL::DigitalOutput::WATER_PUMP_POWER));
		}
		else if(strcmp(key, "wExample_smartShortValue") == 0){
			success = true;
			return String(millis());
		}
		else if(strcmp(key, "wExample_smartLabelValue") == 0){
			success = true;
			return ("Example label value: " + String(millis()));
		}
		else if(strcmp(key, "wExample_valueSender") == 0){
			success = true;
			return String(_exampleValueSenderValue);
		}
		else if(strcmp(key, "wExample_alarm") == 0){
			success = true;
			return String((int)ExampleAlarmToggleThingy);
		}
		else if(strcmp(key, "wExample_toggle") == 0){
			success = true;
			return String((int)ExampleAlarmToggleThingy);
		}
		else if(strcmp(key, "wExample_longLiveValue") == 0){
			success = true;
			return th_DataLogger::getLastLogLine();
		}
		else {
			success = false;
			return String("NOT FOUND");
		}
	}
	String GetWidgetStrVal(const char* key){ // Allow the user to not check for success
		bool sadBool;
		return GetWidgetStrVal(key, sadBool);
	}

	// Widgets come here to set program values
	bool SetWidgetVal(const char* key, const char* val){
		bool fltConvertWorked = false;
		bool success = false;

		if(strcmp(key, "wTargetPressure") == 0){
			float setTo = lib_Util::StringToFloat(val, fltConvertWorked);

			if(fltConvertWorked) success = MechanicalSystem::SetTargetPressure(setTo);
		}
		else if(strcmp(key, "wWaterPump") == 0) {
			if(val[0] == '1') {
				success = true;
				HAL::setDigitalOutput(HAL::DigitalOutput::WATER_PUMP_POWER, true);
			}
			else if(val[0] == '0'){
				success = true;
				HAL::setDigitalOutput(HAL::DigitalOutput::WATER_PUMP_POWER, false);
			}
		}
		else if(strcmp(key, "wExample_valueSender") == 0) {
			float setTo = lib_Util::StringToFloat(val, fltConvertWorked);

			if(fltConvertWorked){
				_exampleValueSenderValue = setTo;
				success = true;
			}
		}
		else if(strcmp(key, "wExample_toggle") == 0) {
			if(val[0] == '1') {
				success = true;
				ExampleAlarmToggleThingy = true;
			}
			else if(val[0] == '0'){
				success = true;
				ExampleAlarmToggleThingy = false;
			}
		}
		else return false;

		return success;
	}
	bool SetWidgetVal(const char* key, String val) { return SetWidgetVal(key, val.c_str()); }

	namespace routes {

		void index(EthernetClient& client, lib_Eth::HttpMsg& message){
			lib_Eth::respond_text(client, F("This server cannot currently serve up its own web app. The web app must be downloaded from https://github.com/KadenBurgart-LC/TorontoFenestrationController/tree/main/UI_Webpage"));
		}

		// The client UI requests live data in a packet each second. It asks us for certain values, and we give it those specific values.
		void P_liveDataPacketRequest(EthernetClient& client, lib_Eth::HttpMsg& message){
			// This function expects a stringified JSON array of keys that looks like...
			// ["key1","key2","key3",...]
			// The JSON array is parsed using the ArduinoJson library.

			if(message.Method == lib_Eth::REQ_TYPE::POST){
				const size_t JsonInputDocCapacity = JSON_ARRAY_SIZE(LIVE_DATA_MAX_NUM_FIELDS) + LIVE_DATA_JSON_KEY_BUFFER_SIZE;
				StaticJsonDocument<JsonInputDocCapacity> jsonDoc;

				DeserializationError err = deserializeJson(jsonDoc, message.Body);

				if(err){
					Serial.print(F("ERROR: th_WebServer: Live data packet request: Failed to deserialize JSON array: "));
					Serial.println(err.f_str());
					lib_Eth::respond_400(client, F("Invalid JSON formatting"));
					return;
				}

				JsonArray requestedKeys = jsonDoc.as<JsonArray>();

				const size_t JsonOutputDocCapacity = JSON_ARRAY_SIZE(LIVE_DATA_MAX_NUM_FIELDS) + LIVE_DATA_JSON_RESP_BUFFER_SIZE;
				StaticJsonDocument<JsonOutputDocCapacity> jsonResponse;

				for(const char* key : requestedKeys){
					// Start fetching data and building the response object
					jsonResponse[key] = GetWidgetStrVal(key);
				}

				lib_Eth::respond_json(client, "", 200, "OK", false); // send JSON headers without terminating the connection
				serializeJson(jsonResponse, client);
				lib_Eth::closeConnection(client); // now we close the connection manually.
			}
			else lib_Eth::respond_405(client, F("The live data packet request endpoint only accepts POST requests."));
		}

		
		// Request handler builders for widgets
		lib_Eth::RequestHandler WidgetHandler_Toggle_Delayed(const char* widgetId, Thread& onTask, Thread& offTask){
			return [=, &onTask, &offTask](EthernetClient& client, lib_Eth::HttpMsg& message) mutable {
				bool success = false;

				if(message.Method == lib_Eth::REQ_TYPE::POST){
					const char* setVal = message.Body.c_str() + 6; // setTo= is 6 characters

					if(setVal[0] == '1') kernel.StartTerminalAsyncTask(onTask, [=](int8_t result) mutable {
						if(result==1) lib_Eth::respond_text(client, "1");
						else lib_Eth::respond_500(client, "ERROR: The system could not complete an action to turn on a toggle widget.");
					});
					else if (setVal[0] == '0') kernel.StartTerminalAsyncTask(offTask, [=](int8_t result) mutable {
						if(result==1) lib_Eth::respond_text(client, "0");
						else lib_Eth::respond_500(client, "ERROR: The system could not complete an action to turn off a toggle widget.");
					});
					else lib_Eth::respond_400(client, "ERROR: Toggle widgets only accept values of 0 or 1.");
				}
				else if(message.Method == lib_Eth::REQ_TYPE::GET){
					String result = GetWidgetStrVal(widgetId, success);

					if(success) lib_Eth::respond_text(client, result);
					else lib_Eth::respond_500(client, "ERROR: There was an issue getting the value connected to this toggle widget.");
				}
				else lib_Eth::respond_405(client, F("ERROR: Toggle widgets only accept GET or POST requests."));
			};
		}
		lib_Eth::RequestHandler WidgetHandler_Button_Delayed(Thread& task, const char* successMsg = "Button widget action success.", const char* failMsg = "ERROR: Button widget action failed."){
			return [=, &task](EthernetClient& client, lib_Eth::HttpMsg& message) mutable {
				if(message.Method == lib_Eth::REQ_TYPE::GET){
					kernel.StartTerminalAsyncTask(task, [=](int8_t result) mutable {
						if(result == 1) lib_Eth::respond_text(client, successMsg);
						else lib_Eth::respond_text(client, failMsg);
					});
				}
				else lib_Eth::respond_405(client, "ERROR: Button widgets only accept GET requests.");
			};
		}
		lib_Eth::RequestHandler WidgetHandler_SmartLabelValue(const char* widgetId){
			return [widgetId](EthernetClient& client, lib_Eth::HttpMsg& message) -> void {
				if(message.Method == lib_Eth::REQ_TYPE::GET) lib_Eth::respond_text(client, GetWidgetStrVal(widgetId));
				else lib_Eth::respond_405(client, "ERROR: smart-label-value fields only accept GET requests.");
			};
		}
		lib_Eth::RequestHandler WidgetHandler_ValueSender(const char* widgetId){
			return [widgetId](EthernetClient& client, lib_Eth::HttpMsg& message) {
				if(message.Method == lib_Eth::REQ_TYPE::POST){
					bool success = false;
					const char* dataString = message.Body.c_str() + 6; // value=###   <-- the value= part is 6 chars long

					success = SetWidgetVal(widgetId, dataString);

					if(success) lib_Eth::respond_text(client, dataString);
					else lib_Eth::respond_400(client, "ERROR: Failed to set the value for a value-sender widget.");
				}
				else if(message.Method == lib_Eth::REQ_TYPE::GET){
					lib_Eth::respond_text(client, GetWidgetStrVal(widgetId));
				}
				else lib_Eth::respond_405(client, F("ERROR: value-sender widgets only accept GET or POST requests"));
			};
		}
		lib_Eth::RequestHandler WidgetHandler_LiveShortValue(const char* widgetId) { return WidgetHandler_SmartLabelValue(widgetId); }
		lib_Eth::RequestHandler WidgetHandler_SmartShortValue(const char* widgetId) { return WidgetHandler_SmartLabelValue(widgetId); }
		lib_Eth::RequestHandler WidgetHandler_Alarm(const char* widgetId) { return WidgetHandler_SmartLabelValue(widgetId); }
		lib_Eth::RequestHandler WidgetHandler_Toggle(const char* widgetId) {
			return [widgetId](EthernetClient& client, lib_Eth::HttpMsg& message) {
				bool success = false;

				if(message.Method == lib_Eth::REQ_TYPE::POST){
					const char* setVal = message.Body.c_str() + 6; // setTo= is 6 characters

					success = SetWidgetVal(widgetId, setVal);

					if(success) lib_Eth::respond_text(client, setVal);
					else lib_Eth::respond_500(client, "ERROR: Failed to set the value for a toggle widget.");
				}
				else if(message.Method == lib_Eth::REQ_TYPE::GET){
					String result = GetWidgetStrVal(widgetId, success);

					if(success) lib_Eth::respond_text(client, result);
					else lib_Eth::respond_500(client, "ERROR: There was an issue getting the value connected to this toggle widget.");
				}
				else lib_Eth::respond_405(client, F("ERROR: Toggle widgets only accept GET or POST requests."));
			};
		}
	}
}

namespace th_WebServer{
	void initialize(){
		Ethernet.begin(MacAddress, IP);

		// Check for Ethernet hardware present
		if (Ethernet.hardwareStatus() == EthernetNoHardware) {
			Serial.println(F("Ethernet shield was not found. Can not initialize webserver."));
		}
		else if (Ethernet.linkStatus() == LinkOFF) {
			Serial.println(F("Ethernet cable is not connected. Can not initialize webserver."));
		}
		else{
			EthSvr.begin();

			Serial.print(F("Starting local server at "));
			Serial.println(Ethernet.localIP());
		}

		Jarvis.On("/", routes::index);
		Jarvis.On("/wSTOP_ALL", routes::WidgetHandler_Button_Delayed(MechanicalSystem::tk_StopAll::Task, "All functions halted.", "Stop action failed."));

		Jarvis.On("/liveDataPacketRequest", routes::P_liveDataPacketRequest);

		Jarvis.On("/wRTC", routes::WidgetHandler_SmartLabelValue("wRTC"));
		
		Jarvis.On("/wTargetPressure", routes::WidgetHandler_ValueSender("wTargetPressure"));
		Jarvis.On("/wLowPressure", routes::WidgetHandler_LiveShortValue("wLowPressure"));
		Jarvis.On("/wMedPressure", routes::WidgetHandler_LiveShortValue("wMedPressure"));
		Jarvis.On("/wHighPressure", routes::WidgetHandler_LiveShortValue("wHighPressure"));
		Jarvis.On("/wDisplacement1", routes::WidgetHandler_LiveShortValue("wDisplacement1"));
		Jarvis.On("/wDisplacement2", routes::WidgetHandler_LiveShortValue("wDisplacement2"));

		Jarvis.On("/wWaterPump", routes::WidgetHandler_Toggle("wWaterPump"));
		Jarvis.On("/wHPvalvesDirection", routes::WidgetHandler_Toggle_Delayed("wHPvalvesDirection", th_test::thread, th_test::thread));
		Jarvis.On("/wLPvalvesDirection", routes::WidgetHandler_Toggle_Delayed("wLPvalvesDirection", MechanicalSystem::tk_SetLowPressure_Positive::Task, MechanicalSystem::tk_SetLowPressure_Negative::Task));

		Jarvis.On("/wExample_smartShortValue", routes::WidgetHandler_SmartShortValue("wExample_smartShortValue"));
		Jarvis.On("/wExample_smartLabelValue", routes::WidgetHandler_SmartLabelValue("wExample_smartLabelValue"));
		Jarvis.On("/wExample_liveShortValue", routes::WidgetHandler_LiveShortValue("wExample_liveShortValue"));
		Jarvis.On("/wExample_valueSender", routes::WidgetHandler_ValueSender("wExample_valueSender"));
		Jarvis.On("/wExample_toggle", routes::WidgetHandler_Toggle("wExample_toggle"));
		Jarvis.On("/wExample_alarm", routes::WidgetHandler_Alarm("wExample_alarm"));
		Jarvis.On("/wExample_button", routes::WidgetHandler_Button_Delayed(th_test::thread));
	}

	/* Check and see if client requests have come in. Read the requests. Grab the requested path.
	 * Use the path to route to an appropriate action and respond.
	 */
	int8_t tick(){
		Jarvis.Serve();

		return 0; // Tell OSBos to keep running
	}
}
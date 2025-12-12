#include "th_WebServer.h"

#include <OSBos.h>      // The custom library that acts as our simple kernel https://github.com/actuvon/OSBos
#include "lib_Eth.h"
#include "lib_Utils.h"
#include "th_test.h"		    // Used for the example button widget
#include <ArduinoJson.h>		// Downloaded and installed through the Arduino IDE. Author: Benoit Blanchon. Version 7.4.2 installed.
#include "MechanicalSystem.h"
#include "HAL.h"

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

	// The live data packet requester outsources fetching key values to this helper function
	String liveDataKeyValueFetcher(const char* key){
		if(strcmp(key, "millis") == 0) return String(millis());
		else if (strcmp(key, "wExample_liveShortValue") == 0) return (String("LSV: ") + String(millis()/100%100));
		else if (strcmp(key, "wLowPressure") == 0) return String(HAL::getAnalogInputFloat(HAL::AnalogInput::PRESSURE_WINDOW_LOW));
		else if (strcmp(key, "wMedPressure") == 0) return String(HAL::getAnalogInputFloat(HAL::AnalogInput::PRESSURE_WINDOW_MED));
		else if (strcmp(key, "wHighPressure") == 0) return String(HAL::getAnalogInputFloat(HAL::AnalogInput::PRESSURE_WINDOW_HIGH));
		else if (strcmp(key, "wDisplacement1") == 0) return "NO HAL";
		else if (strcmp(key, "wDisplacement2") == 0) return "NO HAL";
		else return String("NOT FOUND");
	}

	namespace routes {
		/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!     HTTP ROUTING HANDLERS     !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		 * 
		 * These methods are run when the client requests specific paths in their HTTP method.
		 * These methods are registered by Jarvis in th_WebServer::initialize(). Look at the Jarvis.On() hookups to see the paths
		 * for each method.
		 *
		 * Name the method starting with...
		 *   G_ for methods that accept GET requests (request data from the server).
		 *   P_ for methods that accept POST requests (send data to the server).
		 *   DO NOT USE PUT REQUESTS because programmers SUCK THEY'RE FUCKING AWFUL
		 * 
		 *   RULE: If a path accepts multiple types of requests, you can combine those prefixes in the order G, U, O.
		 *   RULE: If the request is coming from a UI widget, the handler should be named after the ID of the widget. 
		 *         Widget IDs start with a lowercase w.
		 *
		 *   Example: If we have a method for adding or retrieving log messages, where GET returns the last log message,
		 *            or POST adds a new log line, we might name the handler GP_wLogLine, if the AJAX requests 
		 *	          are coming from a widget with ID "wLogLine".
		 */

		void index(EthernetClient& client, lib_Eth::HttpMsg& message){
			lib_Eth::respond_text(client, F("This server cannot currently supply its own web app. The web app must be downloaded from https://github.com/KadenBurgart-LC/TorontoFenestrationController/tree/main/UI_Webpage"));
		}

		void G_wExample_smartShortValue(EthernetClient& client, lib_Eth::HttpMsg& message){
			if(message.Method == lib_Eth::REQ_TYPE::GET) lib_Eth::respond_text(client, String(millis()));
			else lib_Eth::respond_405(client, F("smart-short-value widgets only accept GET requests."));
		}

		void G_wExample_smartLabelValue(EthernetClient& client, lib_Eth::HttpMsg& message){
			if(message.Method == lib_Eth::REQ_TYPE::GET) lib_Eth::respond_text(client, "Example label value: " + String(millis()));
			else lib_Eth::respond_405(client, F("smart-label-value widgets should only accept GET requests."));
		}

		// Example toggle handler (turns on and off the example alarm)
		void GP_wExample_toggle(EthernetClient& client, lib_Eth::HttpMsg& message){
			if(message.Method == lib_Eth::REQ_TYPE::POST){
				if(message.Body == "setTo=1"){
					ExampleAlarmToggleThingy = true;
					lib_Eth::respond_text(client, String("1"));
				}
				else {
					ExampleAlarmToggleThingy = false;
					lib_Eth::respond_text(client, String("0"));
				}
			}
			else if(message.Method == lib_Eth::REQ_TYPE::GET){
				lib_Eth::respond_text(client, String((int)ExampleAlarmToggleThingy));
			}
			else lib_Eth::respond_405(client, F("toggle widgets only accept GET or PUT requests."));
		}

		// Example value-sender handler
		void GP_wExample_valueSender(EthernetClient& client, lib_Eth::HttpMsg& message){
			static float myVal = 0;

			if(message.Method == lib_Eth::REQ_TYPE::POST){
				myVal = atof(message.Body.c_str()+6); // value=###   <-- the value part is 6 chars long
				lib_Eth::respond_text(client, String(myVal));
			}
			else if(message.Method == lib_Eth::REQ_TYPE::GET) lib_Eth::respond_text(client, String(myVal));
			else lib_Eth::respond_405(client, F("value-sender widgets only accept GET or POST requests."));
		}

		// Example alarm handler (the example toggle turns the alarm on and off)
		void G_wExample_alarm(EthernetClient& client, lib_Eth::HttpMsg& message){
			if(message.Method == lib_Eth::REQ_TYPE::GET){
				lib_Eth::respond_text(client, String((int)ExampleAlarmToggleThingy));
			}
			else lib_Eth::respond_405(client, F("alarm widgets only accept GET requests."));
		}

		void G_wExample_liveShortValue(EthernetClient& client, lib_Eth::HttpMsg& message){
			if(message.Method == lib_Eth::REQ_TYPE::GET){
				lib_Eth::respond_text(client, (String("LSV:") + String(millis()/100%100)));
			} else lib_Eth::respond_405(client, F("This method only accepts GET requests."));
		}

		// Asynchronous terminal task with callback response to WebApp
		void G_wExample_button(EthernetClient& client, lib_Eth::HttpMsg& message){
			/* This shows a more advanced feature of this architecture by initiating a terminal async task that
			   turns the CPU LED blue for a few seconds, then turns it green again. The client doesn't get their
			   success message until the async task inidcates that it's done. */

			kernel.StartTerminalAsyncTask(th_test::thread, [client](int8_t result) mutable {
				if(result == 1) lib_Eth::respond_text(client, F("SUCCESS!"));
				else lib_Eth::respond_text(client, F("FAIL!"));
			});
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
					jsonResponse[key] = liveDataKeyValueFetcher(key);
				}

				lib_Eth::respond_json(client, "", 200, "OK", false); // send JSON headers without terminating the connection
				serializeJson(jsonResponse, client);
				lib_Eth::closeConnection(client); // now we close the connection manually.
			}
			else lib_Eth::respond_405(client, F("The live data packet request endpoint only accepts POST requests."));
		}

		// Negative pressure toggle widget
		void P_wNegativePressure(EthernetClient& client, lib_Eth::HttpMsg& message){
			if(message.Body == "setTo=1"){

			} else if(message.Body == "setTo = 0"){

			}
			else {

			}
		}

		void G_RTC(EthernetClient& client, lib_Eth::HttpMsg& message){
			if(message.Method == lib_Eth::REQ_TYPE::GET) lib_Eth::respond_text(client, HAL::RTC_GetDateTime());
			else lib_Eth::respond_405(client, "There is currently no mechanism for setting the RTC through the web server. Must use the SerialConsole.");
		}

		void G_wSTOP_ALL(EthernetClient& client, lib_Eth::HttpMsg& message){
			kernel.StartTerminalAsyncTask(MechanicalSystem::tk_StopAll::Task, [client](int8_t result) mutable {
				if(result == 1) lib_Eth::respond_text(client, F("All functions halted"));
				else lib_Eth::respond_text(client, F("ERROR"));
			});
		}

		void GP_wTargetPressure(EthernetClient& client, lib_Eth::HttpMsg& message){
			if(message.Method == lib_Eth::REQ_TYPE::POST){
				bool success = false;
				float setTo = 0;

				setTo = lib_Util::StringToFloat(message.Body.c_str() + 6, success);  // value=###   <-- the value part is 6 chars long

				if(success){
					success = MechanicalSystem::SetTargetPressure(setTo);
				}

				if(success){
					lib_Eth::respond_text(client, String(setTo));
				}
				else {
					lib_Eth::respond_400(client, F("Failed to set the target pressure."));
					//lib_Eth::respond_text(client, String(MechanicalSystem::GetTargetPressure()));
				}
			}
			else if(message.Method == lib_Eth::REQ_TYPE::GET) lib_Eth::respond_text(client, String(MechanicalSystem::GetTargetPressure()));
			else lib_Eth::respond_405(client, F("The TargetPressure widget only accepts GET or POST requests."));
		}

		void G_wLowPressure(EthernetClient& client, lib_Eth::HttpMsg& message)  { lib_Eth::respond_text(client, String(HAL::getAnalogInputFloat(HAL::AnalogInput::PRESSURE_WINDOW_LOW))); }
		void G_wMedPressure(EthernetClient& client, lib_Eth::HttpMsg& message)  { lib_Eth::respond_text(client, String(HAL::getAnalogInputFloat(HAL::AnalogInput::PRESSURE_WINDOW_MED))); }
		void G_wHighPressure(EthernetClient& client, lib_Eth::HttpMsg& message) { lib_Eth::respond_text(client, String(HAL::getAnalogInputFloat(HAL::AnalogInput::PRESSURE_WINDOW_HIGH))); }

		void GP_wWaterPump(EthernetClient& client, lib_Eth::HttpMsg& message){
			if(message.Method == lib_Eth::REQ_TYPE::POST){
				if(message.Body == "setTo=1"){
					HAL::setDigitalOutput(HAL::DigitalOutput::WATER_PUMP_POWER, true);
					lib_Eth::respond_text(client, String("1"));
				}
				else {
					HAL::setDigitalOutput(HAL::DigitalOutput::WATER_PUMP_POWER, false);
					lib_Eth::respond_text(client, String("0"));
				}
			}
			else if(message.Method == lib_Eth::REQ_TYPE::GET){
				lib_Eth::respond_text(client, String(HAL::getDigitalOutputState(HAL::DigitalOutput::WATER_PUMP_POWER)));
			}
			else lib_Eth::respond_405(client, F("toggle widgets only accept GET or POST requests."));
		}

		/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   / HTTP ROUTING HANDLERS     !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
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
		Jarvis.On("/wExample_smartShortValue", routes::G_wExample_smartShortValue);
		Jarvis.On("/wExample_valueSender", routes::GP_wExample_valueSender);
		Jarvis.On("/wExample_toggle", routes::GP_wExample_toggle);
		Jarvis.On("/wExample_alarm", routes::G_wExample_alarm);
		Jarvis.On("/wExample_liveShortValue", routes::G_wExample_liveShortValue);
		Jarvis.On("/wExample_button", routes::G_wExample_button);
		Jarvis.On("/wExample_smartLabelValue", routes::G_wExample_smartLabelValue);

		Jarvis.On("/wSTOP_ALL", routes::G_wSTOP_ALL);
		Jarvis.On("/wRTC", routes::G_RTC);

		Jarvis.On("/wTargetPressure", routes::GP_wTargetPressure);
		Jarvis.On("/wLowPressure", routes::G_wLowPressure);
		Jarvis.On("/wMedPressure", routes::G_wMedPressure);
		Jarvis.On("/wHighPressure", routes::G_wHighPressure);

		Jarvis.On("/wWaterPump", routes::GP_wWaterPump);

		Jarvis.On("/liveDataPacketRequest", routes::P_liveDataPacketRequest);
	}

	/* Check and see if client requests have come in. Read the requests. Grab the requested path.
	 * Use the path to route to an appropriate action and respond.
	 */
	int8_t tick(){
		Jarvis.Serve();

		return 0; // Tell OSBos to keep running
	}
}
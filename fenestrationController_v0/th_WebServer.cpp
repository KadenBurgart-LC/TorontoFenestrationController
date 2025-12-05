#include "th_WebServer.h"

#include "lib_Eth.h"
#include <ArduinoJson.h>		// Downloaded and installed through the Arduino IDE. Author: Benoit Blanchon. Version 7.4.2 installed.
#include "th_test.h"
#include "lib_OSBos.h"

extern OSBos kernel;
extern Thread th_test::thread;

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

		// Example smart-short-value handler
		void G_wExample_smartShortValue(EthernetClient& client, lib_Eth::HttpMsg& message){
			if(message.Method == lib_Eth::REQ_TYPE::GET) lib_Eth::respond_text(client, String(millis()));
			else lib_Eth::respond_405(client, F("smart-short-value widgets only accept GET requests."));
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
			else lib_Eth::respond_405(client, F("value-sender widgets only accept GET or PUT requests."));
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

		void G_wExample_button(EthernetClient& client, lib_Eth::HttpMsg& message){
			kernel.StartTerminalAsyncTask(th_test::thread, [client](int8_t result) mutable {
				Serial.println("\n\n\nDONE asdfadsafsdfasdfadsfasdfasdfasd\n\n\n");
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

				lib_Eth::respond_json(client, ""); // send JSON headers
				serializeJson(jsonResponse, client);
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
		Jarvis.On("/w/wExample_smartShortValue", routes::G_wExample_smartShortValue);
		Jarvis.On("/w/wExample_valueSender", routes::GP_wExample_valueSender);
		Jarvis.On("/w/wExample_toggle", routes::GP_wExample_toggle);
		Jarvis.On("/w/wExample_alarm", routes::G_wExample_alarm);
		Jarvis.On("/w/wExample_liveShortValue", routes::G_wExample_liveShortValue);
		Jarvis.On("/w/wExample_button", routes::G_wExample_button);
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
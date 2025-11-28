#include "th_WebServer.h"

#include <SPI.h>		// standard arduino library
#include <Ethernet.h>	// standard arduino library
//#include <EthernetWebServer.h> // ain't nobody got time for that     https://github.com/khoih-prog/EthernetWebServer

#define USE_WS100 false

// Private members
namespace {
	bool PRINT_INCOMMING_REQUESTS = false;
	bool PRINT_INCOMING_PATHS = true;

	byte MacAddress[6] = {0xDE, 0xAD, 0xEF, 0xFE, 0xED};
	IPAddress IP(192,168,1,177);
	IPAddress GATEWAY(192,168,1,1);
	IPAddress SUBNET(255,255,255,0);
	const uint8_t ETH_CS_PIN = 5;   			// From the P1AM-ETH documentation      https://facts-engineering.github.io/modules/P1AM-ETH/P1AM-ETH.html
	EthernetServer* EthSvr = nullptr;

	//EthernetWebServer Server(80);

	/* When we receive an HTTP request there is a bunch of information in there that we don't need, but we
	 * DO need the path. The path comes in on the first line between the first two spaces.
	 * This function grabs the text between the first two spaces and returns it.
	 */
	String getRequestPath(String request){
		String path;

		int space1 = request.indexOf(' ');
		int space2 = request.indexOf(' ', space1 + 1);

		path = request.substring(space1+1, space2);

		return path;
	}

	/* Respond to a client request with a text packet
	 */
	void respond_text(EthernetClient client, String text, int code = 200, String reason = "OK"){
		String response = R"===(
HTTP/1.1 {{{code}}} {{{reason}}}
Content-Type: text/html; charset=utf-8
Access-Control-Allow-Origin: *

{{{TEXT}}})===";

		response.replace("{{{code}}}", String(code));
		response.replace("{{{reason}}}", String(reason));
		response.replace("{{{TEXT}}}", String(text));

		client.print(response);
	}

	/* Respond to a client request with a JSON packet
	 */
	void respond_json(EthernetClient client, String json, int code = 200, String reason = "OK"){
		String response = R"===(
HTTP/1.1 {{{code}}} {{{reason}}}
Content-Type: application/json
Access-Control-Allow-Origin: *

{{{JSON}}})===";

		response.replace("{{{code}}}", String(code));
		response.replace("{{{reason}}}", String(reason));
		response.replace("{{{JSON}}}", String(json));

		client.print(response);
	}

	void respond_404(EthernetClient client){
		String response = R"===(
HTTP/1.1 404 Not Found
Content-Type: text/plain
Access-Control-Allow-Origin: *

404 Not Found: The requested resource does not exist.)===";

		client.print(response);
	}

	void pathHandler_get_pressure(EthernetClient client){
		String response = R"===({"millis":{{{millis}}},"pressure":{{{pressure}}}})===";

		response.replace("{{{millis}}}", String(millis()));
		response.replace("{{{pressure}}}", "100");

		respond_json(client, response);
	}

	void handleRoot(){
		Server.send(200, "text/plain", "Hello world");
	}
}

namespace th_WebServer{
	void initialize(){
		Ethernet.begin(MacAddress, IP);

		// Check for Ethernet hardware present
		if (Ethernet.hardwareStatus() == EthernetNoHardware) {
			Serial.println("Ethernet shield was not found. Can not initialize webserver.");
		}
		else if (Ethernet.linkStatus() == LinkOFF) {
			Serial.println("Ethernet cable is not connected. Can not initialize webserver.");
		}
		else{
			EthSvr = new EthernetServer(80);
			EthSvr->begin();

			Serial.print("Starting local server at ");
			Serial.println(Ethernet.localIP());
		}
	}

	/* Check and see if client requests have come in. Read the requests. Grab the requested path.
	 * Use the path to route to an appropriate action and respond.
	 */
	void tick(){
		Server.handleClient();

		EthernetClient client = EthSvr->available();

		if(client){
			// Read the incoming request
		    String request = "";
		    while (client.available()) {
		      request += (char)client.read();
		    }

		    String path = getRequestPath(request);

		    if(PRINT_INCOMMING_REQUESTS){
			    Serial.println("Web Server request received: ");
			    Serial.println(request);
		    }
		    else if (PRINT_INCOMING_PATHS){
		    	Serial.print("Web Server request received for path: ");
			    Serial.println(path);
		    }

		    // ROUTING GOES HERE
		    if(path == "/") respond_text(client, "Request received. Server is online.");
		    else if(path == "/get/pressure") pathHandler_get_pressure(client);
		    else if(path == "/get/w5") respond_404(client); // Structural pressure direction
		    else if(path == "/set/w1") respond_404(client); // Structural blower toggle
		    else {
		    	respond_404(client);
		    }

			delay(1);
			client.stop();
		}
	}
}


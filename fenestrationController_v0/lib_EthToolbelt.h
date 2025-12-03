#ifndef LIB_ETH_TOOLBELT_H
#define LIB_ETH_TOOLBELT_H

#include <Arduino.h>	// standard arduino library
#include <SPI.h>		// standard arduino library
#include <Ethernet.h>	// standard arduino library
#include <functional>	// standard c++ library
#include <map>			// standard c++ library

namespace {
	bool PRINT_INCOMMING_REQUESTS = true;
	bool PRINT_INCOMING_PATHS = false;
}

namespace lib_EthToolbelt {
	enum class REQ_TYPE {
		GET,		// GET requests data from the server
		POST,		// POST sends data to the server
		//PUT,		// DO NOT USE PUT!!! Those worthless security popmpusshitheadmotherfuckingdouchecunttaintfuckers FUCK! Cross-origin PUT requests are blocked by modern browsers, which send an OPTIONS request to the server first, asking for permission to send a cross-origin PUT since it's considered a nontrivial command, whereas POST is apparently fine FUCK THESE WORTHLESS FUCKING CUNTS I don't want to implement their security BULLSHIT on my tiny little embedded server those motherfuckers can choke on their own cocks and die.
		OTHER,
		ERROR
	};

	struct HttpMessage {
		REQ_TYPE 		Method;
		String			Path;
		String			Body;

		HttpMessage(const String& rawMessage) {
			int endOfFirstLine = rawMessage.indexOf('\n');
			if(endOfFirstLine == -1) return; // Basic error check

			String firstLine = rawMessage.substring(0, endOfFirstLine);

			int firstSpace = rawMessage.indexOf(' ');
			if(firstSpace != -1){
				String methodStr = firstLine.substring(0, firstSpace);

				if(methodStr == "GET") Method = REQ_TYPE::GET;
				//else if(methodStr == "PUT") Method = REQ_TYPE::PUT;
				else if(methodStr == "POST") Method = REQ_TYPE::POST;
				else Method = REQ_TYPE::OTHER;

				int secondSpace = firstLine.indexOf(' ', firstSpace+1);
				if(secondSpace != -1){
					this->Path = firstLine.substring(firstSpace+1, secondSpace);
				}
				else {
					Method = REQ_TYPE::ERROR;
					Path = "";
				}
			}
			else{
				Method = REQ_TYPE::ERROR;
				Path = "";
			}

			int bodyStart = rawMessage.indexOf("\r\n\r\n"); // The HTTP protocol designates this as THE THING between the headers and body
			if(bodyStart == -1) bodyStart = rawMessage.indexOf("\n\n"); // Give the request one last chance before giving up on it.
			
			if(bodyStart != -1){
				// Skip the blank lines
				bodyStart += (rawMessage.substring(bodyStart, bodyStart+4) == "\r\n\r\n") ? 4 : 2;
				this->Body = rawMessage.substring(bodyStart);
			}
			else {
				this->Body = "";
			}
		}
	};

	/* Respond to a client request with a text packet
	 */
	inline void respond_text(EthernetClient& client, const String& text, int code = 200, String reason = "OK"){
		client.print(F("HTTP/1.1 "));
		client.print(code);
		client.print(" ");
		client.println(reason);
		client.println(F("Content-Type: text/html; charset=utf-8"));
		client.println(F("Access-Control-Allow-Origin: *"));
		client.println();
		client.print(text);
	}

	inline void respond_json(EthernetClient& client, const String& json, int code = 200, String reason = "OK"){
		client.print(F("HTTP/1.1 "));
		client.print(code);
		client.print(" ");
		client.println(reason);
		client.println(F("Content-Type: application/json"));
		client.println(F("Access-Control-Allow-Origin: *"));
		client.println();
		client.print(json);
	}

	inline void respond_404(EthernetClient& client){
		client.println(F("HTTP/1.1 404 Not Found"));
		client.println(F("Content-Type: text/plain"));
		client.println(F("Access-Control-Allow-Origin: *"));
		client.println();
		client.println(F("404 Not Found: The requested resource does not exist."));
	}

	inline void respond_405(EthernetClient& client, const String& info = F("The requested resource does not exist.")){
		client.println(F("HTTP/1.1 404 Not Found"));
		client.println(F("Content-Type: text/plain"));
		client.println(F("Access-Control-Allow-Origin: *"));
		client.println();
		client.print(F("404 Not Found: "));
		client.println(info);
	}

	inline void respond_400(EthernetClient& client, const String& info = F("The server could not understand the request.")){
		client.println(F("HTTP/1.1 400 Bad Request"));
		client.println(F("Content-Type: text/plain"));
		client.println(F("Access-Control-Allow-Origin: *"));
		client.println();
		client.print(F("400 Bad Request: "));
		client.println(info);
	}




	// The structure of request handler functions to go in the EthernetButler routing table
	using RequestHandler = std::function<void(EthernetClient& client, HttpMessage& message)>;

	class EthernetButler {
	private:
		EthernetServer& _ethSvr;

		// Routing table
		std::map<String, RequestHandler> _routes;

	public:
		inline EthernetButler(EthernetServer& ethSvr) : _ethSvr(ethSvr) { }

		// Hook up a request handler in the routing table
		inline void On(const String& path, RequestHandler handler){
			_routes[path] = handler;
		}

		// Serve an HTTP client
		inline void Serve(){
			EthernetClient client = _ethSvr.available();

			if(client){
				// Read the incoming HTTP message
			    String messageStr = "";
			    while (client.available()) {
			      messageStr += (char)client.read();
			    }

			    if(PRINT_INCOMMING_REQUESTS){
			    	Serial.println("\nHTTP Message incoming...");
			    	Serial.println(messageStr);
			    }

			    HttpMessage message(messageStr);

			    if(PRINT_INCOMING_PATHS){
			    	Serial.print("HTTP Message incoming for path: ");
			    	Serial.println(message.Path);
			    }

				if(_routes.count(message.Path)){
					_routes[message.Path](client, message);
				}
				else respond_404(client);

				delay(1);
				client.stop();
			}
		}
	};


}

#endif
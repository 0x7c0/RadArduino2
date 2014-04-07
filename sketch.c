/*
Controlling two servo like a radar
see https://github.com/0x7c0/RadArduino2 for more info
modified on 7 Apr 2014

0x7c0 <0x7c0@teboss.tk>
http://www.gnu.org/licenses/gpl.html GPL v3 License
*/


// Include
#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>
#include <Servo.h>
#include <math.h>
// Define
#define			HTTP_BUFF 20
#define			HTTP_FILE 7
#define			MAX_X 180
#define			MAX_Y 90
// Pin
const int		xPin = 7; //pwm
const int		yPin = 8; //pwm
const int		resetPin = 40; //digital
const int		sdcPin = 4; //digital
const int		ethPin = 10; //digital
const int		greLed = 52; //digital
const int		redLed = 53; //digital
const int		photoRes = A10; //analog
const int		thermRes = A8; //analog
// HTTP Capture
char			getRequest[HTTP_BUFF] = "";
unsigned int	getIndi = 0;
unsigned int	X = 1000;
unsigned int	Y = 1000;
// Servo
Servo			servoX;
Servo			servoY;
const int		LIMIT = 10;
// File
File			webFile;
File			dataFile;
// Ethernet
byte mac[]		= {0xFE,0xAD,0xBE,0xEF,0xFE,0xED};
byte ip[]		= {192,168,2,110}; // CHANGE THIS!
byte gateway[]	= {192,168,2,1}; // CHANGE THIS!
byte subnet[]	= {255,255,255,0};
//byte dns[]	= {8,8,8,8};
EthernetServer server(80);
// Web Output
char* GET[HTTP_FILE]= {
	"index.htm",
	"style_.gz",
	"script.gz",
	"angle_.gz",
	"menu_.png",
	"home_.png",
	"favic.ico"
	};
char* TYP[3][HTTP_FILE]= {
	{
		"/index.html",
		"/style.css",
		"/script.js",
		"/angle.png",
		"/menu.png",
		"/home.png",
		"/favicon.ico",
	},
	{
		"Content-Type: text/html",
		"Content-Type: text/css",
		"Content-Type: application/javascript",
		"Content-Type: image/png",
		"Content-Type: image/png",
		"Content-Type: image/png",
		"Content-Type: image/x-icon",
	},
	{
		"0",
		"1",
		"1",
		"1",
		"0",
		"0",
		"0",
	}
};
char* LOG = "log.txt";
// End

void setup() {
	//debug
	Serial.begin(9600);
	//pin
	pinMode(resetPin, OUTPUT);
	pinMode(sdcPin, OUTPUT);
	//pinMode(ethPin, OUTPUT);
	pinMode(greLed, OUTPUT);
	pinMode(redLed, OUTPUT);

	// initialize SD card
	//digitalWrite(ethPin, LOW);
	//digitalWrite(sdcPin, HIGH);
	Serial.println("Initializing SD card...");
	if (!SD.begin(sdcPin)) {
		Serial.println("ERROR - SD card initialization failed!");
		return;
	}
	// check for file
	for (int i=0; i<HTTP_FILE; i++) {
		if (!SD.exists(GET[i])) {
			Serial.print("ERROR - Can't find ");
			Serial.println(GET[i]);
			return;  // can't find index file
		}
	}
	if (!SD.exists(LOG)) {
		Serial.print("ERROR - Can't find ");
		Serial.println(LOG);
		return;  // can't find index file
	}
	else {
		dataFile = SD.open(LOG, FILE_READ);
		if(dataFile) {
			char temp[3],c,d;
			while(dataFile.available()) {		// X=123-Y=45-
				d = (char) dataFile.read();		// X or Y
				dataFile.read();				// =
				c = (char) dataFile.read();
				for(int i=0;c!='-';i++) {
					temp[i] = c;
					c = (char) dataFile.read();
				}
				String Coord(temp);
				if(d=='X') {
					Update('X',Coord.toInt());
				}
				else if(d=='Y') {
					Update('Y',Coord.toInt());
				}
				memset(temp, 0, sizeof temp);
			}
			dataFile.close();
		}
	}
	Serial.println("SUCCESS - SD card initialized.");
	
	// initialize Ethernet device
	//digitalWrite(sdcPin, LOW);
	//digitalWrite(ethPin, HIGH);
	Ethernet.begin(mac, ip, gateway, subnet);
	server.begin();
	Serial.println("SUCCESS - Ethernet initialized.");
	// servo Pin
	servoX.attach(xPin);
	servoY.attach(yPin);
	if(servoX.attached() && servoY.attached()) {
		Serial.println("SUCCESS - Servo initialized.");
	}
	else {
		Serial.println("ERROR - Servo initialization failed!");
		return;
	}
	digitalWrite(redLed,LOW);
	digitalWrite(greLed,LOW);
	Serial.println("START");
}
void loop() {
	// try to get client
	EthernetClient client = server.available();
	String getStr;
	
	if (client) {
		digitalWrite(greLed,HIGH);
		boolean currentLineIsBlank = true;
		boolean indice = false;
		while (client.connected()) {
			if (client.available()) {	// client data available to read
				char c = client.read();	// read 1 byte (character) from client
				// filter GET request
				// GET /index.html HTTP/1.1
				if (c == 'G') {
					c = client.read();
					if (c == 'E') {
						c = client.read();
						if (c == 'T') {
							Serial.print("GET=");
							c = client.read(); // space
							while (true) {
								c = client.read();
								if (c == ' ') {
									break;
								}
								//Serial.println(c);
								getRequest[getIndi] = c;
								getIndi++;
							}
							Serial.print(getRequest);
							Serial.println("@");
						}
					}
				}
				if (c == '\n' && currentLineIsBlank) {
					// convert to string and clear char array
					String getStr(getRequest);
					Clear();

					// if check
					// first page
					if (getStr == "/" || getStr == "/index.html") {
						indice = true;
					}

					// ajax GET info
					else if (getStr.startsWith("/Set?")) {
						indice = true;
						int getRequestStart = getStr.indexOf('?' );
						int getRequestFirst = getStr.indexOf('=' );
						int getRequestFinal = getStr.indexOf('/',2);
						int getRequestDuall = getStr.indexOf('&' );	// 2 GET
						if (getRequestDuall>0) {	// /index.html?X=20&Y=30
							int getRequestSecon = getStr.indexOf('=', getRequestFirst+1);
							String inputFirst_1 = getStr.substring(getRequestStart+1, getRequestFirst);	// X
							String valueFirst_1 = getStr.substring(getRequestFirst+1, getRequestDuall);	// 20
							String inputFirst_2 = getStr.substring(getRequestDuall+1, getRequestSecon);	// Y
							String valueFirst_2 = getStr.substring(getRequestSecon+1, getRequestFinal);	// 30
							if (inputFirst_1 == "X") {
								Update('X',valueFirst_1.toInt());
								Update('Y',valueFirst_2.toInt());
							}
							// reverse
							else if (inputFirst_1 == "Y") {
								Update('Y',valueFirst_1.toInt());
								Update('X',valueFirst_2.toInt());
							}
						}
						else {						// /index.html?Y=50 or X=50
							String inputFirst = getStr.substring(getRequestStart+1, getRequestFirst);	// Y
							String valueFirst = getStr.substring(getRequestFirst+1, getRequestFinal);	// 50
							if (inputFirst == "X") {
								Update('X',valueFirst.toInt());
							}
							else if (inputFirst == "Y") {
								Update('Y',valueFirst.toInt());
							}
						}
					}

					//ajax SAVE info
					else if (getStr.startsWith("/Save/")) {
						digitalWrite(redLed,HIGH);
						client.println("HTTP/1.1 200 OK");
						client.println("Connection: close");
						Serial.println("SAVE");
						Serial.println("");
						SD.remove(LOG);
						dataFile = SD.open(LOG, FILE_WRITE);
						if(dataFile) {				// X=123-Y=45-
							dataFile.print("X=");
							dataFile.print(X);
							dataFile.print("-Y=");
							dataFile.print(Y);
							dataFile.print("-");
							dataFile.close();
						}
						digitalWrite(redLed,LOW);
					}

					//ajax RESET info
					else if (getStr.startsWith("/Reset/")) {
						digitalWrite(redLed,HIGH);
						client.println("HTTP/1.1 200 OK");
						client.println("Connection: close");
						Serial.println("RESET");
						Serial.println("");
						delay(1);
						servoX.detach();
						servoY.detach();
						digitalWrite(redLed,LOW);
						digitalWrite(resetPin, LOW);
					}
					// ajax SET info
					else if (getStr.startsWith("/Coordinate/")) {
						digitalWrite(redLed,HIGH);
						GetValue();
						double temp = analogRead(thermRes);
						int phot = analogRead(photoRes);
						client.println("HTTP/1.1 200 OK");
						client.println("Connection: close");
						client.println();
						// JSON
						client.print("{\"coordinate\":{\"X\":");
						client.print(X);
						client.print(",\"Y\":");
						client.print(Y);
						client.print("},\"temp\":");
						client.print(GetTemp(temp),1);
						client.print(",\"light\":");
						client.print(phot);
						client.print(",\"network\":\"");
						client.print(Ethernet.localIP());
						client.print("\",\"file\":[");
						for (int i=0; i<HTTP_FILE; i++) {
							File dFile = SD.open(GET[i]);
							if (dFile) {
								if(i>0) {
									client.print(",");
								}
								client.print("{");
								client.print("\"name\":\"");
								client.print(GET[i]);
								client.print("\",\"size\":");
								client.print(dFile.size());
								client.print("}");
							}
						}
						client.println("]}");
						digitalWrite(redLed,LOW);
					}

					// print other file
					else {
						for(int i=1; i<HTTP_FILE; i++) {
							if (getStr == TYP[0][i]) {
								webFile = SD.open(GET[i]);
								if (webFile) {
									client.println("HTTP/1.1 200 OK");
									client.println(TYP[1][i]);
									if(TYP[2][i] == "1") {
										client.println("Content-Encoding: gzip");
									}
									client.print("Content-Length: ");
									client.println(webFile.size());
									client.println("Cache-Control: max-age=302400, public");
									client.println("Connection: close");
									client.println();
								}
								break;
							}
						}
					}
					// endif check

					// print index.html
					if (indice) {
						webFile = SD.open(GET[0]);
						if (webFile) {
							client.println("HTTP/1.1 200 OK");
							client.println(TYP[1][0]);
							client.print("Content-Length: ");
							client.println(webFile.size());
							client.println("Connection: close");
							client.println();
						}
					}
					// read file and write into web client
					if (webFile) {
						while(webFile.available()) {
							client.write(webFile.read());
						}
						webFile.close();
					}
					
					break;
				}

				if (c == '\n') {
					// last character on line of received text. Starting new line with next character read
					currentLineIsBlank = true;
				} 
				else if (c != '\r') {
					// you've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		//delay(1);		// give the web browser time to receive the data
		client.stop();	// close the connection
		digitalWrite(greLed,LOW);
	}
}

//update value in memory
void Update(const char T,const int N) {
	if (T == 'X') {
		if (N>=0 && N<=MAX_X && N!=X) {
			Serial.print("Update  X=");
			Serial.print(X);
			Serial.print(" To=");
			Serial.print(N);
			Serial.print(" and ServoX=");
			X = N;
			if(N<=0+LIMIT) {
				servoX.write(0+LIMIT);
			}
			else if (N>=MAX_X-LIMIT) {
				servoX.write(MAX_X-LIMIT);
			}
			else {
				servoX.write(N);
			}
			Serial.println(servoX.read());
			return;
		}
	}
	else if (T == 'Y') {
		if (N>=0 && N<=MAX_Y && N!=Y) {
			Serial.print("Update  Y=");
			Serial.print(Y);
			Serial.print(" To=");
			Serial.print(N);
			Serial.print(" and ServoY=");
			Y = N;
			if(N<=0+LIMIT) {
				servoY.write(0+LIMIT);
			}
			else if (N>=MAX_Y-LIMIT) {
				servoY.write(MAX_Y-LIMIT);
			}
			else {
				servoY.write(N);
			}
			Serial.println(servoY.read());
			return;
		}
	}
}

//print value in memory (global)
void GetValue() {
	Serial.print("Now  X=");
	Serial.print(X);
	Serial.print(" and Y=");
	Serial.println(Y);
	return;
}

//clear http buffer
void Clear() {
	getIndi = 0;
	memset(getRequest, 0, sizeof getRequest);
	/*for (int i=0; i<HTTP_BUFF; i++) {
		getRequest[i] = ' ';
	}*/
	getRequest[0] = '\0';
	return;
}

//function to transform volt to temperature
double GetTemp(const double rd) {
	//value for 5k resistor
	const double A = 0.00076647;
	const double B = 0.00023051;
	const double C = 0.000000073815;
	//Resistance conversion used later
	const double R = ( 1023 / rd ) * 50000;
	//Temperature conversion equation
	const double T = A + B * (log(R)+ C * log(R) * log(R) * log(R));

	return ( 1 / T ) - 273.15;
}

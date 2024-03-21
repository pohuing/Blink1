#include<SoftwareSerial.h>

SoftwareSerial ldSerial = SoftwareSerial(6, 5);


enum DataType : byte {
	EngineeringMode = 0x01, TargetDataComposition = 0x02
};


enum TargetState : byte
{
	NoTarget = 0x00, SportsTarget = 0x01, StationaryTarget = 0x02, StationaryAndStationaryTargets = 0x03
};

static void prettyPrint(DataType t) {
	switch (t)
	{
	case EngineeringMode:
		Serial.print("EngineeringMode");
		break;
	case TargetDataComposition:
		Serial.print("TargetDataComposition");
		break;
	}
}


static void prettyPrint(TargetState s) {
	switch (s)
	{
	case NoTarget:
		Serial.print("NoTarget                      ");
		break;
	case SportsTarget:
		Serial.print("SportsTarget                  ");
		break;
	case StationaryTarget:
		Serial.print("StationaryTarget              ");
		break;
	case StationaryAndStationaryTargets:
		Serial.print("StationaryAndStationaryTargets");
		break;
	}
}
struct Datagramm
{
	byte frameHeader[4];
	byte intraDataLength[2];// 0c 00 
	byte commandWord[2]; // a0 01
	byte type[2];
	byte major[2];
	byte minor[4];
	byte __[4];
};

struct DetectionDatagramm {
	byte frameHeader[4]{ 0xF4, 0xF3, 0xF2, 0xF1 };

	short frameDataLength;
	DataType type;
	byte head{ 0xAA };
	TargetState targetState;
	short distance;
	byte exerciseTargetEnergyValue;
	short stationaryTargetDistance1;
	byte stationaryTargetEnergyValue;
	short detectionDistanceCm;
	byte tail{ 0x55 };
	byte check{ 0x00 };

	byte MFR[4]{0xF8,0xF7, 0xF6, 0xF5 };

	void prettyPrint() {
		//Serial.print("Frame header: ");
		//printBytes(frameHeader, 4);
		
		//Serial.print(" Frame data length: ");
		//Serial.print(frameDataLength);
		
		Serial.print(" Data Type: ");
		::prettyPrint(type);
		
		//Serial.print(" Head: ");
		//printBytes(&head, 1);
		
		Serial.print(" Target State: ");
		::prettyPrint(targetState);
		
		Serial.print(" Distance: ");
		Serial.print(distance);

		Serial.print(" Exercise Target Energy Value: ");
		Serial.print(exerciseTargetEnergyValue);

		Serial.print(" stationaryTargetDistance1: ");
		Serial.print(stationaryTargetDistance1);

		Serial.print(" stationaryTargetEnergyValue: ");
		Serial.print(stationaryTargetEnergyValue);

		Serial.print(" detectionDistanceCm: ");
		Serial.print(detectionDistanceCm);

		//Serial.print(" MFR: ");
		//printBytes(MFR, 4);
	}
};

void printBytes(uint8_t* data, size_t len) {
	for (size_t i = 0; i < len; i++)
	{
		if (data[i] < 16) {
			Serial.print(0);
		}
		Serial.print(data[i],HEX);
	}
}


void sendMessage(void* buf, unsigned short size) {
	ldSerial.write(0xFD);
	ldSerial.write(0xFC);
	ldSerial.write(0xFB);
	ldSerial.write(0xFA);

	ldSerial.write((char*) &size, 2);
	ldSerial.write((char*) buf, size);

	ldSerial.write(0x04);
	ldSerial.write(0x03);
	ldSerial.write(0x02);
	ldSerial.write(0x01);
	ldSerial.flush();
}
struct {
	byte payload[2]{ 0x0A, 0x00 };
} GET_FIRMWARE;

void setup() {
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Native USB only
	}

	pinMode(6, INPUT);
	pinMode(5, OUTPUT);

	Serial.write("initializing ldserial");

	ldSerial.begin(9600);
	Serial.write("Initialized ldserial");
	//sendMessage(&GET_FIRMWARE, 2);

}

long counter = 0;


// the loop function runs over and over again forever
void loop() {
	while (ldSerial.available() == 0);

	DetectionDatagramm dd;
	unsigned char buffer[sizeof(DetectionDatagramm)] {};
	byte c = ldSerial.readBytes((unsigned char*) & buffer, sizeof(buffer));
	//printBytes(buffer, sizeof(buffer));
	memcpy(&dd, &buffer, sizeof(buffer));
	dd.prettyPrint();



	//Datagramm d = Datagramm();

	//byte buffer[50] = {};

	//auto len = ldSerial.readBytes((uint8_t*)&d, sizeof(Datagramm));
	//auto len = ldSerial.readBytes((uint8_t*) &buffer, sizeof(buffer));

	//Serial.print("Read bytes: ");
	//Serial.println(len);
	//Serial.print("Overflow:");
	//Serial.println(ldSerial.overflow());

	//for (size_t i = 0; i < sizeof(buffer); i++)
	//{
	//	Serial.print(" ");
	//	if (((unsigned char*) &buffer)[i] < 16) {
	//		Serial.print(0);
	//	}
	//	Serial.print(((unsigned char*)&buffer)[i], HEX);
	//
	//}
	Serial.println();

}


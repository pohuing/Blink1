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

	static bool tryRead(const byte* buffer, size_t bufferSize, DetectionDatagramm* out) {
		size_t window = 0;
		while (window + sizeof(DetectionDatagramm) < bufferSize) {
			if (buffer[window] == 0xF4 && buffer[window +1 ] == 0xF3 && buffer[window +2] == 0xF2 && buffer[window+3] == 0xF1)
			{

				//Serial.println("Found frameHeader");
				//unsigned short length = buffer[window + offsetof(DetectionDatagramm, frameDataLength) +1 ] << 8 | buffer[window + offsetof(DetectionDatagramm, frameDataLength)];
				//Serial.print("Got Length: ");
				//Serial.println(length);

				if (buffer[window + offsetof(DetectionDatagramm, head)] != 0xAA)
				{
					//Serial.println("Missing head at: ");
					//printBytes(buffer, bufferSize, window + offsetof(DetectionDatagramm, head));
					return false;
				}

				if (buffer[window + offsetof(DetectionDatagramm, tail)] != 0x55) {
					//Serial.print("Missing Tail at: ");
					//Serial.println(window + offsetof(DetectionDatagramm, tail));
					//printBytes(buffer, bufferSize, window + offsetof(DetectionDatagramm, tail));
					return false;
				}		

				if (buffer[window + offsetof(DetectionDatagramm, check)] != 0x00) {
					//Serial.print("Missing Check at: ");
					//Serial.println(window + offsetof(DetectionDatagramm, check));
					//printBytes(buffer, bufferSize, window + offsetof(DetectionDatagramm, check));
					return false;
				}

				if (buffer[window + offsetof(DetectionDatagramm, MFR)] != 0xF8 || 
					buffer[window + offsetof(DetectionDatagramm, MFR) + 1] != 0xF7 || 
					buffer[window + offsetof(DetectionDatagramm, MFR) + 2] != 0xF6 || 
					buffer[window + offsetof(DetectionDatagramm, MFR) + 3] != 0xF5)
				{
					//Serial.print("Missing MFR at:");
					//printBytes(buffer, bufferSize, window + offsetof(DetectionDatagramm, MFR));
					return false;
				}

				memcpy(out, buffer + window, sizeof(DetectionDatagramm));
				return true;
			}
			else {
				window++;
			}
		}
		return false;
	}

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

enum AckStatus : short {
	SUCCESS = 0x0000, FAILURE = 0x0001,
};

struct ConfigurationCommandResponse
{
	byte HEADER[4]{ 0xFD, 0xFC, 0xFB, 0xFA };
	short length;//should be 8
	byte command[2];
	AckStatus ackStatus;
	byte protocollVersion[2];
	byte buffer[2];
	byte MFR[4]{ 4,3,2,1 };

	static bool tryRead(byte* buffer, size_t bufferSize, ConfigurationCommandResponse* out) {
		size_t window = 0;
		while (window + sizeof(ConfigurationCommandResponse) < bufferSize) {
			if (buffer[window] != 0xFD ||
				buffer[window + 1] != 0xFC ||
				buffer[window + 2] != 0xFB || 
				buffer[window + 3] != 0xFA) {
				window++;
				continue;
			}
			short length = buffer[window + offsetof(ConfigurationCommandResponse, length) + 1] << 8 | buffer[window + offsetof(ConfigurationCommandResponse, length)];


			if (length != 8) {
				return false;
			}
			if (buffer[window + offsetof(ConfigurationCommandResponse, command)] != 0xFF || buffer[window + offsetof(ConfigurationCommandResponse, command) + 1] != 0x01)
			{
				//printBytes(buffer[window + offsetof(ConfigurationCommandResponse, command)], 2, -1);
				return false;
			}

			

			if (buffer[window + offsetof(ConfigurationCommandResponse, MFR)] != 0x04 ||
				buffer[window + offsetof(ConfigurationCommandResponse, MFR) + 1] != 0x03 ||
				buffer[window + offsetof(ConfigurationCommandResponse, MFR) + 2] != 0x02 ||
				buffer[window + offsetof(ConfigurationCommandResponse, MFR) + 3] != 0x01)
			{
				return false;
			}

			memcpy(out, buffer + window, sizeof(ConfigurationCommandResponse));
			return true;

		}

		return false;
	}

	void prettyPrint() {
		Serial.print("ConfigurationCommandResponse: ");
		Serial.print("Ack status: ");
		switch (ackStatus)
		{
		case SUCCESS:
			Serial.println("SUCCESS");
			break;
		case FAILURE:
			Serial.println("FAILURE");
			break;
		default:
			Serial.println("UNKNOWN");
			break;
		}
	}
};

struct FirmwareCommandResponse {
	byte HEADER[4]{ 0xFD, 0xFC, 0xFB, 0xFA };
	short length;//should be 12
	byte command[2];
	AckStatus ackStatus;
	byte firmwareType[2];
	byte majorVersion[2];
	byte minorVersion[4];
	byte MFR[4]{ 4,3,2,1 };


	static bool tryRead(byte* buffer, size_t bufferSize, FirmwareCommandResponse* out) {
		size_t window = 0;
		while (window + sizeof(FirmwareCommandResponse) < bufferSize) {
			if (buffer[window] != 0xFD ||
				buffer[window + 1] != 0xFC ||
				buffer[window + 2] != 0xFB ||
				buffer[window + 3] != 0xFA) {
				window++;
				continue;
			}
			short length = buffer[window + offsetof(FirmwareCommandResponse, length) + 1] << 8 | buffer[window + offsetof(FirmwareCommandResponse, length)];
			
			if (length != 12) {
				return false;
			}

			if (buffer[window + offsetof(FirmwareCommandResponse, MFR)] != 0x04 ||
				buffer[window + offsetof(FirmwareCommandResponse, MFR) + 1] != 0x03 ||
				buffer[window + offsetof(FirmwareCommandResponse, MFR) + 2] != 0x02 ||
				buffer[window + offsetof(FirmwareCommandResponse, MFR) + 3] != 0x01)
			{
				return false;
			}

			memcpy(out, buffer + window, sizeof(FirmwareCommandResponse));
			return true;

		}

		return false;
	}

	void prettyPrint() {
		Serial.print("FirmwareCommandResponse: ");
		Serial.print("V");
		Serial.print(majorVersion[1], HEX);
		Serial.print(".");
		printByte(majorVersion[0]);

		Serial.print(".");
		printByte(minorVersion[3]);
		printByte(minorVersion[2]);
		printByte(minorVersion[1]);
		printByte(minorVersion[0]);

		Serial.println();
	}
};


void printByte(byte b) {
	if (b < 16)
	{
		Serial.print("0");
	}
	Serial.print(b, HEX);
}

void printBytes(const uint8_t* data, size_t len, int highlightAt) {
	for (size_t i = 0; i < len; i++)
	{
		if (data[i] < 16) {
			Serial.print(0);
		}
		Serial.print(data[i],HEX);
	}
	if (highlightAt > -1 && highlightAt < len) {
		Serial.println();
		for (size_t i = 0; i < highlightAt; i++)
		{
			Serial.print("  ");
		}
		Serial.println("^^");
	}
}


void sendMessage(void* buf, short size) {
	ldSerial.write(0xFD);
	ldSerial.write(0xFC);
	ldSerial.write(0xFB);
	ldSerial.write(0xFA);

	ldSerial.write((byte*) &size, 2);
	ldSerial.write((byte*) buf, size);

	ldSerial.write(0x04);
	ldSerial.write(0x03);
	ldSerial.write(0x02);
	ldSerial.write(0x01);
	ldSerial.flush();
}

struct {
	byte payload[2]{ 0xA0, 0x00 };
} GET_FIRMWARE;

struct {
	byte commandWord[2]{ 0xFF, 0x00 };
	byte commandValue[2]{ 0x01, 0x00 };
} ENABLE_CONFIG_COMMANDS;

struct {
	byte commandWord[2]{ 0xFE, 0x00 };
} DISABLE_CONFIG_COMMANDS;


void setup() {
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Native USB only
	}

	pinMode(6, INPUT);
	pinMode(5, OUTPUT);

	Serial.println("initializing ldserial");

	ldSerial.begin(57600);
	Serial.println("Initialized ldserial");

}


byte buffer[50]{ 0 };

unsigned int cursor = 0;

// the loop function runs over and over again forever
void loop() {
	if (Serial.available())
	{
		auto command = Serial.read();
		if (command == 'a') {
			Serial.print("Got command: ");
			Serial.println(command);
			sendMessage(&ENABLE_CONFIG_COMMANDS, (short)sizeof(ENABLE_CONFIG_COMMANDS));
		}		
		if (command == 'b') {
			Serial.print("Got command: ");
			Serial.println(command);
			sendMessage(&DISABLE_CONFIG_COMMANDS, (short)sizeof(DISABLE_CONFIG_COMMANDS));
		}
		if (command == 'c') {
			Serial.print("Got command: ");
			Serial.println(command);
			sendMessage(&GET_FIRMWARE, (short)sizeof(GET_FIRMWARE));
		}
	}

	auto available = ldSerial.available();
	if (available > 0) {
		if (cursor == sizeof(buffer) - 1) {
			for (size_t i = 0; i < sizeof(buffer) - 1; i++)
			{
				buffer[i] = buffer[i + 1];
			}

			buffer[cursor] = ldSerial.read();
		}
		else {
			buffer[cursor] = ldSerial.read();
			cursor++;
		}

		DetectionDatagramm dd;
		auto isDetectionDatagramm = DetectionDatagramm::tryRead(buffer, sizeof(buffer), &dd);
		
		if (isDetectionDatagramm) {
			dd.prettyPrint();
			Serial.println("Resetting buffer");
			memset(buffer, 0, sizeof(buffer));
			cursor = 0;
			return;
		}

		ConfigurationCommandResponse cCR;
		auto isConfigurationCommandResponse = ConfigurationCommandResponse::tryRead(buffer, sizeof(buffer), &cCR);
		if (isConfigurationCommandResponse) {
			cCR.prettyPrint();
			Serial.println("Resetting buffer");
			memset(buffer, 0, sizeof(buffer));
			cursor = 0;
		}

		FirmwareCommandResponse fCR;
		auto isFirmwareCommandResponse = FirmwareCommandResponse::tryRead(buffer, sizeof(buffer), &fCR);
		if (isFirmwareCommandResponse) {
			fCR.prettyPrint();
			Serial.println("Resetting buffer");
			memset(buffer, 0, sizeof(buffer));
			cursor = 0;
		}
	}
	
	

	//printBytes(buffer, sizeof(buffer), -1);
	//Serial.println();
}


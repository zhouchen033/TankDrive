
// left motor pins
const int L1 = 3;
const int L2 = 5;
const int L3 = 6;
const int L4 = 9;


// RC Receiver
const byte ch1_pin = 12;
const byte ch2_pin = 4;

// ultraSonic pins
const byte echo_pin = 10;
const byte trig_pin = 2;

// sound detector pin
const byte sod_pin = 7;

byte MSG_STATE = 0;
byte MSG_ID = -1;

byte payload[5];
byte payload_size = 0;

byte arrV[7];
const byte HEADER = 0xAA;
const byte ENDER = 0xAB;
// MSGID LIST
const byte MSGID_XY = 0xF1;
const byte MSGID_SEN = 0xFA;

int ch1RawData = 0;
int ch2RawDate = 0;
int ch1Val = 0;
int ch2Val = 0;

void setup()
{
	Serial.begin(9600);
	pinMode(L1, OUTPUT);
	pinMode(L2, OUTPUT);
	pinMode(L3, OUTPUT);
	pinMode(L3, OUTPUT);
	pinMode(echo_pin, INPUT);
	pinMode(trig_pin, OUTPUT);
	pinMode(sod_pin, INPUT);

	// turn on leds
	/*digitalWrite(led1_pin, HIGH);
	digitalWrite(led2_pin, HIGH);
	digitalWrite(led3_pin, HIGH);
	digitalWrite(led4_pin, HIGH);*/
	pinMode(ch1_pin, INPUT);
	pinMode(ch2_pin, INPUT);
}

void buzz() {
	for (int i = 0; i < 2; i++) {
		tone(8, 150, 500);
		delay(100);
		noTone(8);
	}
}

// Add the main program code into the continuous loop() function
void parseMsg(char c) {
	byte b = (byte)c;
	switch (MSG_STATE) {
	case 0x00:
		//Serial.println("STATE 00");
		if (b == 0xAA) {
			MSG_STATE = 1;
			MSG_ID = -1;
			payload_size = 0;
		}
		break;
	case 0x01:
		MSG_STATE = 2;
		MSG_ID = b;
		break;
	case 0x02:
		if (b == 0xAB) {
			MSG_STATE = 0;
		}
		else {
			payload[payload_size++] = b;
		}
		break;
	}
}

void driveMotor() {
	// ch2-x, -+
	// ch1-y, up- down+
	byte base = 100;
	byte maxPow = 255;
	byte step = (maxPow - base) / 100;

	int x, y = 0;

	if (abs(ch1Val) > 10 || abs(ch2Val) > 10) {
		if (ch2Val < 0) {
			// forwards
			x = base + abs(ch2Val)*step + 2 * (ch1Val)* step;
			y = base + abs(ch2Val)*step - 2 * (ch1Val)* step;

			analogWrite(L1, 0);
			analogWrite(L2, x);
			analogWrite(L3, 0);
			analogWrite(L4, y);
		}
		else {
			// backwards
			x = base + abs(ch2Val)*step + 2 * (ch1Val)* step;
			y = base + abs(ch2Val)*step - 2 * (ch1Val)* step;

			analogWrite(L1, x);
			analogWrite(L2, 0);
			analogWrite(L3, y);
			analogWrite(L4, 0);
		}
	}
	else {
		analogWrite(L1, 0);
		analogWrite(L2, 0);
		analogWrite(L3, 0);
		analogWrite(L4, 0);
	}
}

byte readDist() {
	digitalWrite(trig_pin, LOW);
	delayMicroseconds(2);
	digitalWrite(trig_pin, HIGH);
	delayMicroseconds(10);
	digitalWrite(trig_pin, LOW);
	float dist = pulseIn(echo_pin, HIGH) / 58.0;
	if (dist > 255) {
		return 255;
	}
	else {
		return (byte)dist;
	}
}

void readSensorAndSendOut() {
	arrV[0] = HEADER;
	arrV[1] = MSGID_SEN;

	// Distance sensor
	arrV[2] = readDist();

	// Temperature sensor
	float temp = (5.0 * analogRead(A1) * 100.0) / 1024;
	int tmpDec = (int)temp;
	float rms = temp - tmpDec;
	int rmd = (int)(rms * 10);
	arrV[3] = (byte)tmpDec;
	arrV[4] = (byte)rmd;

	// sound sensor
	int val = digitalRead(sod_pin);
	arrV[5] = byte(val);
	arrV[6] = ENDER;
	Serial.flush();
	Serial.write(arrV, 7);

}

void executeCMD() {
	//driveMotor();
}

void loop()
{
	ch2RawDate = pulseIn(ch2_pin, HIGH);
	ch1RawData = pulseIn(ch1_pin, HIGH);

	ch1Val = map(ch1RawData, 999, 2000, -100, 100);
	ch2Val = map(ch2RawDate, 999, 2000, -100, 100);

	Serial.print("A");

	// work around
	digitalWrite(L1, HIGH);
	digitalWrite(L2, LOW);
	digitalWrite(L3, HIGH);
	digitalWrite(L4, LOW);
	delay(1000);

	digitalWrite(L1, LOW);
	digitalWrite(L2, LOW);
	digitalWrite(L3, LOW);
	digitalWrite(L4, LOW);
	delay(1000);

	digitalWrite(L1, LOW);
	digitalWrite(L2, HIGH);
	digitalWrite(L3, LOW);
	digitalWrite(L4, HIGH);
	delay(1000);

	executeCMD();
	// read sensors & send out
	readSensorAndSendOut();
	delay(300);

}


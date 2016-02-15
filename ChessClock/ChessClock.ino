/*
 Name:		ChessClock.ino
 Created:	2/15/2016 8:37:45 PM
 Author:	Rickard Staaf
*/

// the setup function runs once when you press reset or power the board
#include <Encoder.h>
#include <Bounce2.h>
#include <LiquidCrystal.h>

int WHITE = 0;
int BLACK = 1;
LiquidCrystal lcd[2] = { LiquidCrystal(10, 9, 4, 5, 6, 7), LiquidCrystal(10, 8, 4, 5, 6, 7) };
Encoder rotarySelector(3, 2);
Bounce rotaryButton = Bounce();
//const int rotaryButton = 11;
const int moveButton[2] = { A0, A1 };

unsigned long countdown = 9500000;

unsigned long moveStartMillis[2] = { 0,0 };
unsigned long currentTime[2] = { 0,0 };
long moveTime[2] = { 0,0 };
unsigned long currentMoveTime[2] = { 0,0 };
unsigned long startTime[2] = { 7200000,7200000 };
unsigned long additionalTime[2] = { 3600000,3600000 };
unsigned long additionalTimeMove[2] = { 40,40 };
unsigned long preDelay[2] = { 5000,5000 };
long currentDelay[2] = { 0,0 };
int move[2] = { 0,0 };
String modes[5] = { "Sudden Death","Fischer Delay","Bronstein Delay","Simple Delay","Hourglass" };
String modesShort[5] = { "SuDe","FiDe","BrDe","SiDe","HoGl" };
int mode = 3;
int currentMove;
int rotarySelectorValue;
int adjRSV;
int prevAdjRSV;
String interfaceModes[4] = { "SETUP", "PREGAME", "GAME", "POSTGAME" };
int interfaceMode = 0;
String setupModes[3] = { "COLOR", "GAMEMODE", "STARTTIME" };
int setupMode = 0;
String timeSetupModes[6] = { "WHITE START", "BLACK START", "WHITE ADDITIONAL", "BLACK ADDITIONAL", "WHITE DELAY", "BLACK DELAY" };
unsigned long* timeSetups[6] = { &startTime[WHITE], &startTime[BLACK], &additionalTime[WHITE], &additionalTime[BLACK], &preDelay[WHITE], &preDelay[BLACK] };
long timeSetupMultipliers[6] = { 60000,60000,60000,60000,1000,1000 };
int timeSetupMode = 0;

byte customChar0[8] = {
	0b00011,
	0b00011,
	0b00011,
	0b00011,
	0b00011,
	0b00011,
	0b00011,
	0b00011
};
byte customChar1[8] = {
	0b00011,
	0b00011,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};
byte customChar2[8] = {
	0b11111,
	0b11111,
	0b00011,
	0b00011,
	0b00011,
	0b00011,
	0b00011,
	0b00011
};
byte customChar3[8] = {
	0b11111,
	0b11111,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};
byte customChar4[8] = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b01100,
	0b01100,
	0b01100,
	0b00000
};

void setup() {
	prepareLcd(&lcd[WHITE]);
	prepareLcd(&lcd[BLACK]);
	rotaryButton.attach(11, INPUT_PULLUP);
	pinMode(moveButton[WHITE], INPUT);
	pinMode(moveButton[BLACK], INPUT);
	rotarySelector.write(1000);
}

void prepareLcd(LiquidCrystal *lcd) {
	lcd->createChar(0, customChar0);
	lcd->createChar(1, customChar1);
	lcd->createChar(2, customChar2);
	lcd->createChar(3, customChar3);
	lcd->createChar(4, customChar4);
	lcd->begin(20, 4);
	lcd->clear();
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (interfaceModes[interfaceMode] == "SETUP") {
		if (setupModes[setupMode] == "COLOR") {
			readRotarySelector();
			lcd[WHITE].setCursor(0, 0);
			lcd[WHITE].print("WHITE");
			lcd[BLACK].setCursor(0, 0);
			lcd[BLACK].print("BLACK");
			if (adjRSV % 2 == 0) {
				WHITE = 0;
				BLACK = 1;
			}
			else {
				WHITE = 1;
				BLACK = 0;
			}
			if (rotaryButton.update() == 1 && rotaryButton.read() == LOW) {
				lcd[WHITE].clear();
				lcd[BLACK].clear();
				setupMode = 1;
			}
		}
		if (setupModes[setupMode] == "GAMEMODE") {
			readRotarySelector();
			if (adjRSV % 5 != mode) {
				mode = adjRSV % 5;
				lcd[BLACK].clear();
			}
			lcd[WHITE].setCursor(0, 0);
			lcd[WHITE].print("MODE");
			lcd[BLACK].setCursor(0, 0);
			lcd[BLACK].print(modes[mode]);
			if (rotaryButton.update() == 1 && rotaryButton.read() == LOW) {
				lcd[WHITE].clear();
				lcd[BLACK].clear();
				rotarySelector.write(0);
				setupMode = 2;
			}
		}
		if (setupModes[setupMode] == "STARTTIME") {
			readRotarySelector();
			long adjustedTime = *timeSetups[timeSetupMode] + (adjRSV * timeSetupMultipliers[timeSetupMode]);
			if (adjustedTime <= 0) {
				adjustedTime = 0;
				rotarySelector.write(-(*timeSetups[timeSetupMode] / timeSetupMultipliers[timeSetupMode]) * 4);
			}
			lcd[WHITE].setCursor(0, 0);
			lcd[WHITE].print(timeSetupModes[timeSetupMode]);
			lcd[BLACK].setCursor(0, 0);
			writeMillis(adjustedTime, &lcd[BLACK]);
			if (rotaryButton.update() == 1 && rotaryButton.read() == LOW) {
				lcd[WHITE].clear();
				lcd[BLACK].clear();
				*timeSetups[timeSetupMode] = adjustedTime;
				timeSetupMode++;
				rotarySelector.write(0);
				if (timeSetupMode == 6)
				{
					interfaceMode = 1;
				}
			}
		}
	}
	if (interfaceModes[interfaceMode] == "PREGAME") {
		interfaceMode = 2;
		moveStartMillis[WHITE] = millis();
		moveStartMillis[BLACK] = millis();
		currentTime[WHITE] = startTime[WHITE];
		currentTime[BLACK] = startTime[BLACK];
		moveTime[WHITE] = currentTime[WHITE];
		moveTime[BLACK] = currentTime[BLACK];
		move[WHITE] = 1;
		currentMove = WHITE;
	}
	if (interfaceModes[interfaceMode] == "GAME") {
		handleButtonPresses(WHITE);
		handleButtonPresses(BLACK);
		if (currentMove == WHITE) {
			calculateMoveTime(WHITE);
		}
		else {
			calculateMoveTime(BLACK);
		}
		writeMillis(moveTime[WHITE], &lcd[WHITE]);
		writeMillis(moveTime[BLACK], &lcd[BLACK]);
		updateStatus();
		if (moveTime[BLACK] == 0 || moveTime[WHITE] == 0) {
			interfaceMode = 3;
			lcd[WHITE].clear();
			lcd[BLACK].clear();
		}
	}
	if (interfaceModes[interfaceMode] == "POSTGAME") {
		if (currentMoveTime[BLACK] == 0) {
			writeWin(BLACK);
		}
		else {
			writeWin(WHITE);
		}
	}
}

void writeWin(int COLOR) {
	int OTHER = COLOR == WHITE ? BLACK : WHITE;
	lcd[COLOR].print("WINNER WINNER WINNER");
	lcd[OTHER].print("LOOSER LOOSER LOOSER");
}

void calculateMoveTime(int COLOR) {
	currentMoveTime[COLOR] = millis() - moveStartMillis[COLOR];
	if (modes[mode] == "Sudden Death") {
		moveTime[COLOR] = currentTime[COLOR] - (currentMoveTime[COLOR]);
		if (moveTime[COLOR] <= 0) {
			moveTime[COLOR] = 0;
		}
	}
	if (modes[mode] == "Simple Delay") {
		currentDelay[COLOR] = preDelay[COLOR] - currentMoveTime[COLOR];
		if (currentDelay[COLOR] <= 0) {
			currentDelay[COLOR] = 0;
			moveTime[COLOR] = currentTime[COLOR] - (currentMoveTime[COLOR]) + preDelay[COLOR];
			if (moveTime[COLOR] <= 0) {
				moveTime[COLOR] = 0;
			}
		}
	}
}

void handleButtonPresses(int COLOR) {
	int OTHER = COLOR == WHITE ? BLACK : WHITE;
	if (currentMove == COLOR && digitalRead(moveButton[COLOR]) == LOW) {
		currentMove = OTHER;
		move[OTHER]++;
		moveStartMillis[OTHER] = millis();
		currentTime[COLOR] = moveTime[COLOR];
		if (move[OTHER] == additionalTimeMove[OTHER] + 1) {
			currentTime[OTHER] += additionalTime[OTHER];
			moveTime[OTHER] += additionalTime[OTHER];
		}
	}
}

void readRotarySelector() {
	prevAdjRSV = adjRSV;
	rotarySelectorValue = rotarySelector.read();
	adjRSV = (abs(rotarySelectorValue) + 1) / 4;
	if (rotarySelectorValue < 0) {
		adjRSV = -adjRSV;
	}
}

void updateStatus() {
	lcd[WHITE].setCursor(0, 3);
	lcd[BLACK].setCursor(0, 3);
	lcd[WHITE].print(createStatus(WHITE));
	lcd[BLACK].print(createStatus(BLACK));
}

String createStatus(int COLOR) {
	String status;
	status = (COLOR == WHITE ? "W:" : "B:") + String(move[COLOR]);
	if (move[COLOR] < 100) {
		status += " ";
	}
	if (move[COLOR] < 10) {
		status += " ";
	}
	status += "   " + String(currentDelay[COLOR] / 60000) + ":" + intToString(((currentDelay[COLOR] % 60000 + 999) / 1000));
	int spaceNeeded = 16 - status.length();
	for (int i = 0; i < spaceNeeded; i++) {
		status += " ";
	}
	status += modesShort[mode];
	return status;
}

String intToString(int number) {
	String out = number < 10 ? "0" : "";
	out += String(number);
	return out;
}

void writeMillis(unsigned long millis, LiquidCrystal *lcd) {
	int hours = millis / 3600000;
	int minutes = (millis % 3600000) / 60000;
	int seconds = ((millis % 3600000) % 60000) / 1000;
	int tens = (((millis % 3600000) % 60000) % 1000) / 100;
	writeTime(hours, minutes, seconds, tens, lcd);
}

void writeTime(int hours, int minutes, int seconds, int tens, LiquidCrystal *lcd) {
	writeNumber(hours / 10, 1, lcd);
	writeNumber(hours % 10, 2, lcd);
	writeNumber(-1, 3, lcd);
	writeNumber(minutes / 10, 4, lcd);
	writeNumber(minutes % 10, 5, lcd);
	writeNumber(-1, 6, lcd);
	writeNumber(seconds / 10, 7, lcd);
	writeNumber(seconds % 10, 8, lcd);
	lcd->setCursor(19, 0);
	lcd->print(tens);
}

void writeNumber(int number, int position, LiquidCrystal *lcd) {
	switch (number) {
	case 0:
		lcd->setCursor(position * 2, 0);
		lcd->write((uint8_t)0);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 1);
		lcd->write((uint8_t)0);
		lcd->write((uint8_t)0);
		lcd->setCursor(position * 2, 2);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)3);
		break;
	case 1:
		lcd->setCursor(position * 2, 0);
		lcd->write(" ");
		lcd->write((uint8_t)0);
		lcd->setCursor(position * 2, 1);
		lcd->write(" ");
		lcd->write((uint8_t)0);
		lcd->setCursor(position * 2, 2);
		lcd->write(" ");
		lcd->write((uint8_t)1);
		break;
	case 2:
		lcd->setCursor(position * 2, 0);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 1);
		lcd->write((uint8_t)0);
		lcd->write((uint8_t)3);
		lcd->setCursor(position * 2, 2);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)3);
		break;
	case 3:
		lcd->setCursor(position * 2, 0);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 1);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 2);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)3);
		break;
	case 4:
		lcd->setCursor(position * 2, 0);
		lcd->write((uint8_t)0);
		lcd->write((uint8_t)0);
		lcd->setCursor(position * 2, 1);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 2);
		lcd->write(" ");
		lcd->write((uint8_t)1);
		break;
	case 5:
		lcd->setCursor(position * 2, 0);
		lcd->write((uint8_t)0);
		lcd->write((uint8_t)3);
		lcd->setCursor(position * 2, 1);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 2);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)3);
		break;
	case 6:
		lcd->setCursor(position * 2, 0);
		lcd->write((uint8_t)0);
		lcd->write(" ");
		lcd->setCursor(position * 2, 1);
		lcd->write((uint8_t)0);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 2);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)3);
		break;
	case 7:
		lcd->setCursor(position * 2, 0);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 1);
		lcd->write(" ");
		lcd->write((uint8_t)0);
		lcd->setCursor(position * 2, 2);
		lcd->write(" ");
		lcd->write((uint8_t)1);
		break;
	case 8:
		lcd->setCursor(position * 2, 0);
		lcd->write((uint8_t)0);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 1);
		lcd->write((uint8_t)0);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 2);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)3);
		break;
	case 9:
		lcd->setCursor(position * 2, 0);
		lcd->write((uint8_t)0);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 1);
		lcd->write((uint8_t)1);
		lcd->write((uint8_t)2);
		lcd->setCursor(position * 2, 2);
		lcd->write(" ");
		lcd->write((uint8_t)1);
		break;
	case -1:
		lcd->setCursor(position * 2, 0);
		lcd->write(" ");
		lcd->write((uint8_t)4);
		lcd->setCursor(position * 2, 1);
		lcd->write(" ");
		lcd->write((uint8_t)4);
		break;
	}
}

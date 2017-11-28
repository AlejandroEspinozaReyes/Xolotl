#include<Servo.h>
#include<Wire.h>
#include<EmonLib.h>
#include<ESP8266Manager.h>
#include<SoftwareSerial.h>

Servo myServo;
int positionIDLE = 90;
int positionON = 10;
int positionOFF = 170;
int servoPin = 11;
boolean switchState = false;

const int baudRate = 9600;
ESP8266Manager espManager(baudRate, 3, 2);

void setup() {
    Serial.begin(baudRate);
    pinMode(A0, INPUT);

    // Setting servo to the idle position
    Serial.println("ATTACHING THE SERVO");
    myServo.attach(servoPin);
    delay(2000);
    myServo.write(positionIDLE);
    delay(2000);
    myServo.detach();

    String response;
    do {
        Serial.println("PINGING WIFI MODULE");
        response = espManager.writeESP8266("AT\r\n", 6000);
    } while (!espManager.findText("OK", response));
    Serial.println("WIFI MODULE OK");
}

void loop() {
    boolean newSwitchState = readSwitchState();

    if (newSwitchState != switchState) {
        switchState = newSwitchState;
        Serial.println("NEW SWITCH STATE DETECTED");
        setSwitchState();
    }

    delay(5000);
    getSwitchState();
    delay(5000);
}

boolean readSwitchState() {
    int readACDelay = readAC();

    if (readACDelay > 0) {
        return true;
    } else {
        return false;
    }
}

int readAC() {
    double maxValue = 0;
    double readValue = 0;
    uint32_t readEndTime = millis() + 100;

    while (millis() < readEndTime) {
        double readValue = analogRead(A0);
        if (readValue > maxValue) {
            maxValue = readValue / 10;
        }
    }

    Serial.print("READ DOUBLE VALUE: ");
    Serial.println(maxValue);
    Serial.print("READ INT VALUE: ");
    Serial.println((int)maxValue);
    return (int) maxValue;
}

void changeSwitchState(boolean newSwitchState) {
    switchState = newSwitchState;

    if (switchState) {
        myServo.attach(servoPin);
        delay(1000);
        myServo.write(positionON);
        delay(1000);
        myServo.write(positionIDLE);
        delay(1000);
        myServo.detach();
    } else {
        myServo.attach(servoPin);
        delay(1000);
        myServo.write(positionOFF);
        delay(1000);
        myServo.write(positionIDLE);
        delay(1000);
        myServo.detach();
    }
}

bool connect() {
    Serial.println("CONNECTING TO ACCESS POINT");
    String response = espManager.writeESP8266("AT+CWJAP=\"LEXRULES\",\"lexrules\"", 5000); 
    delay(2000);
    return isConnected();
}

bool isConnected() {
    Serial.println("IS CONNECTED TO ACCESS POINT ?");
    String response = espManager.writeESP8266("AT+CWJAP?\r\n", 5000); 

    if (espManager.findText("OK", response)) {
        if (espManager.findText("No AP", response)) {
            Serial.println("NO, IT'S NOT CONNECTED");
            return false;
        } else {
            Serial.println("YES, IT'S CONNECTED");
            return true;
        }
    } else {
        Serial.println("DON'T KNOW IF IT'S CONNECTED");
        return false;
    }
}

bool getSwitchState() {
    Serial.println("CALLING WS");
    String response = espManager.writeESP8266("AT+CIPSTART=\"TCP\",\"192.168.1.70\",8080\r\n", 3000); 

    if (espManager.findText("OK", response)) {
        Serial.println("CONNECTED TO THE SERVER");
    } else {
        Serial.println("CONNECTION IMPOSSIBLE");
        return false;
    }

    Serial.println("SENDING DATA LENGTH");
    response = espManager.writeESP8266("AT+CIPSEND=52\r\n", 3000); 

    if (espManager.findText(">", response)) {
        Serial.println("CONNECTED TO THE SERVER");
        Serial.println("SENDING THE PATH");
        espManager.writeESP8266("GET /Quetzalcoatl/iot/SwitchService/state\r\n", 100); 
        Serial.println("SENDING THE PARAMS");
        response = espManager.writeESP8266("HTTP/1.1\r\n", 15000); 
        if (espManager.findText("true", response) && !switchState) {
            Serial.println("OPEN THE SWITCH");
            changeSwitchState(true);
        } else if (espManager.findText("false", response) && switchState) {
            Serial.println("CLOSE THE SWITCH");
            changeSwitchState(false);
        } else {
            Serial.println("----- NO ACTION");
        }
    } else {
        Serial.println("ERROR IN CONNECTION");
        espManager.writeESP8266("AT+CIPCLOSE\r\n", 3000); 
        return false;
    }
}

bool setSwitchState() {
    String state = switchState ? "true" : "false";

    Serial.println("CALLING WS");
    String response = espManager.writeESP8266("AT+CIPSTART=\"TCP\",\"192.168.1.70\",8080\r\n", 3000); 

    if (espManager.findText("OK", response)) {
        Serial.println("CONNECTED TO THE SERVER");
    } else {
        Serial.println("CONNECTION IMPOSSIBLE");
        return false;
    }

    Serial.println("SENDING DATA LENGTH");
    response = espManager.writeESP8266("AT+CIPSEND=52\r\n", 3000); 

    if (espManager.findText(">", response)) {
        Serial.println("CONNECTED TO THE SERVER");
        Serial.println("SENDING THE PATH");
        espManager.writeESP8266("GET /Quetzalcoatl/iot/SwitchService/state/" + state + "\r\n", 100); 
        Serial.println("SENDING THE PARAMS");
        response = espManager.writeESP8266("HTTP/1.1\r\n", 15000); 
        if (espManager.findText("true", response) && !switchState) {
            Serial.println("OPEN THE SWITCH");
            changeSwitchState(true);
        } else if (espManager.findText("false", response) && switchState) {
            Serial.println("CLOSE THE SWITCH");
            changeSwitchState(false);
        } else {
            Serial.println("----- NO ACTION");
        }
    } else {
        Serial.println("ERROR IN CONNECTION");
        espManager.writeESP8266("AT+CIPCLOSE\r\n", 3000); 
        return false;
    }
}

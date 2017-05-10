#include <Wire.h>





int16_t readBuffer[6];
float fb[3];
void setup() {
//	Serial.begin(115200);
	pinMode(5, OUTPUT);//t0
	pinMode(6, OUTPUT);

	pinMode(3, OUTPUT);//t3
	pinMode(11, OUTPUT);

	pinMode(9, OUTPUT);//t2
	pinMode(10, OUTPUT);


	int val = 254;
	//all 2000hz in 16mhz
	TCCR0A = 163;
	TCCR0B = 3;
	OCR0A = val;
	OCR0B = val;

	TCCR2A = 163;
	TCCR2B = 4;
	OCR2A = val;
	OCR2B = val;

	TCCR1A = 163;
	TCCR1B = 2;
	OCR1A = 127 * 4;
	OCR1B = 127 * 4;


  Wire.begin(9);
  Wire.onRequest(requestEvent); // data request to slave
  Wire.onReceive(receiveEvent); // data slave received

  fb[0] = analogRead(0);
  fb[1] = analogRead(1);
  fb[2] = analogRead(2);

}



int temp;

#define writePWMg(n)						\
	if (Wire.available()) {				\
		temp = Wire.read();				\
		n=temp*4;							\
		}

#define writePWM(n)						\
	if (Wire.available()) {				\
		temp = Wire.read();				\
		n=temp;							\
		}								
	


void receiveEvent(int countToRead) {

	int av = Wire.available();
	if (av == 4) {
		//Serial.println(8);

		writePWM(OCR0A)
		writePWM(OCR0B)
		writePWM(OCR2A)
		writePWM(OCR2B)
	}
	else if (av==2){
		writePWMg(OCR1A)
		writePWMg(OCR1B)
	}
	else {
		while (Wire.available())  
			Wire.read();
	}

}

//	A0	-	3v from balance
//	A1	-	6v form balance
//	A2	-	9v form power



void requestEvent() {
	readBuffer[0] = fb[0];
	readBuffer[1] = fb[1];
	readBuffer[2] = fb[2];
		
		Wire.write((byte*)readBuffer, 6);

}
#define DELAY 1



void loop() {

	fb[0] += ((float)analogRead(0)- fb[0])*0.03;
	delay(DELAY);
	fb[1] += ((float)analogRead(1) - fb[1])*0.03;
	delay(DELAY);
	fb[2] += ((float)analogRead(2) - fb[2])*0.03;
	delay(DELAY);	
//	Serial.println(fb[2]);
//	Serial.println(analogRead(2));
}

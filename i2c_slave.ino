#include <Wire.h>



#define BEEP 13
uint8_t beeps_coder[] = {0, B00001000,B00001001,B00001010,B00001011,B00001100,B00001101,B00001110,B00001111,B00000001,B00000010,B00000011,B00000100,B00000101,B00000110,B00000111 };//4 beeps if 0 short 1 long beep
int16_t readBuffer[6];
float fb[3];
void setup() {
//	Serial.begin(115200);
	pinMode(BEEP, OUTPUT); //BEEP
	pinMode(5, OUTPUT);//t0
	pinMode(6, OUTPUT);

	pinMode(3, OUTPUT);//t3
	pinMode(11, OUTPUT);

	pinMode(9, OUTPUT);//t2
	pinMode(10, OUTPUT);


	int val = 127;
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

enum COMMANDS_BIT { SOUND_ON = 1,BEEP_CODE=30 };


volatile uint8_t beep_code=0;
volatile uint8_t cnt = 0;
volatile bool beep_on=false;
uint8_t old_cnt = 0;
uint16_t err = 0;

void receiveEvent(int countToRead) {

	int av = Wire.available();
	if (av == 4) {
		writePWM(OCR0A)
		writePWM(OCR0B)
		writePWM(OCR2A)
		writePWM(OCR2B)
	}
	else if (av==2){
		writePWMg(OCR1A)
		writePWMg(OCR1B)
	}
	else  if (av == 1) {
		uint8_t command = Wire.read();
		beep_on		= command&SOUND_ON;
		beep_code =  (command&BEEP_CODE) >> 1;
		
		
	}
	else
	{
		while (Wire.available())  
			Wire.read();
	}
	cnt++;
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

/*/if no commands from orange pi we will shutdown motors or we can read data from mpu and stabilize copter for
	it down out*/
void stop_motors() {
	OCR0A = 127;
	OCR0B = 127;
	OCR2A = 127;
	OCR2B = 127;

}
bool beep_bit = false;
uint16_t beep_cnt = 0;
uint16_t beep_code_cnt;
uint8_t  beep_code_n=0;
bool beep_code_on = false;



void beeps_code() {
	
	beep_code_cnt++;
	if (beep_code_on == false) {
		if (beep_code_cnt > 100) {
			beep_code_cnt = 0;
			beep_code_on = true;
		}
	}
	else {
		bool long_beep = (beeps_coder[beep_code] >> beep_code_n) & 1;
		if (beep_code_cnt > ((long_beep) ? 600 : 200)) {
			beep_code_cnt = 0;
			beep_code_n++;
			beep_code_on = false;
			if (beep_code_n == 4) {
				beep_code_cnt = 0;
				beep_code_n = 0;
				beep_code = 0;
				beep_code_on = false;
			}

		}
		else
			digitalWrite(BEEP, beep_bit ^= true);
	}
}

void beep() {
	if (beep_code>0) {
		beeps_code();
	}else
		if (beep_on) {
			uint16_t beep_cnd = 20000;
			if (fb[2] > 300)
				beep_cnd = (fb[2] < 900) ? 0 : 1;
			else
				if (fb[0] > 300 || fb[1] > 300)
					beep_cnd = 2;

			if (beep_cnd == beep_cnt) {
				beep_cnt ^= beep_cnt;//=0
				digitalWrite(BEEP, beep_bit ^= true);
			}
			else
				beep_cnt++;
		}
}
void update_voltage() {
	fb[0] += ((float)analogRead(0)- fb[0])*0.03;
	beep();
	delay(DELAY);
	fb[1] += ((float)analogRead(1) - fb[1])*0.03;
	beep();
	delay(DELAY);
	fb[2] += ((float)analogRead(2) - fb[2])*0.03;
	beep();
	delay(DELAY);
}
uint8_t err_beep_cnt = 0;
bool err_beep_f = false;
void loop() {
	if (cnt != old_cnt) {
		old_cnt = cnt;
		err = 0;
		
	}
	else {
		err++;
		if (err > 300) {
			stop_motors();
			if ((err_beep_cnt++) > 20) {
				err_beep_f ^= true;
				err_beep_cnt ^= err_beep_cnt;
			}
			if (err_beep_f)
				digitalWrite(BEEP, beep_bit ^= true);
			
		}
	}
	update_voltage();
//	Serial.println(fb[2]);
//	Serial.println(analogRead(2));
}

/*

  FSKCW QRSS MEPT GPS timed.

  I thank the work OF Thomas LA3PNA, Mark K6HX, Jason NT7S, KD1JV and several others for hints and code in this basic "newbie"
  FSKCW QRSS MEPT project, sources found through Github and Google searches.

  I purchased the V.KEL VK16E module on Ebay for AU$4.40. The Adafruit SI5351 for AU12.50.
  The LCD display is an old Nokia type PCD8544 from Jaycar.

  The TX is simple a BC549 rf preamp, with BS170's configured in class E for the final and a Low Pass Filter designed using
  WA4DSY's online calculator and Coil32 coil calculation software program.

  Goto www.qsl.net/vk3edw for pictures, circuit and grabber


  It is published as "do as you want" licence.  John Gilbert VK3EDW January 2017


*/


#include <si5351.h>
#include <Wire.h>
#include <PCD8544.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>



char message[ ] = " VK3EDW"; // Here you put in your beacon message.

int calvalue = 4630;// here you set the cal value for the SI5351

//  frequency is lower    real frequency is 70398320 as seen on zl2ik grabber
unsigned long long int frequencylow = 70398430 ; // here you set the beacon nominal frequency

unsigned long long int frequencyhigh = 70398480 ; // here you set the beacon send frequency

unsigned long long int frequencystandby = 10000 ; // here you set the MEPT standby frequency, this gives a smooth start to transmit sequence.

//unsigned long long int frequencylow = 101399350 ; // here you set the beacon nominal frequency

//unsigned long long int frequencyhigh = 101399410 ; // here you set the beacon send frequency






int LedPin = 13;        //Led to show transmissions

Si5351 si5351;// The adafruit si5351 sig gen

static PCD8544 lcd;// The lcd display

SoftwareSerial ss(9, 8);// The serial connection to GPS


TinyGPSPlus gps;// The TinyGPS++












// establish structure of morse code remove or add  // where appropriate for your message. This reduces dynamic memory usage

struct t_mtab {
  char c, pat;
} ;

struct t_mtab morsetab[] = {
  //{'.', 106},
  //{',', 115},
  //{'?', 76},
  //{'/', 41},
  //{'A', 6},
  //{'B', 17},
  //{'C', 21},
  {'D', 9},
  {'E', 2},
  //{'F', 20},
  //{'G', 11},
  //{'H', 16},
  //{'I', 4},
  //{'J', 30},
  {'K', 13},
  //{'L', 18},
  //{'M', 7},
  //{'N', 5},
  //{'O', 15},
  //{'P', 22},
  //{'Q', 27},
  //{'R', 10},
  //{'S', 8},
  //{'T', 3},
  //{'U', 12},
  {'V', 24},
  {'W', 14},
  //{'X', 25},
  //{'Y', 29},
  //{'Z', 19},
  //{'1', 62},
  //{'2', 60},
  {'3', 56},
  //{'4', 48},
  //{'5', 32},
  //{'6', 33},
  //{'7', 35},
  //{'8', 39},
  //{'9', 47},
  //{'0', 63}
} ;

#define N_MORSE  (sizeof(morsetab)/sizeof(morsetab[0]))

#define SPEED  (0.180)              //define speed of cw being sent
#define DOTLEN  (1200/SPEED)        //DOT length set
#define DASHLEN  (3*(1200/SPEED))    //Dash length set



void dash()        //send dash
{

  si5351.set_freq(frequencyhigh, 0, SI5351_CLK0);
  digitalWrite(LedPin, HIGH) ;
  si5351.output_enable(SI5351_CLK0, 1);
  delay(DASHLEN);
  digitalWrite(LedPin, LOW) ;
  si5351.output_enable(SI5351_CLK0, 0);
  si5351.set_freq(frequencylow, 0, SI5351_CLK0);
  si5351.output_enable(SI5351_CLK0, 1);
  delay(DOTLEN) ;
}



void dit()        //send dit
{

  si5351.set_freq(frequencyhigh, 0, SI5351_CLK0);
  digitalWrite(LedPin, HIGH) ;
  si5351.output_enable(SI5351_CLK0, 1);
  delay(DOTLEN);
  digitalWrite(LedPin, LOW) ;
  si5351.output_enable(SI5351_CLK0, 0);
  si5351.set_freq(frequencylow, 0, SI5351_CLK0);
  si5351.output_enable(SI5351_CLK0, 1);
  delay(DOTLEN);
}



void send(char c)        //send routine
{
  int i ;
  if (c == ' ') {
        delay(7 * DOTLEN) ;
    return ;
  }
  for (i = 0; i < N_MORSE; i++) {
    if (morsetab[i].c == c) {
      unsigned char p = morsetab[i].pat ;
            while (p != 1) {
        if (p & 1)
          dash() ;
        else
          dit() ;
        p = p / 2 ;
      }
      delay(2 * DOTLEN) ;
      return ;
    }
  }
}



void setup()        //setup arduino

{

  lcd.begin(84, 48);    //establish LCD display size

  ss.begin(9600);// Begin the gps connection

  pinMode(LedPin, OUTPUT) ;        //Set LedPin as an output
  digitalWrite(LedPin, LOW);      //Set ledPin off

  frequencyhigh = frequencyhigh * 10ULL;    //frequencyhigh multiplier
  frequencylow = frequencylow * 10ULL;      //frequencylow multiplier
  frequencystandby = frequencystandby * 10ULL;    //frequencystandby multiplier

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);    //set si5351 output level
  si5351.set_correction(calvalue);//set si5351 calibration value

  si5351.set_freq(frequencystandby, 0, SI5351_CLK0);    //set si5351 standby frequency
  si5351.output_enable(SI5351_CLK0, 1);                //Turn on si5351 at standby frequency



}

void displayInfo()        //LCD display of GPS UTC time and number of satellites being used
{
  if (gps.time.isValid())
  lcd.setCursor(0, 0);
  if (gps.time.hour() < 10) lcd.print ("0");
  lcd.print(gps.time.hour());
  lcd.print(F(":"));
  if (gps.time.minute() < 10) lcd.print ("0");
  lcd.print(gps.time.minute());
  lcd.print(F(":"));
  if (gps.time.second() < 10) lcd.print ("0");
  lcd.print(gps.time.second());
  lcd.print(F(" UT "));
  lcd.setCursor(70, 0);
  lcd.print(gps.satellites.value());
  gpsgatetime ();
}




void gpsgatetime ()        //setup of transmission times

{
  if (gps.time.minute() == 0 && gps.time.second() == 12)
    senddata();
  if (gps.time.minute() == 10 && gps.time.second() == 12)
    senddata();
  if (gps.time.minute() == 20 && gps.time.second() == 12)
    senddata();
  if (gps.time.minute() == 30 && gps.time.second() == 12)
    senddata();
  if (gps.time.minute() == 40 && gps.time.second() == 12)
    senddata();
  if (gps.time.minute() == 50 && gps.time.second() == 12)
    senddata();
}


void senddata ()          //send routine
{
  si5351.set_freq(frequencylow, 0, SI5351_CLK0);
  si5351.output_enable(SI5351_CLK0, 1);
  delay(1000);
  sendmsg(message);
  delay(2000);
  digitalWrite(LedPin, LOW) ;
  si5351.set_freq(frequencystandby, 0, SI5351_CLK0);
  si5351.output_enable(SI5351_CLK0, 1);
  delay(1000);

}


void sendmsg(char *str)    //sendmsg through data string

{

  while (*str)
    send(*str++);
    delay(250);
  (gps.encode(ss.read()));

}



void loop()

{
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo();

}




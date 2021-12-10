/* 
 * COMPONENTS USED:
 * Arduino Uno R3
 * SimCom SIM800A module
 * DS1307 RTC module
 * Novel NS101 AC-DC Power Adapter (for SIM800A)
 * 10K Potentiometer (Scroll Ring)
 * Pushbuttons (x2)
 * White LED (array of 4x3)
 * Resistors (220R, 330R, 5K, 10K, etc.)
 * 9V battery and adapter with Japan Jack.
 * 4GB SanDisk Memory Card for EEPROM storage
 * Full sized SIM card (900MHz bandwidth)
 * 
 * Libraries used mentioned as comments within code.
 * 
 */


#include <GSMSim.h>                                   /*GSM SIM Library created by user 'erdemarslan' and hosted on github.com at
                                                        https://github.com/erdemarslan/GSMSim  */
#include <Wire.h>
#include <DS1307.h>                                   /*Functional libraries for Serial Communication and DS1307 Real Time Clock at 
                                                       https://github.com/PaulStoffregen/DS1307RTC */
const int pushYes = 3;
const int pushNo = 4;                                //const pin numbers for 'Yes and 'No' buttons
const int ledMatrix = 13;
const int piezoPIN = 9;                              //Piezo speaker connected at PWM D9 to play alarm tones and emit sound on call

#define RX 7                                         //SIM800A Rx pin connected to Arduino D7
#define TX 8                                         //SIM800A Tx pin connected to Arduino D8
#define RESET 2
#define BAUD 9600                                    //Set baud rate on SIM800A to 9600 default

int sensorValue = 0;
int buttonStateYes = 0;
int buttonStateNo = 0;
int UIstate = 1;                                     //Decides what state UI is in Switch statements in stateMachine()
int UIstate_buf = 1;                                 //Saves previous UI state to decide output based on path taken
int UIstate_read = 0;                                //Reading variable to detect change in potentiomater value

int logNumber = -1;                                  //so that first log is saved at logs[0]
char logs[10];                                       //char array for logs that saves last 10 numbers

uint8_t alarmMin, alarmHour;                         //unsigned typedef 8 bit integer for efficient storage
                                                     //global variables for Alarm timings. Can be changed in case 4 and accessed in loop function for alarms

/**********************************************************************************************************************************/
  /* Global variables for RTC*/
  uint8_t sec, min, hour, day, month;
  uint16_t year;                                     //8-bit and 16-bit numbers for time and date

/***********************************************************************************************************************************/

DS1307 rtc;                                     //create RTC object called 'rtc'
GSMSim gsm;                                     //create GSM object gsm with function {GSMSim gsm(RX, TX);} RX,TX default.


void setup()
{
  Serial.begin(9600);                           // Begin serial communication with laptop
  pinMode(pushYes,INPUT);
  pinMode(pushNo,INPUT);                      //pin modes for pushbuttons on 2 and 4
  while(!Serial);                              //wait for serial port to connect
  //init RTC
  Serial.println("Initialising Mobile Phone, please wait");
  //set day + date for RTC
  
  rtc.set(58, 45, 9, 2, 5, 2019);             //09:45:00 2.5.2019 as in sec, min, hour, day, month, year  {58 seconds to start at 00 as program initialises}
  //function for stop/pause RTC - uncomment if required
  // rtc.stop();
  rtc.start();                               // RTC initialised one time. Can be accessed in UIstatetime to display time within stateMachine() and as alarm function in loop()

  gsm.start();                               // baud default 9600 {Function: gsm.start(BAUD);}
  gsm.setRingerVolume(95);                       //On a scale of 0-100
  
  delay(2000);
}


void loop()
{
 UIstate_read = analogRead(A0);
/**********************************************************************************************************************************/
//                            [ FIRST TEST TO CHECK UI STATE ACCORDING TO SCROLL RING]


    if (analogRead(A0) != UIstate_read) {                            // Initial Check for what state mobile phone is in
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 1;
        UIstate_buf = 1;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 2;
        UIstate_buf = 1;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 3;
        UIstate_buf = 1;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {       //No break statements, one time check
        UIstate = 4;
        UIstate_buf = 1;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 5;
        UIstate_buf = 1;
      }
    }
/***********************************************************************************************************************************/
  stateMachine();                       //Main function of machine. Defined later in code
/***********************************************************************************************************************************/  
                                       //CODE BLOCK WHICH PLAYS ALARM AT TIME SET FROM CASE 41.
  
  rtc.get(&sec, &min, &hour, &day, &month, &year);             //Fetch time from RTC at every cycle
  if ((hour == alarmHour) && (min == alarmMin)) {              //If alarm set in hours and minutes matches, play alarm
    for (int i = 0; i < 5; i++){                               //Plays 5 times
      tone( piezoPIN, 2000, 3000);                             //Note combination to play
      tone( piezoPIN, 3000, 4000);
      tone( piezoPIN, 2000, 1500);
      tone( piezoPIN, 4000, 5000);
    }
  }
/************************************************************************************************************************************************/
  
  delay(1000);
}





/************************************************************************************************************************************************************************************
 END OF LOOP FUNCTION
 UP NEXT: stateMachine MAIN RUNNING FUNCTION
*************************************************************************************************************************************************************************************/
//                                                                     [ VOID STATE MACHINE FUNCTION ]

void stateMachine () {
  
  switch(UIstate){

/**********************************************************************************************************************************************************/
 //                                                               [ CLOCK MAIN PAGE ]                                                   
  
  
  
  case 1:  {                               
      //get time from RTC
    rtc.get(&sec, &min, &hour, &day, &month, &year);                     //Codeblock from DS1307 library (refer link at top)
    
    UIstate_read = analogRead(A0);
  
    Serial.print("\n");
    Serial.print(hour, DEC);                                            //DEC decimal base-10 values as second parameter of Serial.print()
    Serial.print(":");
    Serial.print(min, DEC);
    Serial.print(":");
    Serial.print(sec, DEC);
    Serial.print(" am");

    Serial.print("\nThursday ");                                       //  {HOW DO I GET WHAT DAY IT IS FROM AN RTC??}
    Serial.print(day, DEC);
    Serial.print("/");
    Serial.print(month, DEC);
    Serial.print("/");
    Serial.print(year, DEC);

/*****************************************************************/
/*                      state change tests                       */

    if (analogRead(A0) != UIstate_read) {                            // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 1;
        UIstate_buf = 1;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 2;
        UIstate_buf = 1;
        break;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 3;
        UIstate_buf = 1;
        break;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {
        UIstate = 4;
        UIstate_buf = 1;
        break;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 5;
        UIstate_buf = 1;
        break;
      }
    }
/*************************************************************************/

    
    if (UIstate == 1) {                     // if state is still 'clock', then wait for 9 seconds to refresh clock
     delay(9000);
    }
                                            //{CHANGE THIS TO CHANGE REFRESH RATE OF DISPLAYED CLOCK} {Current Rate - once in 9 seconds}
    break;
  }




/***********************************************************************************************************************************************************
END OF SECTION: CLOCK MAIN PAGE (1)                                   COMPLETED
UP NEXT: CALLS MAIN PAGE (2)
************************************************************************************************************************************************************/
//                                                                [CALLS MAIN PAGE]   
                                                                       
  case 2: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    Serial.println("CALLING");

    if (digitalRead(pushYes)) {
      UIstate = 21;                                                      //If "CALLING' is selected, UI shifts to dialler (21)
      UIstate_buf = 2;
      digitalWrite(ledMatrix, HIGH);                                     //matrix of leds under keypad lights up to make keypad visible and hence easy to type with.
      delay(500);
      break;                                                             // break case and go to dialler (21)
      }

   if (digitalRead(pushNo)){
      UIstate = 1;
      UIstate_buf = 2;
      break;
    }
        
/*****************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 1;
        UIstate_buf = 2;
        break;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 2;
        UIstate_buf = 2;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 3;
        UIstate_buf = 2;
        break;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {
        UIstate = 4;
        UIstate_buf = 2;
        break;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 5;
        UIstate_buf = 2;
        break;
      }
    }
    /*****************************************************************/
    delay(2000); 

    break;
  }

/***********************************************************************************************************************************************************
END OF SECTION: CALLING MAIN PAGE (2)                               COMPLETED
UP NEXT: DIALLER (21)
************************************************************************************************************************************************************/
//                                                                [DIALLER PAGE]   

  case 21: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    Serial.println("Type phone number to call.");
    char* phone_no = ("+917775829359");                                   //char* reserves next 12 characters in memory address.

    phone_no = Serial.read();                                             //Phone number entered from Serial Monitor read and stored.
    
    if (digitalRead(pushYes)){
      Serial.println("Calling " + String(phone_no));                      //If dialled and Yes button is pushed, make the call to phone_no
      gsm.call(phone_no);
      delay(5000);                                                        //delay for call to connect
      if (gsm.call(phone_no)){
        Serial.println("Connected.");
        logNumber = logNumber + 1;                                        //Logs updated with two separate variable, an int and a char array
        logs[logNumber] = phone_no;
        while(1){                                                          //Infinite loop will break only if call is cancelled
          if (digitalRead(pushNo)){                                        // If No button is pushed, hang up the call
            Serial.println("Hanging up.");
            gsm.callHangoff();
            break; 
          }                                                               
        }
      }
      else{
        Serial.println("Not connected.");                                 //Status reported as connected if connected and otherwise
      }
      
      
        }
    
   if (digitalRead(pushNo)) {                                     //If 'No' button is pushed, UI returns to 'Calling'
      UIstate = 2;
      UIstate_buf = 21;
      digitalWrite(ledMatrix, LOW);                                     //Backlight LED matrix switched off
      break;
    }
    
/*****************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                  // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 21;
        UIstate_buf = 21;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //For first two cases, keep at dialling
        UIstate = 21;
        UIstate_buf = 21;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {      //Switch to contacts
        UIstate = 22;
        UIstate_buf = 21;
        digitalWrite(ledMatrix, LOW);                                     //Switch off backlight
        break;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {      //Switch to Logs
        UIstate = 23;
        UIstate_buf = 21;
        digitalWrite(ledMatrix, LOW);
        break;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 23;
        UIstate_buf = 21;
        digitalWrite(ledMatrix, LOW);
        break;
      }
    }
    /*****************************************************************/
    delay(2000); 

    break;
  }

/***********************************************************************************************************************************************************************/
/***********************************************************************************************************************************************************
END OF SECTION: DIALLER (21)                                         COMPLETED
UP NEXT: CONTACTS (22)
************************************************************************************************************************************************************/
//                                                                [CONTACTS PAGE]  
  
  case 22: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    Serial.println("Type in index number to call saved contact.");        //Printing Contact List
    Serial.println("1. Advait");
    Serial.println("2. Aai");
    Serial.println("3. Baba");
    Serial.println("4. Aneesh");
    Serial.println("5. Roaming Cell");

    delay(5000);                                                          //delay to make decision

/***********************************************************************************************/  
    
      int selectedNo = Serial.read();
      char* phone_no = ("+917775829359");
      
      if (selectedNo == 1)
         phone_no = ("+917775829359");                                   //char* reserves next 12 characters in memory address.
      else if (selectedNo == 2)
         phone_no = ("+919011072091");                                   // Phone number allocated according to number stored in memory
      else if (selectedNo == 2)
         phone_no = ("+919011089105"); 
      else if (selectedNo == 2)
         phone_no = ("+919422084612");
      else if (selectedNo == 2)
         phone_no = ("+919422322091");
      else
         phone_no = ("+917775829359");                               //Default number to call

      delay(500);

      Serial.println("Calling " + String(phone_no));                      //If dialled and Yes button is pushed, make the call to phone_no
      gsm.call(phone_no);
      delay(5000);                                                        //delay for call to connect
      if (gsm.call(phone_no)){
        Serial.println("Connected.");
        while(1){                                                          //Infinite loop will break only if call is cancelled
          if (digitalRead(pushNo)){                                        // If No button is pushed, hang up the call
            Serial.println("Hanging up.");
            gsm.callHangoff();
            break; 
          }                                                               
        }
      }
      else{
        Serial.println("Not connected.");                                 //Status reported as connected if connected and otherwise
      }
      
    
    if (digitalRead(pushNo)) {                                     //If 'No' button is pushed, UI returns to 'Calling'
      UIstate = 2;
      UIstate_buf = 22;
      break;
    }


    
/*****************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                  // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 21;
        UIstate_buf = 22;
        digitalWrite(ledMatrix, HIGH);
        break;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch to Dialler
        UIstate = 21;
        UIstate_buf = 22;
        digitalWrite(ledMatrix, HIGH);
        break;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {      //Remain at contacts
        UIstate = 22;
        UIstate_buf = 22;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {      //Switch to Logs
        UIstate = 23;
        UIstate_buf = 21;
        break;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 23;
        UIstate_buf = 22;
        digitalWrite(ledMatrix, LOW);
        break;
      }
    }
    /*****************************************************************/
   
    delay(2000); 
    break;
    
  }



/***********************************************************************************************************************************************************************/
/***********************************************************************************************************************************************************
END OF SECTION: CONTACTS (22)                                      COMPLETED
UP NEXT: LOGS (23)
************************************************************************************************************************************************************/
//                                                                [LOGS PAGE]  

  case 23: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    Serial.println ("LOGS");
    Serial.println ("Select index number to call.");
    int i;
    for (i = 0; i < 11; i++) {                                            //Prints out last 10 saved logs
      Serial.println(i + ". " + logs[i]);                                 // '1. log[1]'
    }
    int selectedOption = Serial.read();

/*************************************************************************************/

    if (selectedOption == 1){
      Serial.println("Calling " + String(logs[1]));                      //If dialled and Yes button is pushed, make the call to phone_no
      gsm.call(logs[1]);
      delay(5000);                                                        //delay for call to connect
      if (gsm.call(logs[1])){
        Serial.println("Connected.");
        while(1){                                                          //Infinite loop will break only if call is cancelled
          if (digitalRead(pushNo)){                                        // If No button is pushed, hang up the call
            Serial.println("Hanging up.");
            gsm.callHangoff();
            break; 
          }                                                               
        }
      }
      else{
        Serial.println("Not connected.");                                 //Status reported as connected if connected and otherwise
      }
    }

/*********************************************************************************************************/

     if (selectedOption == 2){
      Serial.println("Calling " + String(logs[2]));                      //If dialled and Yes button is pushed, make the call to phone_no
      gsm.call(logs[2]);
      delay(5000);                                                        //delay for call to connect
      if (gsm.call(logs[2])){
        Serial.println("Connected.");
        while(1){                                                          //Infinite loop will break only if call is cancelled
          if (digitalRead(pushNo)){                                        // If No button is pushed, hang up the call
            Serial.println("Hanging up.");
            gsm.callHangoff();
            break; 
          }                                                               
        }
      }
      else{
        Serial.println("Not connected.");                                 //Status reported as connected if connected and otherwise
      }
    }
       
/*****************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 21;
        UIstate_buf = 23;
        break;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 21;
        UIstate_buf = 23;
        break;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 22;
        UIstate_buf = 23;
        break;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {
        UIstate = 23;
        UIstate_buf = 23;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 23;
        UIstate_buf = 23;
      }
    }
    /*****************************************************************/

    if (digitalRead(pushNo)){                                           //block of code to go back to 'CALLING' (2)
      UIstate = 2;
      UIstate_buf = 23;
      break;
    }

    
    delay(2000); 
    break;
  }


/***********************************************************************************************************************************************************************/
/***********************************************************************************************************************************************************
END OF SECTION: LOGS (23)                                               COMPLETED
UP NEXT: MESSAGING (3)
************************************************************************************************************************************************************/
//                                                                [MESSAGING MAIN PAGE]  

  case 3: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    Serial.println("MESSAGING");

    if (digitalRead(pushYes)) {
      UIstate = 31;                                                      //If "CALLING' is selected, UI shifts to dialler (21)
      UIstate_buf = 3;
      digitalWrite(ledMatrix, HIGH);                                     //matrix of leds under keypad lights up to make keypad visible and hence easy to type with.
      delay(500);
      break;                                                             // break case and go to dialler (21)
    }

    if (digitalRead(pushNo)){
      UIstate = 1;
      UIstate_buf = 3;
      break;
    }

    
/*****************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 1;
        UIstate_buf = 1;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 2;
        UIstate_buf = 1;
        break;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 3;
        UIstate_buf = 1;
        break;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {
        UIstate = 4;
        UIstate_buf = 1;
        break;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 5;
        UIstate_buf = 1;
        break;
      }
    }
    /*****************************************************************/
    delay(2000); 
    break;
  }


/***********************************************************************************************************************************************************************/
/***********************************************************************************************************************************************************
END OF SECTION: MESSAGING MAIN PAGE (3)                               COMPLETED
UP NEXT: TYPE MESSAGE PAGE (31)
************************************************************************************************************************************************************/
//                                                                [TYPE MESSAGE PAGE]  

  case 31: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    gsm.smsTextMode(true);                                                // TEXT or PDU mode. TEXT is readable.

    char* number = "+905123456789";
    char* message = "Hello there! This is you, but from the future. Ewan McGregor passed away last night. You have no purpose left on this planet. Hindsight is harsh."; // message lenght must be <= 160. Only english characters.
    
    /* message length must be <= 160. Only english characters. */

    if (digitalRead(pushNo)) {
      UIstate = 3;
      UIstate_buf = 31;
      break;
    }
/********************************************************************/
//           [MESSAGE INPUT BLOCK]

    Serial.println ("Enter number of receiver.");
    
    if (digitalRead(pushNo)) {
      UIstate = 3;
      UIstate_buf = 31;
      break;
    }
    
    delay(5000);
    number = Serial.read();
    delay(1000);
    if (digitalRead(pushYes)) {
        Serial.println ("Type in the message.");
        if (digitalRead(pushNo)) {
         UIstate = 3;
         UIstate_buf = 31;
         break;
        }
       delay(10000);
       message = Serial.read();
       delay(1000);
     }
    
    
  

/*****************************************************************/    
//           [MESSAGE SEND FUNCTION BLOCK]

    if(digitalRead(pushYes)){
       int retryNo = 0;                                                       //After 5 retries, kill sending message command
       while(1){                                                              //Infinite loop breaks only if message is sent.
        bool statusSMS = (gsm.smsSend(number, message));                     // if success it returns true (1) else false (0)
        Serial.println("Sending.");
        delay(2000);
        if (statusSMS == true){
          Serial.println ("Sent");
          break;                                                             //breaks only if sent
        }
        else{
          Serial.println ("Retrying.");                                     //If failed, try again
          retryNo = retryNo +1;
        }
        if (retryNo >= 5) {
          Serial.println("Sorry, we're having network connectivity issues");
          break;
        }
       } 
    }

    else if (digitalRead(pushNo)){
      UIstate = 3;
      UIstate_buf = 32;
      break;
    }

    
/*********************************************************************/
    
/*****************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 31;
        UIstate_buf = 31;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 31;
        UIstate_buf = 31;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 31;
        UIstate_buf = 31;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {
        UIstate = 32;
        UIstate_buf = 31;
        break;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 33;
        UIstate_buf = 31;
        break;
      }
    }
    /*****************************************************************/
    
    if (digitalRead(pushNo)) {
      UIstate = 3;
      UIstate_buf = 31;
      break;
    }
    
    delay(2000); 
    break;
  }
  
/***********************************************************************************************************************************************************************/
/***********************************************************************************************************************************************************
END OF SECTION: TYPE MESSAGE PAGE (31)                     ERROR WHEN TRYING TO COMPILE - REBUILD
UP NEXT: MESSAGING CONTACTS (32)
************************************************************************************************************************************************************/
//                                                                [MESSAGING CONTACTS PAGE]  

  case 32: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    Serial.println("Type in index number to message saved contact.");        //Printing Contact List
    Serial.println("1. Advait");
    Serial.println("2. Aai");
    Serial.println("3. Baba");
    Serial.println("4. Aneesh");
    Serial.println("5. Roaming Cell");

/*********************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 31;
        UIstate_buf = 32;
        break;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 31;
        UIstate_buf = 32;
        break;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 31;
        UIstate_buf = 32;
        break;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {
        UIstate = 32;
        UIstate_buf = 32;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 33;
        UIstate_buf = 32;
        break;
      }
    }
    /*****************************************************************/
    delay(2000); 
    break;
  }


/***********************************************************************************************************************************************************************/
/***********************************************************************************************************************************************************
END OF SECTION: CHOOSE RECEIVER (32)                                  COMPLETED
UP NEXT: READ MESSAGES (33)
************************************************************************************************************************************************************/
//                                                                [READ MESSAGES PAGE]  

  case 33: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    gsm.smsTextMode(true);                                                // TEXT or PDU mode. TEXT is readable :)
    String unreadSMS = "0";                                                    //integer value variable for number of unread messages

    Serial.println("Unread messages: ");
    Serial.print(gsm.smsListUnread());                                   // if not unread messages have it returns "NO_SMS"
    unreadSMS = gsm.smsListUnread();
    delay(2000);

    Serial.println("Read?");
    delay(2000);

    if (digitalRead(pushYes)) {
      Serial.println("First Unread Message: ");
      Serial.println(gsm.smsRead(1));                                   // if no message in that index, it returns IXDEX_NO_ERROR
    }
    else if (digitalRead(pushNo)){
      UIstate = 3;
      UIstate_buf = 33;
      break;
    }

/*****************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 1;
        UIstate_buf = 1;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 2;
        UIstate_buf = 1;
        break;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 3;
        UIstate_buf = 1;
        break;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {
        UIstate = 4;
        UIstate_buf = 1;
        break;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 5;
        UIstate_buf = 1;
        break;
      }
    }
    /*****************************************************************/
    delay(2000); 
    break;
  }



/***********************************************************************************************************************************************************************/
/***********************************************************************************************************************************************************
END OF SECTION: READ MESSAGES (33)                                    COMPLETED
UP NEXT: ALARMS MAIN PAGE [4]
************************************************************************************************************************************************************/
//                                                                [ALARMS MAIN PAGE]  

  case 4: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    Serial.println("ALARMS");

     if (digitalRead(pushYes)){
      UIstate = 41;
      UIstate_buf = 4;
      break;
     }

     if (digitalRead(pushNo)){
      UIstate = 1;
      UIstate_buf = 4;
      break;
     }


/*****************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 1;
        UIstate_buf = 4;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 2;
        UIstate_buf = 4;
        break;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 3;
        UIstate_buf = 4;
        break;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {
        UIstate = 4;
        UIstate_buf = 4;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 5;
        UIstate_buf = 4;
        break;
      }
    }
    /*****************************************************************/
    delay(2000); 
    break;
  }  


/***********************************************************************************************************************************************************************/
/***********************************************************************************************************************************************************
END OF SECTION: ALARMS MAIN PAGE [4]                                  COMPLETED
UP NEXT: SET ALARMS PAGE (41) 
************************************************************************************************************************************************************/
//                                                                [SET ALARMS PAGE]  

  case 41: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    Serial.println("SET ALARM.");
    Serial.println("Hour:");
    delay(5000);
    alarmHour = Serial.read();
    if (digitalRead(pushYes)) {                                           //If hour is entered, and yes is pushed, go to entering minutes
      Serial.println("Minute:");
      delay(5000);
      alarmMin = Serial.read();

      if(digitalRead(pushYes)){                                           //Alarm saved notification bearing time saved at.
        Serial.println("Alarm saved at ");
        Serial.print(alarmHour);
        Serial.print(":");
        Serial.print(alarmMin);
        break;
      }
      else if (digitalRead(pushNo)){                                     //If at any point, No is pushed, break to UIstate 4 - "ALARMS."
        UIstate = 4;
        UIstate_buf = 41;
        break;
      }
    }
    else if (digitalRead(pushNo)){
      UIstate = 4;
      UIstate_buf = 41;
      break;
    }

    if (digitalRead(pushNo)) {
      UIstate = 4;
      UIstate_buf = 41;
      break;
    }


/*****************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 41;
        UIstate_buf = 41;
        break;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 41;
        UIstate_buf = 41;
        break;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 41;
        UIstate_buf = 41;
        break;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {
        UIstate = 41;
        UIstate_buf = 41;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 41;
        UIstate_buf = 41;
        break;
      }
    }
    /*****************************************************************/
    delay(2000); 
    break;
  } 

/***********************************************************************************************************************************************************************/
/***********************************************************************************************************************************************************
END OF SECTION: SET ALARMS PAGE (41)                    WRITE YOUR OWN CODE FOR THIS PAGE
UP NEXT: MUSIC (5)
************************************************************************************************************************************************************/
//                                                                [MUSIC PAGE]  

  case 5: {
    
    UIstate_read = analogRead(A0);                                        // Reading taken to run through test in instance of scroll
    Serial.println(UIstate);                                              // TEST LINE - DELETE FROM FINAL CODE

    Serial.println("MUSIC.");

/* This section of the code i.e. UIstate (5) Music is currently non-functional due to EEPROM memory shortage    
 *  Plan to be updated in the future.
 *  Because what use is a phone without music, right?
 *  Right?
 *  Okay.
 */

/*****************************************************************/
/*                      state change tests                       */        
    
    if (analogRead(A0) != UIstate_read) {                                // This means scroll ring has been turned
      if ((analogRead(A0) >= 0) && (analogRead(A0) <= 204)) {
        UIstate = 1;
        UIstate_buf = 5;
        break;
      }
      else if ((analogRead(A0) >= 205) && (analogRead(A0) <= 408)) {       //Switch UIstate to calling, messaging, etc.based on scroll ring's position
        UIstate = 2;
        UIstate_buf = 5;
        break;
      }
      else if ((analogRead(A0) >= 409) && (analogRead(A0) <= 612)) {
        UIstate = 3;
        UIstate_buf = 5;
        break;
      }
      else if ((analogRead(A0) >= 613) && (analogRead(A0) <= 816)) {
        UIstate = 4;
        UIstate_buf = 5;
      }
      else if ((analogRead(A0) >= 817) && (analogRead(A0) <= 1024)) {
        UIstate = 5;
        UIstate_buf = 5;
      }
    }
    /*****************************************************************/
    delay(2000); 
    break;
  }   
  
  }

}














               

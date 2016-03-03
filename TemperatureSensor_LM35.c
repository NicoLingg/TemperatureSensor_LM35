/*!
*\mainpage 
*\author: Nico Lingg und Andreas Steiner
*\version: 2.0
*\brief Temperatursensor LM35 mit RTOS
*\date 26.06.2013
*
* Projektaufgabe VI Temperatursensor LM 35, Time-Delay, Mailbox und LCD
*
*
*Bei diesem Projekt wird mit Hilfe eines Echtzeitbetriebsprogrammes (RTOS) der Analogwert eines 
*Temperatursensor LM 35 alle 50ms abgetastet, ueber 10 Messwerte gemittel und durch eine Mailbox an 
*einen weitere Task weitegegeben.(Task TMessung())
*
*Der Inhalt, der in der Mailbox steht, wird ausgelesen und auf ein LCD-Modul ausgegeben (Task TLCD())
*
*Im ersten Task (TCreateTasks()) werden Task TMessung() und Task TLCD() erzeugt und eine LED mit 10 Hz blinken lassen.
*
*<b>Fuer die Mainfunktion bitte hier klicken: main()</b>
* 
*/
/************************************************************************************************

									Includes

************************************************************************************************/

#include "includes.h"

/************************************************************************************************

										Defines

************************************************************************************************/

#define TASK_STK_SIZE	   512
#define TCreateTasks_PRIO	10
#define TMessung_PRIO		20
#define TLCD_PRIO			30 	


/************************************************************************************************

									Funktions Deklarationen

************************************************************************************************/

//Tasks
void TCreateTasks(void *pdata);
void TMessung(void *pdata);
void TLCD(void *pdata);

//Board und RTOS
void Board_Init(void);
void HeartBeat_TickInit (void);

//LCD-Display
void LCD_Init(void);
void LCD_Initialausgabe(void);

// Funktionen
void LCDPrint(char *buf);
void ADC_Init(void);
void LCD_Initialausgabe(void);
void LED_Leiste(uint16_t Temp);

/************************************************************************************************

									globale Variablen

************************************************************************************************/

OS_STK     TCreateTasksStk[TASK_STK_SIZE];			
OS_STK     TMessungStk[TASK_STK_SIZE];				
OS_STK     TLCDStk[TASK_STK_SIZE];				
OS_EVENT   *TxMbox;


/*!
 * \brief  Main
 *
 *         Die System-Initialisierungen werden ausgefuehrt (Board_Init(),HeartBeat_TickInit(),LCD_Init(),ADC_Init())
 *		   Ein Task (TCreateTasks()) wird erzeug, der alle anderen Task erstellt.
 *		   Das Betriebssystem wird gestartet (OSStart())
 *         
 */
int main(void)
{
	uint8_t err;					// Errorbit

	Board_Init();					// Initialize board
	LCD_Init();						// LCD intitialisieren
	ADC_Init();						// ADC Initialisieren
	LCD_Initialausgabe();				// Testausgabe
	HeartBeat_TickInit();			// Initialize heartbeat
									

	OSInit();
	

	//Create Task createn
	err = OSTaskCreate(TCreateTasks, 		
					(void *)0, 
					&TCreateTasksStk[TASK_STK_SIZE-1],
					TCreateTasks_PRIO);



	
	OSStart();						// Start multitasking, enable heartbeat

	return 0;						//should never been reached
}


/************************************************************************************************

										TASKS

************************************************************************************************/

/*!
 * \brief  1. Task: TCreateTasks
 *		   
 *		  Dieser Task erzeugt alle anderen Task und laesst anschliessend in
 *		  einer Endlosschleife die LED mit einer Frequenz von 10 Hz blinken.
 *		  Beim Auftreten eines Fehlers leuchtet die LED dauerhaft
 *		  
 *\param		pdata	Uebergabepointer
 *
 */
void TCreateTasks( void *pdata)
{
	uint8_t err;

	
	TxMbox = OSMboxCreate((void*) 0);	//Create Mailbox		


	//Create Task TMessung
	err = OSTaskCreate(TMessung, 		
					(void *)0, 
					&TMessungStk[TASK_STK_SIZE-1],
					TMessung_PRIO);


	//Create Task TLCD
	err = OSTaskCreate(TLCD, 		
					(void *)0, 
					&TLCDStk[TASK_STK_SIZE-1],
					TLCD_PRIO);			
    

    while(1)
	{	
		err = OSTimeDlyHMSM(0,0,0,50);
		PORTB ^= 0x10;											// LED toggeln mit 10 Hz
		if (err != OS_NO_ERR) PORTB |= 0x10;					// bei Fehler LED Dauerleuchten		
		
 	 }

}


/*!
 * \brief  2. Task: TMessung
 *		   
 *		   In diesem Task wird der Wert vom ADC ausgelesen.
 *		   Hier bei wird ein Mittelwert ueber 10 Werte ermittelt und alle 50 ms an die Mailbox fuer den Task TLCD() uebergeben.
 *		   Ausserdem wird eine LED-Leiste in Abhaengigkeit der Temperatur angestuert (LED_Leiste())
 *
 *		  
 *		  
 *		   
 *\param   pdata	Uebergabepointer
 *
 */
void TMessung( void *pdata)
{
	char s[80];
	int8_t channel;
	uint16_t Temp[2];
  	uint16_t Daten=0;
	uint8_t i;
	channel=0;  
		
	while(1)
		{
		Daten=0;			


		ADMUX=(ADMUX & ~(0x1F))|(channel & 0x1F);	//Kanal waehlen, ohne andere Bits zu beeinflussen -> hier Kanal 0
  		ADCSRA|=(1<<ADSC);            				//eine Wandlung "single conversion"
  		while (ADCSRA & (1<<ADSC));     				//auf Abschluss der Konvertierung warten
  		

		for(i=0; i<10; i++) 
			{
				// Eine Wandlung
				ADCSRA |= (1<<ADSC);
				// Auf Ergebnis warten...
				while(ADCSRA & (1<<ADSC));
				Daten += ADCW;
			}										
	
	
		Daten /= 10;								//ADC auslesen, runden und zurueckgeben
		Temp[0]=Daten*256/1023;						//ADC umrechnen in °C bei 2,56V Referenzspannnug
		Temp[1]=(Daten/102.3*256)-(Temp[0]*10);		//Nachkommastelle berechnen
	
	
		//LED-Leiste
		LED_Leiste(Temp[0]);
		

		//Erzeugen des Strings fuer die Temperatur
		sprintf(s,"%d,%d%cC",Temp[0],Temp[1],0xDF);


		//Senden des Strings in die Mailbox
		OSMboxPost(TxMbox, (void*) s); 


		//50ms bis nauchste Messung warten
		OSTimeDlyHMSM(0,0,0,50);
		}
}


/*!
 * \brief  3. Task: TLCD
 *		   
 *		   Dieser Task holt die Messwerte aus der Mailbox ab und gibt diese auf einem LCD-Display aus (alle 3 Sekunden).
 * 		   Dabei wartet der Task bis etwas in der Mailbox ist. 
 *		   Tritt ein Fehler auf wird am LCD-Display "Fehler bei der Mailbox" ausgegeben.
 *		  
 *		  
 *		   
 *\param		pdata	Uebergabepointer
 *
 */
void TLCD( void *pdata)
{
	pdata = pdata;				
	uint8_t err;
	char *Temp;


	while(1){

		//String aus Mailbox lesen
		Temp = (char *)OSMboxPend(TxMbox, 0, &err);

		
	
		//Ausgabe auf Display (Temperatur oder Error)
		if (err==OS_NO_ERR)
			{
				LCDPrint(Temp);
			}
		else
			{	printf("Fehler bei der Mailbox"); //Fehler
			
			}	

		// 3 Sekunden bis nauchste Ausgabe warten
		OSTimeDlyHMSM(0,0,3,0);
	}
}


/************************************************************************************************

									Funktionen

************************************************************************************************/


/*!
\func
\param 
\return

\brief		Funktion zum Initialisieren der Platine 	

*/
void Board_Init(void)
{

 	MCUSR=0;					// den Watchdog abschalten
 	wdt_disable();	
    
		
  	MCUCR=0x80;					// JTAG abschalten braucht man wegen USB ja nicht, sicherer gegen Fehler	
  	MCUCR=0x80;		
  	

  	CLKPR=0x80;					// Den Taktteiler auf 1:1 setzen (16 Mhz CPU-Takt)
  	CLKPR=0x00;
 

  	DDRD = 0xff;				// PORT D: alles Ausgaenge fuer LED-Leiste
  	PORTD = 0xff;
  	

	DDRF =0x00;					// PORT F: alle auf Eingang ohne Pullup für ADC 
	PORTF =0x00;

  	DDRB  =  0x10;				// PORT B: LCD und LED Ausgaenge
  	PORTB =  0x10;	
	  

	// init LCD port directions and deselect chip
	DDRA=0x00;
	PORTE &= ~(1<<1);
	DDRE |= 0x0b;
  
		
}



/*!
 * \brief Initialausgabe des LCDs
 *
 *         Funktion zum Testen des LCD-Display, es wird ein ein Initialtext ausgegeben
 *         
 */
void LCD_Initialausgabe(void)
{
	lcd_clear_screen();
	printf("******  Temperatursensor  ******"); 
	printf("\n\r  Lingg u. Steiner - SoSe 2013"); 
	_delay_ms(3000);  // Anzeigedaur in Sekunden
	lcd_clear_screen();
}


/*!
 * \brief Ausgabe der LED-Leiste
 *
 *         Funktion zur Ausgabe auf die LED-Leiste
 *         
 */
void LED_Leiste(uint16_t Temp)
{
   		if (Temp<=5)			 PORTD = 0b00000001;
  	  	if (Temp<=10 && Temp>5)  PORTD = 0b00000011;
    	if (Temp<=15 && Temp>10) PORTD = 0b00000111;
    	if (Temp<=20 && Temp>15) PORTD = 0b00001111;
    	if (Temp<=25 && Temp>20) PORTD = 0b00011111;
    	if (Temp<=30 && Temp>25) PORTD = 0b00111111;
    	if (Temp<=35 && Temp>30) PORTD = 0b01111111;
    	if (            Temp>35) PORTD = 0b11111111;

}

/*!
 * \brief Funktion zur Ausgabe von Text an das LCD
 *
 *         Diese Funktion gibt den an die Funktion uebergebenen Text aus
 *         
 */
void LCDPrint(char *buf)
{

	lcd_set_cursor(0,0);
	printf("Temperatur:");
	lcd_set_cursor(0,2);
	printf("%s                 ", buf);

}




/*!
 * \brief Initialisierung ADC
 *
 *         Funktion fuer die Initialisierung des Analog-Digital Converters (ADC)
 *         
 */

void ADC_Init(void)
{
  	//Analogeingaunge initialisieren
	ADMUX=(1<<REFS1)|(1<<REFS0);						// Interne Referenzspannung 2,56V verwenden
	ADCSRA=(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);;	// Teilungsfaktor auf 128 stellen

	//Dummy-Readout, damit der erste Wert verworfen wird
  	uint16_t Datenmuell;
	ADCSRA |= (1<<ADSC);                  				// eine ADC-Wandlung 
  	while (ADCSRA & (1<<ADSC) );          				// auf Abschluss der Konvertierung warten
 	Datenmuell=ADCW;
	
}



/*!
 * \brief Initialisierung des LCDs
 *
 *         Funktion fuer die Initialisierung des LCD-Display
 *         
 */
void LCD_Init(void)
{
	// LCD Schnittstelle vorbereiten
	lcd_init_interface();
	// Die Standardausgabe (stdout) auf das LCD umleiten
	// Damit kann man dann mit printf() arbeiten
	stdout=&lcd_stream;
	// Den Controller initialisieren, hier fuer Textausgabe mit dem
	// internen Zeichensatz
	lcd_set_system(LCD_FONT_INTERNAL, 6, 8);
	// Das waure der externe europauische Zeichensatz (8x16),
	// dann hat man aber nur 2 Zeilen mit 24 Zeichen
	// lcd_set_system(LCD_FONT_EXTERNAL, 8, 16);
	// Den Cursor einstellen (falls man den benutzt)
	lcd_set_cursor_mode(LCD_CURSOR_OFF,0);
	// Die Anzeige louschen und das Anzeigefenster auf Adresse 0 zurueckstellen
	lcd_clear_screen();
	// Als letztes das Display einschalten
	lcd_control(LCD_ON);
}




/************************************************************************************************

										TIMER

************************************************************************************************/

 /*!
 * \brief Herzschlag Initialisierung
 *
 *         In dieser Routine wird die Taktrate fuer den Heartbeat eingestellt bzw. der Timer initialisiert
 */
void  HeartBeat_TickInit (void)
{	 
    TCCR0A        = 0x02;	// Set TIMER0 prescaler to CTC Mode, CLK/1024 	
    TCCR0B        = 0x05;	// Set CLK/1024 Prescale	  				                      
    TCNT0         =    0;	// Start TCNT at 0 for a new cycle				           
    OCR0A         = (CPU_CLOCK_HZ / OS_TICKS_PER_SEC / 1024 / 2) - 1;	
    TIFR0        |= 0x02;	// Clear  TIMER0 compare Interrupt Flag						        
    TIMSK0       |= 0x02;	// Enable TIMER0 compare Interrupt				             
}






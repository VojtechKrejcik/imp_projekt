#include "MKL05Z4.h"


/* Activation of particular LED display (DS1 - DS4) */
#define D1 0x0700
#define D2 0x0B00
#define D3 0x0D00
#define D4 0x0E00


/* Encoding of digits as active segments on specific LED display (DS1 - DS4) */
#define N0 0x0707
#define N1 0x0006
#define N2 0x0B03
#define N3 0x0907
#define N4 0x0C06
#define N5 0x0D05
#define N6 0x0F05
#define N7 0x0007
#define N8 0x0F07
#define N9 0x0D07


/* Bit-level masks that help to enable/disable DP segment on LED display */
#define MASK_DOT_ON 0x0008
#define MASK_DOT_OFF 0xFFF7


#define PB4_ISF_MASK 0x10


int minutes = 0;
int hours = 0;
int lower_minutes = 0;
int higher_minutes = 0;
int lower_hours = 0;
int higher_hours = 0;
int state = 0;
//3 mozne stavy, ktere se meni v prerusenich pri podrzeni tlacitka
enum states{DEFAULT, MINUTES, HOURS};
/* Just an ordinary delay loop */
void delay(long long bound) {
  long long i;
  for(i=0;i<bound;i++);
}


/* Let's turn off individual segments on the whole display */
void off() {

  PTB->PDOR = GPIO_PDOR_PDO(0x0000);
  PTA->PDOR = GPIO_PDOR_PDO(D1);
  PTA->PDOR = GPIO_PDOR_PDO(D2);
  PTA->PDOR = GPIO_PDOR_PDO(D3);
  PTA->PDOR = GPIO_PDOR_PDO(D4);

}


/* Basic initialization of GPIO features on PORTA and PORTB */
void ports_init (void)
{
  SIM->COPC = SIM_COPC_COPT(0x00);							   // Just disable the usage of WatchDog feature
  SIM->SCGC5 = (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK);  // Turn on clocks for PORTA and PORTB
  SIM->SCGC6 = SIM_SCGC6_RTC_MASK;					// set for rtc

  /* Set corresponding PORTA pins for GPIO functionality */
  PORTA->PCR[8] = ( 0|PORT_PCR_MUX(0x01) );  // display DS4
  PORTA->PCR[9] = ( 0|PORT_PCR_MUX(0x01) );  // display DS3
  PORTA->PCR[10] = ( 0|PORT_PCR_MUX(0x01) ); // display DS2
  PORTA->PCR[11] = ( 0|PORT_PCR_MUX(0x01) ); // display DS1

  /* Set corresponding PORTA port pins as outputs */
  PTA->PDDR = GPIO_PDDR_PDD( 0x0F00 );  // "1" configures given pin as an output

  NVIC_DisableIRQ(31);  // Disable the eventual generation of the interrupt caused by the control button

  /* Set corresponding PORTB pins for GPIO functionality */
  PORTB->PCR[0] = ( 0|PORT_PCR_MUX(0x01) );   // seg A
  PORTB->PCR[1] = ( 0|PORT_PCR_MUX(0x01) );   // seg B
  PORTB->PCR[2] = ( 0|PORT_PCR_MUX(0x01) );   // seg C
  PORTB->PCR[3] = ( 0|PORT_PCR_MUX(0x01) );   // seg DP
  PORTB->PCR[8] = ( 0|PORT_PCR_MUX(0x01) );   // seg D
  PORTB->PCR[9] = ( 0|PORT_PCR_MUX(0x01) );   // seg E
  PORTB->PCR[10] = ( 0|PORT_PCR_MUX(0x01) );  // seg F
  PORTB->PCR[11] = ( 0|PORT_PCR_MUX(0x01) );  // seg G

  /* Set corresponding PORTB port pins as outputs */
  PTB->PDDR = GPIO_PDDR_PDD( 0x0F0F ); // "1" configures given pin as an input
  PORTB->PCR[4] = ( 0 | PORT_PCR_ISF(1) | PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01) |
					    PORT_PCR_PE(1) | PORT_PCR_PS(1)); // display SW1

  /* Let's clear any previously pending interrupt on PORTB and allow its subsequent generation */
 NVIC_ClearPendingIRQ(31);
 NVIC_EnableIRQ(31);
}
/* Single digit shown on a particular section of the display  */
void sn(int number, uint32_t display) {

  uint32_t n;

  switch (number) {
    case 0:
      n = N0; break;
    case 1:
      n = N1; break;
    case 2:
      n = N2; break;
    case 3:
      n = N3; break;
    case 4:
      n = N4; break;
    case 5:
      n = N5; break;
    case 6:
      n = N6; break;
    case 7:
      n = N7; break;
    case 8:
      n = N8; break;
    case 9:
      n = N9; break;
    default:
      n = N0;
  }

  if (display == D2)
    n |= MASK_DOT_ON;
  else
    n &= MASK_DOT_OFF;

  PTA->PDOR = GPIO_PDOR_PDO(display);
  PTB->PDOR = GPIO_PDOR_PDO(n);

  delay(3000);
  off();
}

//funkce zobrazi cas na display
void show_time(){

	PTA->PDOR = GPIO_PDOR_PDO(0x0000);
	PTB->PDOR = GPIO_PDOR_PDO(0x0000);

	//v pripade nastavovani minut, chci zobrazit pouze minuty (hodiny zustanou skryty a obracene)
	if(state == MINUTES || state == DEFAULT){
		sn(lower_minutes, D4);
		sn(higher_minutes, D3);
	}
	if(state == HOURS || state == DEFAULT){
		sn(lower_hours, D2);
		sn(higher_hours, D1);
	}
}
/* Service routine invoked upon the press of a control button */
void PORTB_IRQHandler( void )
{
    delay(100);

    if (PORTB->ISFR & GPIO_PDIR_PDI(PB4_ISF_MASK)) {
      if (!(PTB->PDIR & GPIO_PDIR_PDI(PB4_ISF_MASK))) {
         //vypne prerusenig generovane rtc, aby mohlo zpracovani tohoto preruseni probehnout bez problemu
        NVIC_DisableIRQ(RTC_IRQn);
        uint32_t time_pressed = RTC_TSR;
        int flag_longpress = 0;

        while(!(PTB->PDIR & GPIO_PDIR_PDI(PB4_ISF_MASK)))
        {
            if((RTC_TSR - time_pressed) > 1) //podrzeni tlacitka
            {
            	flag_longpress = 1; //-> vime, ze talcitko je drzeno a ne jen stisknuto
                if(state == DEFAULT){
                	state = MINUTES;
                }else if(state == MINUTES){
                	state = HOURS;
                }else{
                	state = DEFAULT;
                }
                while(!(PTB->PDIR & GPIO_PDIR_PDI(PB4_ISF_MASK)));
            }
        }
        //akce pro stisk tlacitka
        if(flag_longpress == 0){
        	if(state == MINUTES){
        		minutes+=1;
        		if(minutes == 60) minutes = 0;
        	}else if(state == HOURS){
        		hours+=1;
        		if(hours == 24) hours = 0;
        	}else{
        		for(int i = 0; i<200; i++){
        			show_time();
        		}
        	}
        }
        //zajisti, aby RTC_TSR nepredbehlo RTC_TAR
        RTC_TAR += (RTC_TSR - time_pressed);
        //znovu zapne rtc preruseni
        NVIC_ClearPendingIRQ(RTC_IRQn);
        NVIC_EnableIRQ(RTC_IRQn);
      }

      PORTB->PCR[4] |= PORT_PCR_ISF(0x01);  // Confirmation of interrupt after button press
    }

}





//preruseni vyvolane RTC kazdou minutu
void RTC_IRQHandler() {
    if(RTC_SR & RTC_SR_TAF_MASK) {

    	RTC_TAR += 60;
    	minutes += 1;
    	if( minutes >= 60){
    		hours++;
    		minutes = 0;
    	}
    	if(hours >= 24) hours = 0;
    	//vypocet cislic pro zobrazeni na displeji
    	lower_minutes = minutes%10;
    	higher_minutes = (minutes - lower_minutes)/10;

    	lower_hours = hours%10;
    	higher_hours = (hours - lower_hours)/10;


    }


}

void RTCInit() {

    RTC_CR |= RTC_CR_SWR_MASK;
    RTC_CR &= ~RTC_CR_SWR_MASK;

    RTC_TCR = 0x0000; // reset TCR

    RTC_CR |= RTC_CR_OSCE_MASK; // enable 32.768 kHz oscillator

    delay(0x600000);//potreba pro inicializaci rtc modulu

    RTC_SR &= ~RTC_SR_TCE_MASK;

    RTC_TSR = 0x00000000; // MIN value in 32bit register
    RTC_TAR = 60; // prvni preruseni po minute

    RTC_IER |= RTC_IER_TAIE_MASK;

    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_EnableIRQ(RTC_IRQn); //aktivace rtc preruseni

    RTC_SR |= RTC_SR_TCE_MASK;
}



int main(void)
{
	ports_init();
	RTCInit();

	for (;;) {
		if(state == MINUTES){
			show_time();
		}else if(state == HOURS){
			show_time();
		}
		//show_time();

	}

	return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

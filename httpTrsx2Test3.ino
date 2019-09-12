/*
 * En modo debugging....
 */
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "httpTrsx2.h"

void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);
unsigned char USART_Receive(void);
void USART_Flush(void);

#define FOSC F_CPU // Clock Speed
#define BAUD 38400//19200
#define MYUBRR ((16e6/(16.0*BAUD))-1)

void usart_print_string(const char *p);
void usart_print_PSTRstring(const char *p);
void usart_println_string(const char *p);
void usart_println_PSTRstring(const char *p);

void USART_Init(unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char) (ubrr >> 8);
	UBRR0L = (unsigned char) ubrr;
	//UBRR0H = 0;
	//UBRR0L = 3;
	/*Enable receiver and transmitter */
	UCSR0B = (0 << RXEN0) | (1 << TXEN0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (0 << USBS0) | (3 << UCSZ00);
	//UCSRC = (1<<URSEL)|(0<<USBS)|(3<<UCSZ0);
}
void USART_Transmit(unsigned char data)
{
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1 << UDRE0)))
		;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}
unsigned char USART_Receive(void)
{
	/* Wait for data to be received */
	while (!(UCSR0A & (1 << RXC0)))
		;
	/* Get and return received data from buffer */
	return UDR0;
}
//void USART_Flush( void )
//{
//    unsigned char dummy;
//    while ( UCSR0A & (1<<RXC0) ) dummy = UDR0;
//}

//ISR(USART_RX_vect)
//{
//    uint8_t _udr_rx = UDR0;
//    //uint8_t u = UDR;
//}
//////////////////////////////////////////////

void usart_print_string(const char *p)
{
	while (*p)
	{
		USART_Transmit(*p);
		p++;
	}
}

void usart_println_string(const char *p)
{
	usart_print_string(p);
	USART_Transmit('\n');
}

#if defined(__GNUC__) && defined(__AVR__)
    #include <avr/pgmspace.h>

    void usart_print_PSTRstring(const char *p)
    {
        char c;

        while (1)
        {
            c = pgm_read_byte_near(p++);
            if (c == '\0')
                break;
            else
                USART_Transmit(c);
            //p++;
        }
    }

    void usart_println_PSTRstring(const char *p)
    {
        usart_print_PSTRstring(p);
        USART_Transmit('\n');
    }

#endif

//#include <ArduinoJson.h>
//Arduino MEGA pinout
#define LED1 34 //PC3
#define LED2 35 //PC2
#define LED3 36 //PC1
#define LED4 37 //PC0
#define LED5 38 //PD7
#define LED6 39 //PG2

/*NIC setup*/
uint8_t IP[4] =
{ 192, 168, 1, 60 };	//fallback
uint8_t MAC[6] =
{ 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

/*Connect to server - setting*/
//char server[] = "api.quanticoservices.net";
//uint8_t IPaddr_server[] ={ 192, 168, 1, 48 };
IPAddress IPaddr_server(192, 168, 1, 48);

void UART_setup(void)
{
#if defined(__AVR__) && defined(__GNUC__)
		USART_Init(MYUBRR);
    #else
#endif
}
void UART_printChar(char c)
{
#if defined(__AVR__) && defined(__GNUC__)
		USART_Transmit( c);
	#else
#endif
}
#if defined(__AVR__) && defined(__GNUC__)
 	 
    void UART_print(char *str, int8_t mode)
    {
        if (mode == 0)
        {
        	usart_print_string(str);
        }
        else
        {
			#ifdef FS_STRING
        		usart_print_PSTRstring(str);
			#else
        		usart_print_string(str);
			#endif
        }
    }
    
    void UART_println(char *str, int8_t mode)
    {
    	UART_print(str, mode);
    	UART_printChar('\n');
    }
    
	#define UART_printStrk(s) do{UART_print(s, 1);}while(0)
	#define UART_printStr(s) do{UART_print(s, 0);}while(0)
    //
	#define UART_printlnStrk(s) do{UART_println(s, 1); }while(0)
	#define UART_printlnStr(s) do{UART_println(s, 0); }while(0)
#else
void UART_printStr(char *str)
{
}
#endif

#define TRSX_NUMMAX 2
TRSX trsx[TRSX_NUMMAX];
//
void spi_deselect_devices(void);	//only for QUANTICO BOARD
EthernetClient client;	//1 instance
//
void setup(void)
{
	digitalWrite(LED3, LOW);
	pinMode(LED3, OUTPUT);

	char buff[30];
	spi_deselect_devices();	//only for QUANTICO BOARD
	UART_setup();

#ifdef HTTPTRSX_DEBUG
    httpTrsx_UARTdebug_setPrintFx(UART_print);
    httpTrsx_UARTdebug_setPrintlnFx(UART_println);
    httpTrsx_UARTdebug_setPrintCharFx(UART_printChar);
    #endif    

	//1) local network setting
	NIC_begin(MAC, IP);	//by default DHCP
	NIC_getMyIP(buff, sizeof(buff));
	UART_printStrk(FS("My IP: "));
	UART_printlnStr(buff);
	delay(2000);

	//2) Set trsx[0] 
	httpTrsx_setClient(&trsx[0], (Client*) &client);//Only for Arduinochar strval[30];//client.setTimeout(1000);
	httpTrsx_setupServerByIP(&trsx[0], IPaddr_server, 80);
	httpTrsx_setURI(&trsx[0], "/jsondecode1.php");
	httpTrsx_setHost(&trsx[0], "192.168.1.54");

	//Puede ser con una Fx o a traves de HeaderLine libre... eso por definir...
	//httpTrsx_setApiKey(&trsx[0], "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6IjE1MzU0MjczNTVfcGFibG8iLCJkZXZpY2VfaWQiOiI1YjdmMjc3ZmVmNGFkNjgxYjIwM2I0NDQiLCJlbWFpbCI6InBhYmxvZG9uYXlyZUBnbWFpbC5jb20iLCJpYXQiOjE1NjQwODgwMjR9.G8BWFQ1O_KH4hVfibYSlGd-UqQLdWZ1d_sxonbhqANc");
	httpTrsx_setHdrLine(&trsx[0],
			"api_key_write: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6IjE1MzU0MjczNTVfcGFibG8iLCJkZXZpY2VfaWQiOiI1YjdmMjc3ZmVmNGFkNjgxYjIwM2I0NDQiLCJlbWFpbCI6InBhYmxvZG9uYXlyZUBnbWFpbC5jb20iLCJpYXQiOjE1NjQwODgwMjR9.G8BWFQ1O_KH4hVfibYSlGd-UqQLdWZ1d_sxonbhqANc");
	//
	httpTrsx_setExecInterval_ms(&trsx[0], 0);    //ms
	httpTrsx_setExecMode(&trsx[0], EM_RUN_ONCE);    //RUN_ONCE EM_RUN_INTERVAL
#ifdef HTTPTRSX_DEBUG
	httpTrsx_UARTdebug_enabled(&trsx[0], TRUE);//
	#endif

	//2) Set trsx[1] 
	httpTrsx_setClient(&trsx[1], (Client*) &client); //Only for Arduinochar strval[30];//client.setTimeout(1000);
	httpTrsx_setupServerByIP(&trsx[1], IPaddr_server, 80);
	httpTrsx_setURI(&trsx[1], "/jsondecode1.php");
	httpTrsx_setHost(&trsx[1], "192.168.1.54");
	//
	//httpTrsx_setApiKey(&trsx[1], "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6IjE1MzU0MjczNTVfcGFibG8iLCJkZXZpY2VfaWQiOiI1YjdmMjc3ZmVmNGFkNjgxYjIwM2I0NDQiLCJlbWFpbCI6InBhYmxvZG9uYXlyZUBnbWFpbC5jb20iLCJpYXQiOjE1NjQwODgwMjR9.G8BWFQ1O_KH4hVfibYSlGd-UqQLdWZ1d_sxonbhqANc");
	httpTrsx_setHdrLine(&trsx[1],
			"api_key_read: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6IjE1MzU0MjczNTVfcGFibG8iLCJkZXZpY2VfaWQiOiI1YjdmMjc3ZmVmNGFkNjgxYjIwM2I0NDQiLCJlbWFpbCI6InBhYmxvZG9uYXlyZUBnbWFpbC5jb20iLCJpYXQiOjE1NjQwODgwMjR9.G8BWFQ1O_KH4hVfibYSlGd-UqQLdWZ1d_sxonbhqANc");
	//
	httpTrsx_setExecInterval_ms(&trsx[1], 500);	//ms
	httpTrsx_setExecMode(&trsx[1], EM_RUN_INTERVAL);//RUN_ONCE EM_RUN_INTERVAL
#ifdef HTTPTRSX_DEBUG
	httpTrsx_UARTdebug_enabled(&trsx[1], TRUE);//TRUE
	#endif

}
////////////////////////////////////////////////////////////
#define NUM_IC 2
#define LTC_NUM_CELLS 6

float vcellF[NUM_IC][LTC_NUM_CELLS];

uint8_t ltc_connected[NUM_IC];
uint8_t stack_union[NUM_IC];
uint8_t select_2_12V[NUM_IC];

float stack_temperatureF[NUM_IC] =
{ 25.6, 28.15 };
float stack_voltage_ifcF[NUM_IC] =
{ 48.56, 12.78 };
float stack_currentF[NUM_IC] =
{ 256.59, 869.35 };

uint8_t uv_flag[NUM_IC][LTC_NUM_CELLS] =
{
{ 1, 0, 1, 0, 1, 0 },
{ 0, 1, 0, 1, 0, 1 } };
uint8_t ov_flag[NUM_IC][LTC_NUM_CELLS] =
{
{ 1, 0, 1, 0, 1, 0 },
{ 0, 1, 0, 1, 0, 1 } };
uint8_t openwire_flag[NUM_IC][LTC_NUM_CELLS];
uint8_t stack_overCurrent[NUM_IC];
uint8_t stack_overTemperature[NUM_IC];

float internal_digital_powerF[NUM_IC];//digital power supply, nominal:[2.7V-3.6V]
float internal_analog_powerF[NUM_IC];//analog power supply, nominal: [4.5V-5.5V]
float internal_die_tempF[NUM_IC];
float internal_socF[NUM_IC];	//SOC

//FACTOR CORRECTION based FLUKE 175
float FACTCORR_2V = +0.0042;	// + 0.0202 LTC genera una caida de tension
float FACTCORR_12V_C1 = +0.115;	//+0.073;
float FACTCORR_12V_C2 = +0.111;	//+0.064;
float FACTCORR_12V_C3 = +0.063;	//+0.047;
float FACTCORR_12V_C4 = +0.077;	//+0.067;

float ov_2v = 3.500;	// + FACTCORR_2V;//<= 5.73V
float uv_2v = 1.800;	// + FACTCORR_2V;//>= 0.0016V

//ov_12v_S3C / uv_12v_S3C Usados por software
float ov_12v_S3C = 13.500;	//S3C Sum of 3 Cell
float uv_12v_S3C = 10.0;	//1.950;

float shuntVoltFullScale = 0.05;
float shuntRatedCurrent = 1000;

//ov_12v / uv_12v  usados SOLO de manera formal para CONFIG REGISTER
//float FACTCORR_12V = (FACTCORR_12V_C1+FACTCORR_12V_C2+FACTCORR_12V_C3+FACTCORR_12V_C4)/4;
float ov_12v = 5.5;	// + FACTCORR_12V;//<= 5.73V
float uv_12v = 2.5;	// + FACTCORR_12V;//>= 0.0016V

static uint16_t stack_voltage(char *buff)
{
	strcpy(buff, "\"d13\":");
	json_cNumericArr(stack_voltage_ifcF, CNUM_FLOAT, NUM_IC,
			(buff + strlen(buff)));
	return strlen(buff);
}
static uint16_t stack_current(char *buff)
{
	strcpy(buff, "\"d14\":");
	json_cNumericArr(stack_currentF, CNUM_FLOAT, NUM_IC, (buff + strlen(buff)));
	return strlen(buff);
}
static uint16_t stack_temperature(char *buff)
{
	strcpy(buff, "\"d15\":");
	json_cNumericArr(stack_temperatureF, CNUM_FLOAT, NUM_IC,
			(buff + strlen(buff)));
	return strlen(buff);
}
static uint16_t allstacks_uvFlag(char *buff)
{
	strcpy(buff, "\"d10\":");
	json_cNumericArrBi(uv_flag, CNUM_UINT8, NUM_IC, LTC_NUM_CELLS,
			(buff + strlen(buff)));
	return strlen(buff);
}
static uint16_t allstacks_ovFlag(char *buff)
{
	strcpy(buff, "\"d11\":");
	json_cNumericArrBi(ov_flag, CNUM_UINT8, NUM_IC, LTC_NUM_CELLS,
			(buff + strlen(buff)));
	return strlen(buff);
}

struct _ethSend
{
	unsigned stack_voltage :1;
	unsigned stack_current :1;
	unsigned stack_temperature :1;
	unsigned allstacks_uvFlag :1;
	unsigned allstacks_ovFlag :1;
	unsigned __a :3;
} ethSend;

void enableFlagS(void)
{
	ethSend.stack_voltage = 1;
	ethSend.stack_current = 1;
	ethSend.stack_temperature = 1;
	ethSend.allstacks_uvFlag = 1;
	ethSend.allstacks_ovFlag = 1;
}

#define NUMMAX 5

PTRFX_retUINT16_T_arg1_PCHAR ethSend_pfx[NUMMAX] =
{ stack_voltage, stack_current, stack_temperature, allstacks_uvFlag,
		allstacks_ovFlag, };

int8_t httpTrsx_scheduler(char *jsonCstr);

void ethTrsx_job(void)
{
char buff[20];	
	//int8_t cod_ret = 0;
static uint8_t idx;
	static uint8_t bit;
	int8_t i, n;

	static uint8_t *pf = (uint8_t*) &ethSend;

	//added:
	#define JSONCSTR_MAXSIZE (400)
	char jsonCstr[JSONCSTR_MAXSIZE];
	char *pjsonCstr;
	uint16_t nbytes;
	//
	static int8_t sm0;
	int8_t found1;
	int8_t found0;
	//
	jsonCstr[0] = '\0';
	if (sm0 == 0)
	{
		//load-balance
		if (idx < 3)
			n = 1;
		else
			n = 2;

		//build the json
		pjsonCstr = &jsonCstr[0];
		nbytes = 0;
		found0 = 1;
		found1 = 0;

		UART_printStrk(FS("EMPEZANDO:"));
		for (i = 0; i < n; i++)
		{
			if (*pf & (1 << bit++))
			{
				
//++--
strcpy(buff,"\nidx: ");
json_cInteger(idx, &buff[strlen(buff)]);
UART_printlnStr(buff);
//--++				
				//1) clear flag
				//BitTo0(*pf, bit);

				if (found0 == 1)              //solo 1 vez
				{
					strcpy(pjsonCstr, "{");
					pjsonCstr++;
					found0 = 0;

					UART_printlnStrk(FS("{"));
				}
				//2) for the next if found at begin, then stay found = 1
				if (found1 == 1)
				{
					strcat(pjsonCstr, ",");
					pjsonCstr++;

					UART_printlnStrk(FS(","));
				}
				UART_printlnStrk(FS("Antes de convertir"));
				//
				nbytes = ethSend_pfx[idx](pjsonCstr);
				pjsonCstr += nbytes;
				UART_printlnStrk(FS("Despues de convertir"));
				//
				found1 = 1;
			}

			if (++idx >= NUMMAX)
			{
				idx = 0x00;
				pf = (uint8_t*) &ethSend;
				bit = 0;
				//cod_ret = 1;//termino de analizar todos los flags
				break;
			}
			else
			{
				//bit++;
				if (idx % 8 == 0)
				{
					pf++;
					bit = 0;
				}
			}
		}
		//
		if (found1 == 1)
		{
			strcat(pjsonCstr, "}");

UART_printlnStrk(FS("}"));
		}

UART_printlnStr(jsonCstr);
		sm0++;
	}

	if (sm0 == 1)
	{
		UART_printlnStr(jsonCstr);
		
		if (httpTrsx_scheduler(jsonCstr) == 1) //en el 1er envio el buffer[] se envia sin ningun problema
		{
			sm0 = 0x00;

			UART_printlnStrk(FS("FIN"));

			httpTrsx_setExecMode(&trsx[0], EM_RUN_ONCE);
			//delay(500);
		}
		//despues del primer envio, ya no interesa la direccion del buffer
	}

	//return cod_ret;
}

//////////////////////////////////////////////////////
int8_t httpTrsx_scheduler(char *jsonCstr)
{
	static int8_t sm0;
	int8_t cod_ret = 0;
	
	//++--
	//x reading from server
	char stream[15];
	int8_t cod=0;
	JSON json;
	//
	int8_t ii;
	static int8_t iilast;
	//--++

//  if (sm0 == 0)
//  {
//      if (httpTrsx_job(&trsx[0], 0, jsonCstr, strlen(jsonCstr), (void*)NULL, 0) == 1) 
//      {
//      //sm0++;
//    	  UART_printlnStrk(FS("httpTrsx_job END"));
//      cod_ret = 1;
//    }
//  }

	if (sm0 == 0)
	{
		cod_ret = httpTrsx_job(&trsx[0], 0, jsonCstr, strlen(jsonCstr), stream, sizeof(stream));
		if (cod_ret > 0)
		{
			do
			{
				json.name = (void*) NULL;
				cod = jsonDecode(stream, sizeof(stream), &json);
				if (json.name != NULL)
				{
					UART_printStrk(FS("\n"));
					if (strcmp(json.name, "m1") == 0)
					{
					}
					else if (strcmp(json.name, "ch2") == 0)
					{
					}
					else if (strcmp(json.name, "num") == 0)
					{
						double doub = strtod(json.strval, NULL);
						ii = (int8_t )doub;
						if (iilast != ii)
						{
							iilast = ii;
							UART_printStrk(FS("+++++++++++++++++++++++++++"));	
							if ( ii > 0)              //False
								digitalWrite(LED3, HIGH);
							else
								digitalWrite(LED3, LOW);	
						}
					}
				}
			} while (cod == 0);
			//
			if (cod_ret == 1)              //finish transmision
			{
				UART_printlnStrk(FS("httpTrsx_job END:1"));
				cod_ret = 1;
			}
		}
		else if (cod_ret == -1)
		{
			UART_printlnStrk(FS("httpTrsx_job END:-1"));
			cod_ret = 1;
		}
	}

	/*else if (sm0 == 1)
	 {
	 JSON jsonr[1]; //aqui seria conveniente si es null el json de entrada, entonces no envia nada
	 cod_ret = httpTrsx_job(&trsx[1], 1, &jsonr, sizeof(jsonr)/sizeof(JSON), stream, sizeof(stream));
	 if (cod_ret > 0)
	 {
	 do
	 {
	 json.name = (void*)NULL;
	 cod = jsonDecode(stream, sizeof(stream), &json);
	 if (json.name != NULL)
	 {
	 UART_printStrk(FS("\n"));
	 if (strcmp(json.name, "m1") == 0)
	 {
	 }
	 else if (strcmp(json.name, "ch2") == 0)
	 {
	 }
	 else if (strcmp(json.name, "num") == 0)
	 {
	 double doub= strtod(json.strval, NULL);
	 if ( ((int)doub) > 0)//False
	 {
	 //UART_printStrk(FS("LED TURN ON\n"));
	 digitalWrite(LED3,HIGH);
	 }
	 else
	 {
	 //UART_printStrk(FS("LED TURN OFF\n"));
	 digitalWrite(LED3,LOW);
	 }
	 }
	 }
	 }while (cod == 0);
	 //
	 if (cod_ret == 1)//finish transmision
	 {
	 sm0 = 0x00;
	 }
	 }
	 }
	 */
	return cod_ret;
}

////////////////////////////////////////////////////////////
uint8_t l;
void loop(void)
{
	//
	char buff[15];
	strcpy(buff, "\n<<");
	itoa(l, &buff[3], 10);
	strcat(buff, ">>\n");
	UART_printlnStr(buff);
	l++;
	//

	enableFlagS();
	ethTrsx_job();
}

void spi_deselect_devices(void)              //only for QUANTICO BOARD
{
#define WIZNET_CS 10
#define SDCARD_CS 4
#define LTC6804_CS 5
	pinMode(WIZNET_CS, OUTPUT);
	pinMode(SDCARD_CS, OUTPUT);
	pinMode(LTC6804_CS, OUTPUT);
	digitalWrite(WIZNET_CS, HIGH);
	digitalWrite(SDCARD_CS, HIGH);
	digitalWrite(LTC6804_CS, HIGH);
}

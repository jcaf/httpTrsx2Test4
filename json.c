/*
 * json.c
 *
 *  Created on: Aug 29, 2019
 *      Author: jcaf
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "json.h"

int8_t charIsNumber(char c)
{
	if ((c>='0') && (c<='9'))
		return 1;
	else
		return 0;
}

int8_t charIsLetter(char c)
{
	if ( ( (c>='A') && (c<='Z')) || ( (c>='a') && (c<='z') ) )
		return 1;
	else
		return 0;
}
inline void json_cInteger(int i, char *outbuff)
{
	#if defined(__AVR__) && defined(__GNUC__)
		  itoa( i, outbuff, 10 );
    #else
		sprintf(outbuff, "%i", i);
    #endif
}

inline void json_cFloat(float f, char *outbuff)
{
	#if defined(__AVR__) && defined(__GNUC__)
    	dtostrf(f, 0, 2, outbuff);
	#else
        sprintf(outbuff, "%.2f", f);
        //snprintf()
	#endif
}
//void json_cIntegerArr
void json_cFloatArr(float *v, int size, char *outbuff)
{
    char buff[20];

    int i;
    strcpy(outbuff,"[");

    for (i=0; i< size; i++)
    {
		#if defined(__AVR__) && defined(__GNUC__)
    	dtostrf(v[i], 0, 2, buff);
		#else
        snprintf(buff, sizeof(buff), "%.2f", v[i]);
		#endif

        strcat(outbuff, buff);//alternatevly use strlcat

        if (i < size-1)
            strcat(outbuff, ",");
    }
    strcat(outbuff,"]");
}

void json_cNumericArr(void* v, int8_t typeData, int size, char *outbuff)
{
    char buff[20];

    int i;
    strcpy(outbuff,"[");

    if (typeData == CNUM_FLOAT)
    {
        for (i=0; i< size; i++)
        {
            #if defined(__AVR__) && defined(__GNUC__)
              dtostrf(((float*)v)[i], 0, 2, buff);
            #else
                snprintf(buff, sizeof(buff), "%.2f", ((float*)v)[i]);
            #endif

            //
            strcat(outbuff, buff);//alternatevly use strlcat
            if (i < size-1)
                strcat(outbuff, ",");
        }
    }
    else if (typeData == CNUM_INT)
    {
        for (i=0; i< size; i++)
        {
            //_itoa( ((int*)v)[i], buff, 10 );
            snprintf(buff, sizeof(buff), "%i", ((int*)v)[i]);
            //
            strcat(outbuff, buff);//alternatevly use strlcat
            if (i < size-1)
                strcat(outbuff, ",");
        }

    }
    else if (typeData == CNUM_UINT8)
    {
        for (i=0; i< size; i++)
        {
            //_itoa( ((int*)v)[i], buff, 10 );
            snprintf(buff, sizeof(buff), "%i", ((uint8_t*)v)[i]);
            //
            strcat(outbuff, buff);//alternatevly use strlcat
            if (i < size-1)
                strcat(outbuff, ",");
        }

    }

    strcat(outbuff,"]");
}

void json_cNumericArrBi(void* v, int8_t typeData, int sizeX, int sizeY, char *buff)
{
    void *pv;
    int i;
  //
  strcpy(buff,"[");
  pv =v;
  for (i=0; i<sizeX; i++)
  {
    json_cNumericArr( pv, typeData, sizeY, (buff+strlen(buff)) );

        if (typeData == CNUM_FLOAT)
            pv = (float*)pv  +sizeY;
        else if (typeData == CNUM_INT)
            pv = (int*)pv  +sizeY;
        else if (typeData == CNUM_UINT8)
            pv = (uint8_t*)pv  +sizeY;

    //
        if (i < sizeX-1)
            strcat(buff, ",");
  }
  strcat(buff,"]");
}

int8_t jsonDecode(char *stream, size_t streamSize, JSON *json)
{
	static size_t i = 0;
	//int16_t
	size_t iname = 0;
	//int16_t
	size_t ivalue = 0;
	int8_t cod_ret = 0;

	int8_t sm0 = 0;
	int8_t sm1 = 0;

	while (1)
	{
		if (sm0 == 0)
		{
			if (stream[i] == '\"')
			{
				iname= i+1;//save initial position
				sm0++;
			}
		}
		else if (sm0 == 1)//cannot be " or any diferent to 'A-Z''a'b or"0.9"
		{
			if (stream[i] == '\"')//= 'A-Z') ) //letras y numeros
				sm0 = 0x00;
			else
				sm0++;
		}
		else if (sm0 == 2)
		{
			if (stream[i] == '\"')
			{
				sm0++;
			}
			else
            {
                if ( !charIsLetter(stream[i]) && !charIsNumber(stream[i]) )
                {
                    sm0 = 0x00;
                }
            }
		}
		else if (sm0 == 3)
		{
			if (stream[i] == ':')
			{
				stream[i-1] = '\0';
				sm0++;
			}
			else
			{
				sm0 =0;
			}
		}
		else if (sm0 == 4)
		{
			if (stream[i] == '\"')
			{
				ivalue = i+1;//suprim "
				sm0 = 5;//string
			}
			else if (charIsNumber(stream[i]))
			{
				ivalue = i;
				sm0 = 6; //number
			}
			else if (stream[i] == '[')
			{
				ivalue = i;
				sm0 = 7;
			}
			else
			{
				sm0 = 0x00;
			}
		}
		//string----------------------------
		else if (sm0 == 5)
		{
			if (sm1 == 0)
			{
				if (stream[i] == '\"')
				{
					sm1++;
				}
			}
			else if (sm1 == 1)
			{
				if ((stream[i] == ',') || (stream[i] == '}'))
				{
                    stream[i-1] = '\0';
                    sm0 = 8;
				}
			}
		}
		//number----------------------------
		else if (sm0 == 6)
		{
		    if ((stream[i] == ',') || (stream[i] == '}'))
            {
                stream[i] = '\0';
                sm0 = 8;
            }
            else
            {
                if ( (stream[i] != '.') && (!charIsNumber(stream[i])) )
                {
                    sm0 = 0;
                    sm1 = 0;
                }
            }
		}
		//vector----------------------------
		else if (sm0 == 7)
		{

		}
        //exit
        if (sm0 == 8)
        {
            json->name = &stream[iname];
            json->strval = &stream[ivalue];
            sm1 = 0;
            sm0 = 0;
            break;
        }
		//while end
		if (++i >= streamSize)
		{
			i = 0x00;
			sm0 = 0x0;
			sm1 = 0x0;
			cod_ret = 1;
			break;
		}
	}
	return cod_ret;
}

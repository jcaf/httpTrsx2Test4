/*
 * json.h
 *
 *  Created on: Aug 29, 2019
 *      Author: jcaf
 */

#ifndef JSON_H_
#define JSON_H_

	typedef struct _JSON
	{
		const char *name;
		const char *strval;
	}JSON;

	void json_cFloatArr(float *v, int size, char *outbuff);
	int8_t jsonDecode(char *stream, size_t streamSize, JSON *json);
  void json_cNumericArr(void* v, int8_t typeData, int size, char *outbuff);
  void json_cNumericArrBi(void* v, int8_t typeData, int sizeX, int sizeY, char *buff);
  
  
  enum
  {
  CNUM_FLOAT,
  CNUM_INT,
  CNUM_UINT8,
  };
  void json_cNumericArr(void* v, int8_t typeData, int size, char *outbuff);
  void json_cInteger(int i, char *outbuff);
  void json_cFloat(float f, char *outbuff);

#endif /* JSON_H_ */

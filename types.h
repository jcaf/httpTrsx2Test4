#ifndef TYPES_H_
#define TYPES_H_

typedef uint8_t(*PTRFX_retUINT8_T)(void);
typedef uint16_t(*PTRFX_retUINT16_T)(void);
typedef void (*PTRFX_retVOID)(void);
typedef void (*PTRFX_retVOID_arg1_UINT8_T)(uint8_t);
typedef void (*PTRFX_retVOID_arg1_INT8_T)(int8_t);
typedef void (*PTRFX_retVOID_arg1_PCHAR)(char *);
typedef void (*PTRFX_retVOID_arg1_CHAR)(char);
typedef void (*PTRFX_retVOID_arg1_PCHAR_arg2_INT8_T)(char *, int8_t);
typedef uint16_t (*PTRFX_retUINT16_T_arg1_PCHAR)(char *);
typedef unsigned char BOOL;

/******************************************************************************************/
typedef enum _BOOLEAN_E
{
    FALSE_E = 0,
    TRUE_E = 1,
}BOOLEAN_E;
typedef struct _BOOLEAN_T{BOOLEAN_E k;}BOOLEAN_T;
#define TRUE    ((BOOLEAN_T){TRUE_E})
#define FALSE   ((BOOLEAN_T){FALSE_E})
/******************************************************************************************/
typedef enum _ENABLED_E
{
    DISABLE_E = 0,
    ENABLE_E = 1,
}ENABLED_E;
typedef struct _ENABLED_T{ENABLED_E k;}ENABLED_T;
#define ENABLE   ((ENABLED_T){ENABLE_E})
#define DISABLE ((ENABLED_T){DISABLE_E})
/******************************************************************************************/


#endif // TYPES_H_

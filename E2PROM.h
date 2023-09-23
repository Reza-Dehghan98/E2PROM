/** In the Nama of God */
/**
 * @file E2PROM.h
 * @author Reza Dehghan (Rezzadehghgan98@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-09-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */



#ifndef _E2PROM_H_
#define _E2PROM_H_

#ifdef _cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "Queue.h"
#include "StreamBuffer.h"

/*************************************************Configuration***********************************************************/

/**
 * @brief E2PROM Default Value use for Erase the E2PROM Chip
 */
#define E2PROM_DEFAULT_VALUE            0XFF

/**
 * @brief Enable NoiseErase Capability
 */
#define E2PROM_NOISE_ERASE_NON_BLOCKING 1

/**
 * @brief if u want use Enable capability 
 */
#define E2PROM_CHECK_ENABLE             0


#define E2PROM_USE_INTERRUPT_I2C        0
/**
 * @brief 
 */
typedef uint32_t    E2PROM_Timestamp;


/**
 * @brief 
 */
typedef uint16_t    E2PROM_LenType;

/**************************************************************************************************/




/**
 * @brief E2PROM DataType
 */
typedef enum {
    E2PROM_Const    = 0x00,
    E2PROM_Variable = 0x01,
} E2PROM_DataType;



/**
 * @brief E2PROM Command Header
 */
typedef struct {
    uint32_t MemAddress;
    uint16_t Len;
    uint8_t  Mode;
    uint8_t  Type;
} E2PROM_CommandHeader;



/**
 * @brief E2PROM Mode
 */
typedef enum {
    E2PROM_ReadMode         = 0x00,
    E2PROM_WriteMode        = 0x01,
    E2PROM_EraseMode        = 0x02,
    E2PROM_NoiseEraseMode   = 0x03,
} E2PROM_Mode;



/**
 * @brief
 */
typedef enum {
    E2PROM_MemAddrSize8BIT  = (0x00000001U),
    E2PROM_MemAddrSize16BIT = (0x00000002U),
    E2PROM_MemAddrSize_Unknown,
} E2PROM_MemAddrSize;


/**
 * @brief config Struct
 */
typedef struct {
    void*              HI2C;
    E2PROM_Timestamp   WriteDelayTime;
    uint16_t           Size;
    uint8_t            DeviceId;///0xA0
    uint8_t            PageSize;
    E2PROM_MemAddrSize MemAddSize;
} E2PROM_Config;




/*PreDefined Structure*/
struct __E2PROM;
typedef struct __E2PROM E2PROM;



/**
 * @brief   Result of your Process 
 */
typedef enum {
    E2PROM_Ok               = 0,
    E2PROM_Error            = 1,
    E2PROM_Busy             = 2,
    E2PROM_TimeOutError     = 3, 
    E2PROM_HeaderValueError = 4,
    E2PROM_Null             = 5,
} E2PROM_Result;



/**
 * @brief Callback Function Pointer 
 */
typedef void (*E2PROM_CallbackFn)(Stream* stream, uint16_t addr, uint16_t len);




typedef union {
    E2PROM_CallbackFn callbacks[4];
    struct {
        E2PROM_CallbackFn onAfterWrite;
        E2PROM_CallbackFn onRead;
        E2PROM_CallbackFn onWriteError;
        E2PROM_CallbackFn onReadError;
    };
} E2PROM_Callbacks;




/**
 * @brief E2PROM  main Struct
 */
struct __E2PROM {
    struct __E2PROM*     Previous;
    const E2PROM_Config* Config;
    E2PROM_Callbacks     Callbacks;  
    Stream               WriteStream;
    Stream               ReadStream;
#if E2PROM_NOISE_ERASE_NON_BLOCKING
    Stream               NoiseEraseStream;
#endif
    Queue                CommandQueue;
    Queue                ReadQueue;
    E2PROM_CommandHeader CommandHeaderInProcess;
    void*                Args; /**< user arguments */
    E2PROM_Timestamp     NextTick;
    uint8_t*             ConstVal;
    uint8_t              TempLen;
    uint8_t              Lock           : 1;
    uint8_t              Enabled        : 1;
    uint8_t              Configured     : 1;
    uint8_t              EraseExecute   : 1;
    uint8_t              InBlocking     : 1;
    uint8_t              InTransmit     : 1;
    uint8_t              Reserved       : 2;
};

void E2PROM_onWrite(E2PROM* eeprom, E2PROM_CallbackFn cb);
void E2PROM_onRead (E2PROM* eeprom, E2PROM_CallbackFn cb);
void E2PROM_onWriteError(E2PROM* eeprom, E2PROM_CallbackFn cb);
void E2PROM_onReadError(E2PROM* eeprom, E2PROM_CallbackFn cb);
/*Function Pointer*/

typedef E2PROM_Result    (*E2PROM_writeFn)(E2PROM* eeprom, uint16_t address, uint8_t* val, uint16_t len);
typedef E2PROM_Result    (*E2PROM_readFn)(E2PROM* eeprom, uint16_t address, uint8_t* buffer, uint16_t len);
typedef E2PROM_Timestamp (*E2PROM_getTimestampFn)(void);
typedef void             (*E2PROM_delayMsFn)(E2PROM_Timestamp time);
typedef uint32_t         (*E2PROM_getRandomFn)(void);





/**
 * @brief E2PROM Driver Struct
 */
typedef struct {
    E2PROM_writeFn        write;
    E2PROM_readFn         read;
    E2PROM_getTimestampFn getTimestamp;
    E2PROM_delayMsFn      delayMs;
    E2PROM_getRandomFn    rand;
} E2PROM_Driver;

/* Null Define */
#define NULL_DRIVER (E2PROM_Driver*)0
#define E2PROM_NULL (E2PROM*)0
#define NULL        (void*)0

/******************************************************************************************************************/



void*         E2PROM_getArgs(E2PROM* eeprom);
void          E2PROM_init(E2PROM* eeprom, uint8_t* commandQBuffer, uint16_t commandQLen, uint8_t* qReadBuffer, uint16_t qReadLen, uint8_t* streamWriteBuffer, uint16_t streamWriteLen, uint8_t* streamReadBuffer, uint16_t streamReadLen);
void          E2PROM_driverInit(const E2PROM_Driver* driver);
void          E2PROM_deInit(void); 
uint8_t       E2PROM_handle(void);
void          E2PROM_setConfig(E2PROM* eeprom, const E2PROM_Config* config);
void          E2PROM_setEnabled(E2PROM* eeprom, uint8_t enabled);
void          E2PROM_readIRQ(E2PROM* eeprom);
void          E2PROM_writeIRQ(E2PROM* eeprom);
uint8_t       E2PROM_isEnabled(E2PROM* eeprom);
E2PROM_Result E2PROM_add(E2PROM* eeprom, const E2PROM_Config* config);
E2PROM_Result E2PROM_remove(E2PROM* remove);
E2PROM_Result E2PROM_waitForFinishProcess(E2PROM_Timestamp timeout);



/***************************************************** Erase E2PROM ************************************************************/
void          E2PROM_eraseBlocking(E2PROM* eeprom);
void          E2PROM_noiseEraseBlocking(E2PROM* eeprom);
void          E2PROM_erase(E2PROM* eeprom);

#if E2PROM_NOISE_ERASE_NON_BLOCKING
void          E2PROM_noiseEraseInit(E2PROM* eeprom, uint8_t* streamBuffer, uint8_t len);
void          E2PROM_noiseErase(E2PROM* eeprom);
#endif


/************************************************** Write/Read Blocking *********************************************************/
E2PROM_Result  E2PROM_writeBlocking(E2PROM* eeprom, uint16_t addr, void* data, uint16_t len);
E2PROM_Result  E2PROM_readBlocking(E2PROM* eeprom, uint16_t addr, uint8_t* val, uint16_t len);

E2PROM_Result  E2PROM_writeUInt8Blocking(E2PROM* eeprom, uint8_t val, uint16_t addr);
uint8_t        E2PROM_readUInt8Blocking(E2PROM* eeprom, uint16_t addr);

E2PROM_Result  E2PROM_writeUInt16Blocking(E2PROM* eeprom, uint16_t val, uint16_t addr);
uint16_t       E2PROM_readUInt16Blocking(E2PROM* eeprom, uint16_t addr);

E2PROM_Result  E2PROM_writeUInt32Blocking(E2PROM* eeprom, uint32_t val, uint16_t addr);
uint32_t       E2PROM_readUInt32Blocking(E2PROM* eeprom, uint16_t addr);

E2PROM_Result  E2PROM_writeUInt64Blocking(E2PROM* eeprom, uint64_t val, uint16_t addr);
uint64_t       E2PROM_readUInt64Blocking(E2PROM* eeprom, uint16_t addr);

/************************************************** Write/Read NonBlocking *********************************************************/
E2PROM_Result  E2PROM_write(E2PROM* eeprom, uint16_t addr, uint8_t* data, uint16_t len, E2PROM_DataType type);
E2PROM_Result  E2PROM_read(E2PROM* eeprom, uint16_t addr, uint8_t len);

E2PROM_Result  E2PROM_writeUInt8(E2PROM* eeprom, uint8_t val, uint16_t addr);
void           E2PROM_readUInt8(E2PROM* eeprom, uint16_t addr);

E2PROM_Result  E2PROM_writeUInt16(E2PROM* eeprom, uint16_t val, uint16_t addr);
void           E2PROM_readUInt16(E2PROM* eeprom, uint16_t addr);

E2PROM_Result  E2PROM_writeUInt32(E2PROM* eeprom, uint32_t val, uint16_t addr);
void           E2PROM_readUInt32(E2PROM* eeprom, uint16_t addr);

E2PROM_Result  E2PROM_writeUInt64(E2PROM* eeprom, uint64_t val, uint16_t addr);
void           E2PROM_readUInt64(E2PROM* eeprom, uint16_t addr);

/**********************************************************************************************************************************/
int8_t E2PROM_assertMemory (uint8_t* arr1, uint8_t* arr2, uint16_t len);

/*********************************************************************************************************************************/

#ifdef _cplusplus
};
#endif  // cplusplus

#endif  // _E2PROM_H_

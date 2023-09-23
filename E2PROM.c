#include "E2PROM.h"

const E2PROM_Driver* eepromDriver;
static E2PROM* lastE2PROM = E2PROM_NULL;



#define __eeprom()      lastE2PROM
#define __next(E2PROM)  E2PROM = (E2PROM)->Previous



/**
 * @brief this Array use for E2PROM Erase with 0xFF and u can change Value in the Configure  
 */
static const uint8_t E2PROM_PAGE[] = {
    E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE,
    E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE,
    E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE,
    E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE,
    E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE,
    E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE,
    E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE,
    E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE, E2PROM_DEFAULT_VALUE,
};





/**
 * @brief initial the E2PROM Driver
 * @param driver
 */
void E2PROM_driverInit(const E2PROM_Driver* driver) {
    eepromDriver = driver;
}



/**
 * @brief  DeInitial the E2PROM Driver
 *
 * @param driver
 */
void E2PROM_deInit(void) {
    eepromDriver = NULL_DRIVER;
}



/**
 * @brief Set E2PROM Config
 *
 * @param eeprom
 * @param config
 */
void E2PROM_setConfig(E2PROM* eeprom, const E2PROM_Config* config) {
    eeprom->Config = config;
}



/**
 * @brief Erase the E2PROM
 *
 * @param eeprom
 * @return E2PROM_Result
 */
void E2PROM_eraseBlocking(E2PROM* eeprom) {
    eeprom->Lock  = 1;
    uint16_t cnt  = eeprom->Config->Size;
    uint16_t addr = 0;
    while (cnt > 0) {
        eepromDriver->write(eeprom, addr, (void*)E2PROM_PAGE, eeprom->Config->PageSize);
        addr += eeprom->Config->PageSize;
        cnt  -= eeprom->Config->PageSize;
        eepromDriver->delayMs(eeprom->Config->WriteDelayTime);
    }
    eeprom->Lock = 0;
}



/**
 * @brief Erase the E2PROM with Noise Value
 *
 * @param eeprom  Address of your E2PROM
 * @return E2PROM_Result
 */
void E2PROM_noiseEraseBlocking(E2PROM* eeprom) {
    eeprom->Lock  = 1;
    uint16_t cnt  = eeprom->Config->Size / 4;
    uint16_t addr = 0;
    uint32_t temp;
    while (cnt > 0) {
        temp = eepromDriver->rand();
        eepromDriver->write(eeprom, addr, (uint8_t*)&temp, 4);
        addr += 4;
        cnt--;
        eepromDriver->delayMs(eeprom->Config->WriteDelayTime);
    }
    eeprom->Lock = 0;
}


/**
 * @brief initial E2PROM , and u must init your E2PROM before use it
 *
 * @param eeprom            Address of your E2PROM
 * @param commandQBuffer    Address of Buffer u need for initial the CommandQueue
 * @param commandQLen       Length of Buffer, u must Enter sizeof(commandQBuffer)
 * @param qReadBuffer       Address of Buffer u need for initial the ReadQueue
 * @param qReadLen          Length of Buffer, u must Enter sizeof(qReadBuffer)
 * @param streamWriteBuffer Address of Buffer u need for initial the WriteStream
 * @param streamWriteLen    Length of Buffer, u must Enter sizeof(streamWriteBuffer)
 * @param streamReadBuffer  Address of Buffer u need for initial the ReadStream
 * @param streamReadLen     Length of Buffer, u must Enter sizeof(streamReadBuffer)
 */
void E2PROM_init (E2PROM* eeprom, uint8_t* commandQBuffer, uint16_t commandQLen, uint8_t* qReadBuffer, uint16_t qReadLen, uint8_t* streamWriteBuffer, uint16_t streamWriteLen, uint8_t* streamReadBuffer, uint16_t streamReadLen) {
    eeprom->NextTick                          = 0;
    eeprom->Configured                        = 0;
    eeprom->Lock                              = 0;
    eeprom->CommandHeaderInProcess.Len        = 0;
    eeprom->CommandHeaderInProcess.MemAddress = 0;
    eeprom->CommandHeaderInProcess.Mode       = 0;
    eeprom->CommandHeaderInProcess.Type       = 0;
    eeprom->InTransmit                        = 0;
    Queue_init(&eeprom->CommandQueue, commandQBuffer, commandQLen, sizeof(E2PROM_CommandHeader));
    Queue_init(&eeprom->ReadQueue, qReadBuffer, qReadLen, sizeof(E2PROM_CommandHeader));
    Stream_init(&eeprom->WriteStream, streamWriteBuffer, streamWriteLen);
    Stream_init(&eeprom->ReadStream, streamReadBuffer, streamReadLen);
  
}

#if E2PROM_NOISE_ERASE_NON_BLOCKING
/**
 * @brief if u want to use NonBlocking NoiseErase u must use this function and after this u can use E2PROM_noiseErase
 *
 * @param eeprom Address of your E2PROM
 * @param streamBuffer address of buffer for init NoiseEraseStream
 * @param len Length of Buffer, sizeof(streamBuffer)
 */
void E2PROM_noiseEraseInit(E2PROM* eeprom, uint8_t* streamBuffer, uint8_t len) {
    Stream_init(&eeprom->NoiseEraseStream, streamBuffer, len);
}


/**
 * @brief Erase E2PROM with Random Number Generator unit of your MCU and before use this function u must do E2PROM_noiseEraseInit
 *
 * @param eeprom
 */
void E2PROM_noiseErase(E2PROM* eeprom) {
    E2PROM_CommandHeader    cacheHeader;
    cacheHeader.Len        = eeprom->Config->Size;
    cacheHeader.MemAddress = 0;
    cacheHeader.Mode       = E2PROM_NoiseEraseMode;
    cacheHeader.Type       = E2PROM_Variable;
    Queue_writeItem (&eeprom->CommandQueue, &cacheHeader);
}
#endif


/**
 * @brief E2PROM Handle , this function can use in your interrupt and while(1) for handle nonBlocking function
 *
 * @param eeprom Address of your E2PROM Struct
 */
uint8_t E2PROM_handle (void) {
    E2PROM*              pE2PROM  = lastE2PROM;
    uint16_t             len      = 0;
    uint8_t              overPage = 0;
    Stream               temp;
    E2PROM_CommandHeader header;
    uint8_t allProcessDone = 0;
    E2PROM_Result result;
    
#if E2PROM_CHECK_ENABLE
    if (pE2PROM->Enabled) {
#endif
        
            while (pE2PROM != E2PROM_NULL) {
              if (!pE2PROM->Lock) {
                if (Queue_available(&pE2PROM->CommandQueue) > 0 && pE2PROM->CommandHeaderInProcess.Len == 0) {
                    Queue_readItem(&pE2PROM->CommandQueue, &pE2PROM->CommandHeaderInProcess);
                    if (pE2PROM->CommandHeaderInProcess.Type == E2PROM_Const) {
                        switch (pE2PROM->CommandHeaderInProcess.Mode) {
                            case E2PROM_WriteMode:
                                Stream_readBytes(&pE2PROM->WriteStream, (uint8_t*)&pE2PROM->ConstVal, 4);
                                break;
                            case E2PROM_EraseMode:
                                pE2PROM->ConstVal = (uint8_t*)E2PROM_PAGE;
                                break;
                        }
                    }
                }

                if (Queue_available(&pE2PROM->ReadQueue) > 0) {
                    if (pE2PROM->Callbacks.onRead != NULL) {
                        Queue_readItem(&pE2PROM->ReadQueue, &header);
                        if (header.Len > 0 && header.MemAddress < pE2PROM->Config->Size) {
                          Stream_lockRead(&pE2PROM->ReadStream, &temp, header.Len);
                          pE2PROM->Callbacks.onRead(&temp, header.MemAddress, header.Len);
                          Stream_unlockRead(&pE2PROM->ReadStream, &temp);
                        }
                    }
                }

                if (pE2PROM->CommandHeaderInProcess.Len > 0 && pE2PROM->CommandHeaderInProcess.MemAddress <= pE2PROM->Config->Size) {
                    allProcessDone = 1;
                    switch (pE2PROM->CommandHeaderInProcess.Mode) {
                        case E2PROM_WriteMode:
                            if (pE2PROM->NextTick < eepromDriver->getTimestamp()) {
                                switch (pE2PROM->CommandHeaderInProcess.Type) {
                                    case E2PROM_Const:
                                        overPage         = (pE2PROM->CommandHeaderInProcess.Len > pE2PROM->Config->PageSize - (pE2PROM->CommandHeaderInProcess.MemAddress % pE2PROM->Config->PageSize)) ? 1 : 0;
                                        pE2PROM->TempLen = overPage ? pE2PROM->Config->PageSize - (pE2PROM->CommandHeaderInProcess.MemAddress % pE2PROM->Config->PageSize) : pE2PROM->CommandHeaderInProcess.Len;
                                        if (pE2PROM->InTransmit != 1) {
                                          pE2PROM->InTransmit = 1;
                                          result = eepromDriver->write(pE2PROM, pE2PROM->CommandHeaderInProcess.MemAddress, pE2PROM->ConstVal, pE2PROM->TempLen);
                                          if (result != E2PROM_Ok) {
                                            if (pE2PROM->Callbacks.onWriteError != NULL) {
                                              pE2PROM->Callbacks.onWriteError (&pE2PROM->WriteStream, pE2PROM->CommandHeaderInProcess.MemAddress, pE2PROM->CommandHeaderInProcess.Len); 
                                            }
                                          }
                                        }
                                        break;

                                    case E2PROM_Variable:
                                        len              = (pE2PROM->CommandHeaderInProcess.Len > Stream_directAvailable(&pE2PROM->WriteStream)) ? Stream_directAvailable(&pE2PROM->WriteStream) : pE2PROM->CommandHeaderInProcess.Len;
                                        overPage         = (len > pE2PROM->Config->PageSize - (pE2PROM->CommandHeaderInProcess.MemAddress % pE2PROM->Config->PageSize)) ? 1 : 0;
                                        pE2PROM->TempLen = overPage ? pE2PROM->Config->PageSize - (pE2PROM->CommandHeaderInProcess.MemAddress % pE2PROM->Config->PageSize) : len;
                                        if (pE2PROM->InTransmit != 1) {
                                          pE2PROM->InTransmit = 1;
                                          result = eepromDriver->write(pE2PROM, pE2PROM->CommandHeaderInProcess.MemAddress, Stream_getReadPtr(&pE2PROM->WriteStream), pE2PROM->TempLen);
                                          if (result != E2PROM_Ok) {
                                            if (pE2PROM->Callbacks.onWriteError != NULL) {
                                                pE2PROM->Callbacks.onWriteError (&pE2PROM->WriteStream, pE2PROM->CommandHeaderInProcess.MemAddress, pE2PROM->CommandHeaderInProcess.Len); 
                                            }
                                          }
                                        }
                                        break;
                                }
                                pE2PROM->NextTick = eepromDriver->getTimestamp() + pE2PROM->Config->WriteDelayTime;
                            }
                            break;

                        case E2PROM_ReadMode:
                          if (pE2PROM->NextTick < eepromDriver->getTimestamp() && pE2PROM->CommandHeaderInProcess.Len > 0 && pE2PROM->InTransmit == 0) {
                            pE2PROM->InTransmit = 1;  
                            //eepromDriver->delayMs(10);
                            result = eepromDriver->read (pE2PROM, pE2PROM->CommandHeaderInProcess.MemAddress, Stream_getWritePtr(&pE2PROM->ReadStream), pE2PROM->CommandHeaderInProcess.Len);
                            if (result == E2PROM_Error) {
                              if (pE2PROM->Callbacks.onReadError != NULL) {
                                  pE2PROM->Callbacks.onReadError(&pE2PROM->ReadStream, pE2PROM->CommandHeaderInProcess.MemAddress, pE2PROM->CommandHeaderInProcess.Len); 
                              }
                            }
                            pE2PROM->NextTick = eepromDriver->getTimestamp() + pE2PROM->Config->WriteDelayTime;
                          }
                          break;

                        case E2PROM_EraseMode:
                            if (pE2PROM->NextTick < eepromDriver->getTimestamp() && pE2PROM->InTransmit == 0) {
                                overPage         = (pE2PROM->CommandHeaderInProcess.Len > pE2PROM->Config->PageSize - (pE2PROM->CommandHeaderInProcess.MemAddress % pE2PROM->Config->PageSize)) ? 1 : 0;
                                pE2PROM->TempLen = overPage ? pE2PROM->Config->PageSize - (pE2PROM->CommandHeaderInProcess.MemAddress % pE2PROM->Config->PageSize) : pE2PROM->CommandHeaderInProcess.Len;
                                pE2PROM->InTransmit = 1;
                                result = eepromDriver->write(pE2PROM, pE2PROM->CommandHeaderInProcess.MemAddress, pE2PROM->ConstVal, pE2PROM->TempLen);
                                if (result != E2PROM_Ok && pE2PROM->Callbacks.onWriteError != NULL) {
                                    pE2PROM->Callbacks.onWriteError (&pE2PROM->WriteStream, pE2PROM->CommandHeaderInProcess.MemAddress, pE2PROM->CommandHeaderInProcess.Len); 
                                }
                                pE2PROM->NextTick = eepromDriver->getTimestamp() + pE2PROM->Config->WriteDelayTime;
                            }
                            break;

                        case E2PROM_NoiseEraseMode:
                            if (pE2PROM->NextTick < eepromDriver->getTimestamp() && pE2PROM->InTransmit == 0) {
                                for (uint8_t i = 0; i < pE2PROM->Config->PageSize / 4; i++) {
                                    Stream_writeUInt32(&pE2PROM->NoiseEraseStream, eepromDriver->rand());
                                }
                                pE2PROM->InTransmit = 1;
                                result = eepromDriver->write(pE2PROM, pE2PROM->CommandHeaderInProcess.MemAddress, Stream_getReadPtr(&pE2PROM->NoiseEraseStream), pE2PROM->Config->PageSize);
                                if (result != E2PROM_Ok) {
                                    if (pE2PROM->Callbacks.onWriteError != NULL) {
                                        pE2PROM->Callbacks.onWriteError (&pE2PROM->WriteStream, pE2PROM->CommandHeaderInProcess.MemAddress, pE2PROM->CommandHeaderInProcess.Len); 
                                    }
                                }
                                pE2PROM->NextTick = eepromDriver->getTimestamp() + pE2PROM->Config->WriteDelayTime;
                            }
                            break;
                      }
                   }
                }
                pE2PROM = pE2PROM->Previous;
    }
    return allProcessDone;
#if E2PROM_CHECK_ENABLE
    }
#endif
}



/**
 * @brief this function must be in HAL_I2C_MemTxCpltCallback
 *
 * @param eeprom
 */
void E2PROM_writeIRQ (E2PROM* eeprom) {
  eeprom->InTransmit = 0;  
  if (!eeprom->Lock) {
        switch (eeprom->CommandHeaderInProcess.Type) {
            case E2PROM_Variable:

                switch (eeprom->CommandHeaderInProcess.Mode) {
                    case E2PROM_WriteMode:
                        Stream_moveReadPos(&eeprom->WriteStream, eeprom->TempLen);
                        eeprom->CommandHeaderInProcess.MemAddress += eeprom->TempLen;
                        eeprom->CommandHeaderInProcess.Len -= eeprom->TempLen;
                        break;

                    case E2PROM_NoiseEraseMode:
                        Stream_moveReadPos(&eeprom->NoiseEraseStream, eeprom->Config->PageSize);
                        eeprom->CommandHeaderInProcess.MemAddress += eeprom->Config->PageSize;
                        eeprom->CommandHeaderInProcess.Len -= eeprom->Config->PageSize;
                        break;
                }
                break;

            case E2PROM_Const:

                if (eeprom->CommandHeaderInProcess.Mode == E2PROM_WriteMode) {
                    eeprom->ConstVal += eeprom->TempLen;
                }
                eeprom->CommandHeaderInProcess.MemAddress += eeprom->TempLen;
                eeprom->CommandHeaderInProcess.Len -= eeprom->TempLen;
                break;
        }
    } else {
        eeprom->InBlocking = 0;
    }
}


/**
 * @brief this function must be in HAL_I2C_MemRxCpltCallback
 *
 * @param eeprom
 */
void E2PROM_readIRQ (E2PROM* eeprom) {
  eeprom->InTransmit = 0;
    if (!eeprom->Lock && eeprom->CommandHeaderInProcess.Len > 0) {
        Stream_moveWritePos (&eeprom->ReadStream, eeprom->CommandHeaderInProcess.Len);
        Queue_writeItem(&eeprom->ReadQueue, &eeprom->CommandHeaderInProcess);
        eeprom->CommandHeaderInProcess.Len = 0;
    }
    else {
        eeprom->InBlocking = 0;
    }
}



/**
 * @brief Blocking Write Functions
 *
 * @param eeprom Address of your E2PROM struct
 * @param data Address of your Data u want to store in E2PROM Chip
 * @param addr Address of E2PROM Chip u want to store data in it
 * @param len Length of your Data
 * @return E2PROM_Result
 */
E2PROM_Result E2PROM_writeBlocking (E2PROM* eeprom, uint16_t addr, void* data, uint16_t len) {
    E2PROM_CommandHeader cacheHeader;
    uint8_t              overPage = 0;
    uint8_t              tempLen;
    E2PROM_Result        result;
    if ((addr <= eeprom->Config->Size) && (len <= eeprom->Config->Size) && (len > 0)) {
        cacheHeader.MemAddress = addr;
        cacheHeader.Len        = len;
    } else {
        return E2PROM_HeaderValueError;
    }
    eeprom->InBlocking = 1;
    eeprom->Lock       = 1;
    while (cacheHeader.Len > 0) {
        overPage           = cacheHeader.Len > eeprom->Config->PageSize - (cacheHeader.MemAddress % eeprom->Config->PageSize) ? 1 : 0;
        tempLen            = overPage ? eeprom->Config->PageSize - (cacheHeader.MemAddress % eeprom->Config->PageSize) : cacheHeader.Len;
        eeprom->InBlocking = 1;
        result = eepromDriver->write(eeprom, cacheHeader.MemAddress, data, tempLen);
        if (result != E2PROM_Ok) {
           if (eeprom->Callbacks.onWriteError != NULL) {
              eeprom->Callbacks.onWriteError (&eeprom->WriteStream, eeprom->CommandHeaderInProcess.MemAddress, eeprom->CommandHeaderInProcess.Len);
           }
        }
#if E2PROM_USE_INTERRUPT_I2C
        while (eeprom->InBlocking) {
        }
#endif
        cacheHeader.Len        -= tempLen;
        data += tempLen;
        cacheHeader.MemAddress += tempLen;
        eepromDriver->delayMs (eeprom->Config->WriteDelayTime);
    }
    eeprom->Lock = 0;
    return E2PROM_Ok;
}



/**
 * @brief E2PROM
 *
 * @param eeprom Address of your E2PROM struct
 * @param data Address of your Data u want to store in E2PROM Chip
 * @param addr Address of E2PROM Chip u want to store data in it
 * @param len  length of Data
 * @param type Data Type (Const or Variable)
 * @return E2PROM_Result
 */
E2PROM_Result E2PROM_write (E2PROM* eeprom, uint16_t addr, uint8_t* data, uint16_t len, E2PROM_DataType type) {
    E2PROM_CommandHeader cacheHeader;
    if ((addr < eeprom->Config->Size) && (len > 0)) {
        cacheHeader.MemAddress = addr;
        cacheHeader.Len        = len;
        cacheHeader.Type       = type;
        cacheHeader.Mode       = E2PROM_WriteMode;
        Queue_writeItem(&eeprom->CommandQueue, &cacheHeader);

        if (cacheHeader.Type == E2PROM_Const) {
            Stream_writeBytes(&eeprom->WriteStream, (uint8_t*)&data, sizeof(data));
        } else {
            Stream_writeBytes(&eeprom->WriteStream, data, cacheHeader.Len);
        }
//        eeprom->WriteReady = 1;
        return E2PROM_Ok;
    } else {
        return E2PROM_HeaderValueError;
    }
}



/**
 * @brief this function use for read from E2PROM Chip
 *
 * @param eeprom Address of E2PROM Struct
 * @param buffer buffer u want to store ReadValue in it
 * @param addr Address of E2PROM u want to Read from that Address
 * @param len Length of your DataValue
 * @return E2PROM_Result
 */
E2PROM_Result E2PROM_read (E2PROM* eeprom, uint16_t addr, uint8_t len) {
    E2PROM_CommandHeader cacheHeader;
    if ((addr < eeprom->Config->Size) && (len > 0) && (len <= eeprom->Config->Size)) {
        cacheHeader.MemAddress = addr;
        cacheHeader.Len        = len;
        cacheHeader.Type       = E2PROM_Variable;
        cacheHeader.Mode       = E2PROM_ReadMode;
        Queue_writeItem (&eeprom->CommandQueue, &cacheHeader);
        return E2PROM_Ok;
    } else {
        return E2PROM_HeaderValueError;
    }
}



/**
 * @brief
 *
 * @param eeprom   address of eeprom struct
 * @param addr     address of eeprom u want to read
 * @param val      address of buffer u want to store Data in that
 * @param len      length of Data
 * @return         E2PROM_Result
 */
E2PROM_Result E2PROM_readBlocking (E2PROM* eeprom, uint16_t addr, uint8_t* val, uint16_t len) {
  E2PROM_Result result;  
  if ((addr < eeprom->Config->Size) && (len > 0) && (len < eeprom->Config->Size)) {
        eeprom->Lock       = 1;
        eeprom->InBlocking = 1;
      if (eeprom->InTransmit == 0) {
        eeprom->InTransmit = 1;  
        result = eepromDriver->read(eeprom, addr, val, len);
        if (result != E2PROM_Ok) {
            if (eeprom->Callbacks.onReadError != NULL) {
                eeprom->Callbacks.onReadError (&eeprom->ReadStream, eeprom->CommandHeaderInProcess.MemAddress, eeprom->CommandHeaderInProcess.Len); 
            }
        }
      }
#if E2PROM_USE_INTERRUPT_I2C
        while (eeprom->InBlocking) {
            
        }
#endif
        eeprom->Lock = 0;
        return E2PROM_Ok;
    } else {
        return E2PROM_HeaderValueError;
    }
}



/**
 * @brief E2PROM NonBlocking erase  
 *
 * @param eeprom
 */
void E2PROM_erase (E2PROM* eeprom) {
    E2PROM_CommandHeader cacheHeader;
    cacheHeader.Len        = eeprom->Config->Size;
    cacheHeader.MemAddress = 0;
    cacheHeader.Mode       = E2PROM_EraseMode;
    cacheHeader.Type       = E2PROM_Const;
    Queue_writeItem (&eeprom->CommandQueue, &cacheHeader);
}




/**
 * @brief get E2PROM Config
 *
 * @param eeprom Address of E2PROM Struct
 * @return const E2PROM_Config*
 */
const E2PROM_Config* E2PROM_getConfig (E2PROM* eeprom) {
    return eeprom->Config;
}




/**
 * @brief Set user Argument
 *
 * @param eeprom Address of E2PROM Struct
 * @param args
 */
void StatusLed_setArgs (E2PROM* eeprom, void* args) {
    eeprom->Args = args;
}



/**
 * @brief get User Argument
 *
 * @param eeprom Address of E2PROM Struct
 * @return void*
 */
void* E2PROM_getArgs (E2PROM* eeprom) {
    return eeprom->Args;
}




/**
 * @brief Callback Func
 *
 * @param eeprom Address of E2PROM Struct
 * @param cb
 */
void E2PROM_onWrite (E2PROM* eeprom, E2PROM_CallbackFn cb) {
    eeprom->Callbacks.onAfterWrite = cb;
}



/**
 * @brief Callback Func
 *
 * @param eeprom Address of E2PROM Struct
 * @param cb
 */
void E2PROM_onRead (E2PROM* eeprom, E2PROM_CallbackFn cb) {
    eeprom->Callbacks.onRead = cb;
}


/**
 * @brief Callback Func
 *
 * @param eeprom Address of E2PROM Struct
 * @param cb
 */
void E2PROM_onReadError (E2PROM* eeprom, E2PROM_CallbackFn cb) {
    eeprom->Callbacks.onReadError = cb;
}


/**
 * @brief Callback Func
 *
 * @param eeprom Address of E2PROM Struct
 * @param cb
 */
void E2PROM_onWriteError (E2PROM* eeprom, E2PROM_CallbackFn cb) {
    eeprom->Callbacks.onWriteError = cb;
}


/**
 * @brief Add another E2PROM to the Process
 *
 * @param eeprom Address of New E2PROM Struct
 * @param config Address of New E2PROM Config
 * @return E2PROM_Result
 */
E2PROM_Result E2PROM_add (E2PROM* eeprom, const E2PROM_Config* config) {
    // check for null
    if (E2PROM_NULL == eeprom) {
        return E2PROM_Null;
    }
    E2PROM_setConfig(eeprom, config);

    // add E2PROM to linked list
    eeprom->Previous   = __eeprom();
    lastE2PROM         = eeprom;
    eeprom->Configured = 1;

#if E2PROM_CHECK_ENABLE
    eeprom->Enabled    = 1;
#endif

    return E2PROM_Ok;
}




/**
 * @brief Remove the E2PROM
 *
 * @param remove Address of E2PROM Struct user Want to remove that
 * @return E2PROM_Result
 */
E2PROM_Result E2PROM_remove (E2PROM* remove) {

    E2PROM* pE2PROM = __eeprom();
    if (remove == pE2PROM) {
        lastE2PROM = remove->Previous;
        remove->Previous   = E2PROM_NULL;
        remove->Configured = 0;
        remove->Enabled    = 0;
        return E2PROM_Ok;
    }
    while (E2PROM_NULL != pE2PROM) {
        if (remove == pE2PROM->Previous) {
            pE2PROM->Previous  = remove->Previous;
            remove->Previous   = E2PROM_NULL;
            remove->Configured = 0;
            remove->Enabled    = 0;
            return E2PROM_Ok;
        }
        pE2PROM = pE2PROM->Previous;
    }
    return E2PROM_Error;
}


#if E2PROM_CHECK_ENABLE
/**
 * @brief set enable/disable eeprom
 *
 * @param eeprom Address of E2PROM Struct
 * @param enabled
 */
void E2PROM_setEnabled (E2PROM* eeprom, uint8_t enabled) {
    eeprom->Enabled = enabled;
}

/**
 * @brief return 1 if Enable and 0 if Disable
 *
 * @param eeprom Address of E2PROM Struct
 * @return uint8_t
 */
uint8_t E2PROM_isEnabled (E2PROM* eeprom) {
    return eeprom->Enabled;
}

#endif



/**
 * @brief this Function use for compare 2 Array user can test the write and Read,  
 * 
 * @param arr1 Address of Array1
 * @param arr2 Address of Array2
 * @param len  Length user want to compare between the Array1 & Array2 
 * @return int8_t / return 0 if the two Array is Equal
 */
int8_t E2PROM_assertMemory (uint8_t* arr1, uint8_t* arr2, uint16_t len) {
  return memcmp (arr1, arr2, len);
}

/************************************************ Write/Read NonBlocking *****************************************************/
E2PROM_Result E2PROM_writeUInt8 (E2PROM* eeprom, uint8_t val, uint16_t addr) {
    return E2PROM_write (eeprom, addr, (uint8_t*)&val, sizeof(val), E2PROM_Variable);
}
void E2PROM_readUInt8 (E2PROM* eeprom, uint16_t addr) {
    E2PROM_read (eeprom, addr, sizeof(uint8_t));
}


E2PROM_Result E2PROM_writeUInt16 (E2PROM* eeprom, uint16_t val, uint16_t addr) {
    return E2PROM_write (eeprom, addr, (uint8_t*)&val, sizeof(val), E2PROM_Variable);
}
void E2PROM_readUInt16 (E2PROM* eeprom, uint16_t addr) {
    E2PROM_read (eeprom, addr, sizeof(uint16_t));
}


E2PROM_Result E2PROM_writeUInt32 (E2PROM* eeprom, uint32_t val, uint16_t addr) {
    return E2PROM_write (eeprom, addr, (uint8_t*)&val, sizeof(val), E2PROM_Variable);
}
void E2PROM_readUInt32 (E2PROM* eeprom, uint16_t addr) {
    E2PROM_read (eeprom, addr, sizeof(uint32_t));
}



E2PROM_Result E2PROM_writeUInt64 (E2PROM* eeprom, uint64_t val, uint16_t addr) {
    return E2PROM_write (eeprom, addr, (uint8_t*)&val, sizeof(val), E2PROM_Variable);
}

void E2PROM_readUInt64 (E2PROM* eeprom, uint16_t addr) {
    E2PROM_read (eeprom, addr, sizeof(uint64_t));
}

/************************************************** Write/Read Blocking *********************************************************/

E2PROM_Result E2PROM_writeUInt8Blocking (E2PROM* eeprom, uint8_t val, uint16_t addr) {
    return E2PROM_writeBlocking (eeprom, addr, (uint8_t*)&val, sizeof(val));
}
uint8_t E2PROM_readUInt8Blocking (E2PROM* eeprom, uint16_t addr) {
    uint8_t val = 0;
    E2PROM_readBlocking (eeprom, addr, (uint8_t*)&val, sizeof(val));
    return val;
}


E2PROM_Result E2PROM_writeUInt16Blocking (E2PROM* eeprom, uint16_t val, uint16_t addr) {
    return E2PROM_writeBlocking (eeprom, addr, (uint8_t*)&val, sizeof(val));
}
uint16_t E2PROM_readUInt16Blocking (E2PROM* eeprom, uint16_t addr) {
    uint16_t val = 0;
    E2PROM_readBlocking (eeprom, addr, (uint8_t*)&val, sizeof(val));
    return val;
}


E2PROM_Result E2PROM_writeUInt32Blocking (E2PROM* eeprom, uint32_t val, uint16_t addr) {
    return E2PROM_writeBlocking (eeprom, addr, (uint8_t*)&val, sizeof(val));
}

uint32_t E2PROM_readUInt32Blocking (E2PROM* eeprom, uint16_t addr) {
    uint32_t val = 0;
    E2PROM_readBlocking (eeprom, addr, (uint8_t*)&val, sizeof(val));
    return val;
}


E2PROM_Result E2PROM_writeUInt64Blocking (E2PROM* eeprom, uint64_t val, uint16_t addr) {
    return E2PROM_writeBlocking(eeprom, addr, (uint8_t*)&val, sizeof(val));
}
uint64_t E2PROM_readUInt64Blocking (E2PROM* eeprom, uint16_t addr) {
    uint64_t val = 0;
    E2PROM_readBlocking (eeprom, addr, (uint8_t*)&val, sizeof(val));
    return val;
}
/***********************************************************************************************************/


/**
 * @brief if the handle process not complete with this Function u can wait As much as this timeout 
 * 
 * @param timeout 
 * @return E2PROM_Result 
 */
E2PROM_Result E2PROM_waitForFinishProcess (E2PROM_Timestamp timeout) {    
     E2PROM_Timestamp time = eepromDriver->getTimestamp() + timeout;
    while ( E2PROM_handle() && eepromDriver->getTimestamp() < time) {
    }
}


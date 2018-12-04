/****************************************************************************
 *
 * File:          		rsa_rti_en_de.h
 *
 * Description:   		RSA 1024 bits encrypt + decrypt library
 * 
 * Version:		  			1.0
 *
 * Bulit Platform:    IAR 6.3
 *
 * CPU	:							EFM32 Cortex M3
 *
 * ALL RIGHTS RESERVED.
 *
 * Unpublished Confidential Information of RTI.  
 * Do Not Disclose.
 * 
 * No part of this work may be used or reproduced in any form or by any 
 * means, or stored in a database or retrieval system, without prior written 
 * permission of RTI.
 * 
 * Use of this work is governed by a license granted RTI.
 * This work contains confidential and proprietary information of
 * RTI. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
 
#ifndef __RSA_RTI_RSA_EN_DE_H__
#define __RSA_RTI_RSA_EN_DE_H__

#include "stdint.h"
/***************************************************************************//**
*  NOTE 
* 1. This library support RSA 1024 bits encrypt and dccrypt.    
  2. RSA bits just only support 1024 bit.
        If want support any other bits,pls contact RTI. 
  3. This RSA library just ONLY support EFM32 cortex M3 MCU.
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *  Rsa structure initial. 
 *
 * @details
 * Must initial structure before any RSA library used
 * 
 * @param[in] 
 * none
 * 
 * @return
 * 0 => success, if return 1,plse check NOTE:  
 *
 ******************************************************************************/
extern uint8_t rsa_struc_init(void);

/***************************************************************************//**
 * @brief
 *  Rsa encrypt initial procedure. 
 *  Must alloc it before  rsa_encrypt fuction used. 
 *
 * @details
 * There two things in this function. 
 * 1. step up public mod values .
 * 2. step up public key values.
 * 
 * @param[in] p_mod
 *   Pointer to pubic mod buf .
 *
 * @param[in] mod_len
 *   mod_len don't over 32.
 *
 * @param[in] p_pb_key
 *   Pointer to pubic key buf .
 *
 * @param[in] pb_key_len
 *  pb_len don't over 32.
 * @return
 * 0 => success, if return 1,plse check NOTE:  
 *
 ******************************************************************************/
extern uint8_t rsa_encrypt_init(uint32_t *p_mod, uint8_t mod_len, uint32_t *p_pb_key, uint8_t pb_key_len);

/***************************************************************************//**
 * @brief
 *  Rsa dencrypt initial procedure. 
 *  Must alloc it before  rsa_dencrypt fuction used. 
 *
 * @details
 * There two things in this function. 
 * 1. step up private mod values .
 * 2. step up private key values.
 * 
 * @param[in] p_mod
 *   Pointer to private mod buf .
 *
 * @param[in] mod_len
 *   mod_len don't over 32.
 *
 * @param[in] p_pr_key
 *   Pointer to private key buf .
 *
 * @param[in] pr_key_len
 *  pr_key_len don't over 32.
 * @return
 * 0 => success, if return 1,plse check NOTE:  
 *
 ******************************************************************************/
extern uint8_t rsa_decrypt_init(uint32_t *p_mod, uint8_t mod_len, uint32_t *p_pr_key, uint8_t pr_key_len);

/***************************************************************************//**
 * @brief
 *  Rsa rsa_encrypt procedure. 
 *
 * @details
 * 
 * @param[in] p_mes
 *   Pointer to message which need encrypt .
 *
 * @param[in] mes_len
 *   mod_len don't over 32.
 *
 * @return
 * 0 => success, if return 1,plse check NOTE:  
 *
 ******************************************************************************/
extern uint8_t rsa_encrypt(uint32_t *p_mes, uint8_t mes_len);

/***************************************************************************//**
 * @brief
 *  Rsa rsa_dencrypt procedure. 
 *
 * @details
 * 
 * @param[in] p_mes
 *   Pointer to message which need dencrypt .
 *
 * @param[in] mes_len
 *   mod_len don't over 32.
 *
 * @return
 * 0 => success, if return 1,plse check NOTE:  
 *
 ******************************************************************************/

extern uint8_t rsa_decrypt(uint32_t *p_mes, uint8_t mes_len);



#endif
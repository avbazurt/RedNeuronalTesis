/**
 * @file NexWaveform.h
 *
 * The definition of class NexWaveform. 
 *
 * @author Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date 2015/8/13
 * @author Jyrki Berg 3/3/2019 (https://github.com/jyberg)
 *
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#ifndef __NEXWAVEFORM_H__
#define __NEXWAVEFORM_H__

#ifdef STD_SUPPORT
#include <vector>
#endif

#include "NexTouch.h"
#include "NexHardware.h"
/**
 * @addtogroup Component 
 * @{ 
 */

/**
 * NexWaveform component.
 */
class NexWaveform: public NexTouch
{
public: /* methods */
    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name, const NexObject* page=nullptr);
     */
    NexWaveform(uint8_t pid, uint8_t cid, const char *name, const NexObject* page=nullptr);

    /**
     * @copydoc NexObject::NexObject(uint8_t pid, uint8_t cid, const char *name, 
     * int32_t minVal, int32_t maxVal, uint8_t hight,
     * const NexObject* page=nullptr);
     * 
     * Constructor with value scaling parameters, Scales added value to set Waveform scale 
     */
    NexWaveform(uint8_t pid, uint8_t cid, const char *name, 
        float minVal, float maxVal, uint8_t hight,
        const NexObject* page=nullptr);

    /**
     * Add value to show. 
     *
     * @param ch - channel of waveform(0-3). 
     * @param value - the value of waveform (value is scaled to nextion resolution 0-255 based on Min / Max component hight).  
     *
     * @retval true - success. 
     * @retval false - failed. 
     */
    template<typename T>
    bool addValue(uint8_t ch, T value)
    {
        #ifdef STD_SUPPORT
        // compile time data type check 
        static_assert(std::is_arithmetic<T>::value, "Not numeric type");
        #endif
        char buf[15] = {0};
        
        if (ch > 3)
        {
            return false;
        }

        sprintf(buf, "add %u,%u,%u", getObjCid(), ch, ScaleToForm(value));
        sendCommand(buf);
        return recvRetCommandFinished();
    }

    /**
     * Add values to show. 
     *
     * @param ch - channel of waveform(0-3). 
     * @param values - the values of waveform (values are scaled to nextion resolution 0-255 based on Min / Max component hight).  
     *
     * @retval true - success. 
     * @retval false - failed. 
     */
    #ifdef STD_SUPPORT
    template<typename T>
    bool addValues(uint8_t ch, std::vector<T> &values)
    {
        // compile time data type check  
        static_assert(std::is_arithmetic<T>::value, "Not numeric type");

        bool ret=true;
 
        for(uint16_t offset{0}; offset < values.size() && ret;)
        {
            uint32_t sendBytes{values.size()-offset};
            if(sendBytes>124)
            {
                sendBytes=124;
            }
            { 
            char buf[4] = {0};
            utoa(getObjCid(), buf, 10);
            String cmd;
            cmd = "addt ";
            cmd += buf;
            cmd += ",";
            utoa(ch, buf, 10);
            cmd += buf;
            cmd += ",";
            utoa(sendBytes, buf, 10);
            cmd += buf;
            sendCommand(cmd.c_str());
            }

            if(!RecvTransparendDataModeReady())
            {
                ret=false;
                break;
            }
            for(uint16_t i{0}; i<sendBytes; ++i)
            {
                sendRawByte(ScaleToForm(values[offset++]));
            }
            if(!RecvTransparendDataModeFinished())
            {
                ret=false;
            }
        }
 
        return ret;
    }
#endif

    /**
     * Add values to show. 
     *
     * @param ch - channel of waveform(0-3). 
     * @param values - pointer to the values of waveform (values are scaled to nextion resolution 0-255 based on Min / Max component hight). 
     * @param len - values buffer len
     *
     * @retval true - success. 
     * @retval false - failed. 
     */
    template<typename T>
    bool addValues(uint8_t ch, T *values, uint16_t len)
    {
        #ifdef STD_SUPPORT
        // compile time data type check 
        static_assert(std::is_arithmetic<T>::value, "Not numeric type");
        #endif

        bool ret=true;
 
        for(uint16_t offset{0}; offset < len && ret;)
        {
            uint32_t sendBytes{len-offset};
            if(sendBytes>124)
            {
                sendBytes=124;
            }
            { 
            char buf[4] = {0};
            utoa(getObjCid(), buf, 10);
            String cmd;
            cmd = "addt ";
            cmd += buf;
            cmd += ",";
            utoa(ch, buf, 10);
            cmd += buf;
            cmd += ",";
            utoa(sendBytes, buf, 10);
            cmd += buf;
            sendCommand(cmd.c_str());
            }

            if(!RecvTransparendDataModeReady())
            {
                ret=false;
                break;
            }
            for(uint16_t i{0}; i<sendBytes; ++i)
            {
                sendRawByte(ScaleToForm(values[offset++]));
            }
            if(!RecvTransparendDataModeFinished())
            {
                ret=false;
            }
        }
 
        return ret;
    }



    /**
     * Get bco attribute of component
     *
     * @param number - buffer storing data retur
     * @return true if success, false for failure
     */
    bool Get_background_color_bco(uint32_t *number);
	
    /**
     * Set bco attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_background_color_bco(uint32_t number);
	
    /**
     * Get gdc attribute of component
     *
     * @param number - buffer storing data retur
     * @return true if success, false for failure
     */
    bool Get_grid_color_gdc(uint32_t *number);	

    /**
     * Set gdc attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_grid_color_gdc(uint32_t number);			
	
    /**
     * Get gdw attribute of component
     *
     * @param number - buffer storing data retur
     * @return true if success, false for failure
     */
    bool Get_grid_width_gdw(uint32_t *number);	

    /**
     * Set gdw attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_grid_width_gdw(uint32_t number);			
	
    /**
     * Get gdh attribute of component
     *
     * @param number - buffer storing data retur
     * @return true if success, false for failure
     */
    bool Get_grid_height_gdh(uint32_t *number);

    /**
     * Set gdh attribute of component
     *
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_grid_height_gdh(uint32_t number);			
	
    /**
     * Get pco<x> attribute of component
     *
     * @param ch - channel of waveform(0-3). 
     * @param number - buffer storing data retur
     * @return true if success, false for failure
     */
    bool Get_channel_color(uint8_t ch, uint32_t *number);	

    /**
     * Set pco<x> attribute of component
     *
     * @param ch - channel of waveform(0-3). 
     * @param number - To set up the data
     * @return true if success, false for failure
     */
    bool Set_channel_color(uint8_t ch, uint32_t number);

    /**
     *  Clear waveform component
     * @param ch - channel of waveform(0-3). 255 Clear all data
     * @return true if success, false for failure
     * */
    bool Clear(uint8_t ch);

private:

    template<typename T>
    inline uint8_t ScaleToForm(T value)
    {
        if(value > m_maxVal){value=m_maxVal;}
        else if(value < m_minVal){value=m_minVal;}
        value-= m_minVal;
        value *= m_scale;
        uint8_t v = abs((int16_t)value);
        return v;
    }
    float m_minVal;
    float m_maxVal;
    float m_scale;
    uint8_t m_hight;
};

/**
 * @}
 */

#endif /* #ifndef __NEXWAVEFORM_H__ */

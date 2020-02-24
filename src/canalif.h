// canalif.h
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2019 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#if !defined(CANALIF_H)
#define CANALIF_H

#include "canaldlldef.h"
#include <string>

class CCanalIf {

    CCanalIf();
    ~CCanalIf();

    /*!
        Initialize driver

        @param strpath CANAL driver pathn string
        @param strparam CANAL configuration string
        @param flags CANAL flags
        @param bAsync True to enable driver asynchronous functionally
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */

    int init(std::string strpath, std::string strparam, uint32_t flags, bool bAsync = false );

    /*!
        Initialize driver with JSON data

        @param jsonInit Init data as a JSON object
        {
            "path"   : "path to CANAL driver",
            "config" : "CANAL configuration string",
            "flags"  : 12345
            "bAsync" : true|false
        }
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */

    int init( std::string jsonInit );

    /*!
        CanalOpen
    */
    int CanalOpen();

    /*!
        CanalClose
    */
    int CanalClose();

    /*!
        CanalSend - Send CAN message

        @param strJSON CAN msg to send on JSON format
        {
            "can" : {
                "flags"     : 1234
                "obid"      : 1234
                "id"        : 1234
                "data"      : [1,2,3]
                "timestamp" : 1234
            }

            "vscp"      : {
                “head”:0,
                “vscpclass”: 10,
                “vscptype”: 6,
                "guid": "ff:ee:dd:cc:bb:aa:99:88:77:66:55:44:33:22:11:00",
                "timestamp": 1234567,
                "datetime": "2018-03-03T12:01:40",
                "data":[1,2,3,4],
                “unit”: 0,
                “sensorindex”: 0,
                “coding”: 0,
                “value”: 1.2345,
            }
        }

        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalSend(std::string strJSON);

    /*!
        CanalBlockingSend

        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalBlockingSend(std::string &strCanMsg, uint32_t timeout);

    /*!
        CanalReceive

        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalReceive(std::string &strCanMsg);

    /*!
        CanalBlockingReceive

        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalBlockingReceive(std::string &strCanMsg, uint32_t timeout=0);

    /*!
        CanalDataAvailable
        @return number of messages in input queue
    */
    int CanalDataAvailable(void);

    /*!
        CanalGetStatus

        @param jsonStatus Channel status as JSON object

        {
            "status" : 1234,
            "lasterror" : 1234,
            "lastsuberror" : 1234,
            "lasterrorstr" : "Some error string"
        }

        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalGetStatus(std::string &jsonStatus);

    /*!
        CanalGetStatistics

        @param jsonStatistics Statistics as a JSON object
        {
            "cntReceiveFrames"   : 1234,
            "cntTransmittFrames" : 1234,
            "cntReceiveData"     : 1234,
            "cntTransmittData"   : 1234,
            "cntOverruns"        : 1234,
            "cntBusWarnings"     : 1234,
            "cntBusOff"          : 1234
        }

        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalGetStatistics(std::string &jsonStatistics);


    /*!
        CanalSetFilter

        @param filter Filter as a 32-bit number
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalSetFilter(uint32_t filter);

    /*!
        CanalSetMask

        @param mask Mask as a 32-bit number
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalSetMask(uint32_t mask);

    /*!
        CanalSetBaudrate

        Set a new baudrate
        @param baudrate Baudrate as a re.but number with meaning specific to device.
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalSetBaudrate(uint32_t baudrate);

    /*!
        CanalGetVersion

        Get version
        @return Version packed in unsigned int.
    */
    uint32_t CanalGetVersion(void);

    /*!
        CanalGetDllVersion

        Get DLL version
        @return DLL version packed in unsigned int.
    */
    uint32_t CanalGetDllVersion(void);

    /*!
        CanalGetVendorString

        @return Pointer to zero terminated vendor string (UTF8)
    */
    const char *CanalGetVendorString(void);

    /*!
        CanalGetDriverInfo .


    */
    const char *CanalGetDriverInfo(void);

    // Worker thread data
    bool m_bQuit;

public:

    // Handle for dll/dl driver interface
    long m_openHandle;
    
    // DLL handle
    void *m_hdll;

    // Level I (CANAL) driver methods
    LPFNDLL_CANALOPEN m_proc_CanalOpen;
    LPFNDLL_CANALCLOSE m_proc_CanalClose;
    LPFNDLL_CANALGETLEVEL m_proc_CanalGetLevel;
    LPFNDLL_CANALSEND m_proc_CanalSend;
    LPFNDLL_CANALRECEIVE m_proc_CanalReceive;
    LPFNDLL_CANALDATAAVAILABLE m_proc_CanalDataAvailable;
    LPFNDLL_CANALGETSTATUS m_proc_CanalGetStatus;
    LPFNDLL_CANALGETSTATISTICS m_proc_CanalGetStatistics;
    LPFNDLL_CANALSETFILTER m_proc_CanalSetFilter;
    LPFNDLL_CANALSETMASK m_proc_CanalSetMask;
    LPFNDLL_CANALSETBAUDRATE m_proc_CanalSetBaudrate;
    LPFNDLL_CANALGETVERSION m_proc_CanalGetVersion;
    LPFNDLL_CANALGETDLLVERSION m_proc_CanalGetDllVersion;
    LPFNDLL_CANALGETVENDORSTRING m_proc_CanalGetVendorString;

    // Generation 2
    LPFNDLL_CANALBLOCKINGSEND m_proc_CanalBlockingSend;
    LPFNDLL_CANALBLOCKINGRECEIVE m_proc_CanalBlockingReceive;
    LPFNDLL_CANALGETDRIVERINFO m_proc_CanalGetdriverInfo;

private:

    // Driver DLL/DL path
    std::string m_strPath;

    // Device configuration string
    std::string m_strParameter;

    // Device flags for CANAL DLL open
    uint32_t m_deviceFlags;

    // VSCP level for driver
    long m_driverLevel;

    // ------------------------------------------------------------------------
    //                     End of driver worker thread data
    // ------------------------------------------------------------------------

    
};

#endif
// DeviceList.h: interface for the CDeviceList class.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
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

#if !defined(_DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_)
#define _DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_

#include <deque>
#include <string>

#include <pthread.h>
#include <semaphore.h>

#include "canaldlldef.h"
#include "clientlist.h"
#include "devicethread.h"
#include "guid.h"
#include "vscpdlldef.h"

#define NO_TRANSLATION 0 // No translation bit set

// Out - translation bit definitions
#define VSCP_DRIVER_OUT_TR_M1M2F 0x01  // M1 -> M2 Float
#define VSCP_DRIVER_OUT_TR_M1M2S 0x02  // M1 -> M2 String
#define VSCP_DRIVER_OUT_TR_ALL512 0x04 // All to Level II events

// In - translation bit definitions

enum _driver_levels
{
    VSCP_DRIVER_LEVEL1 = 1,
    VSCP_DRIVER_LEVEL2,
    VSCP_DRIVER_LEVEL3
};

class CClientItem;
class cguid;
class CControlObject;

  ///////////////////////////////////////////////////////////////////////////////
  // Driver3Process
  //

  class Driver3Process
{

  public:
    Driver3Process();
    ~Driver3Process();

    void OnTerminate(int pid, int status);
};

///////////////////////////////////////////////////////////////////////////////
// CDeviceItem
//

class CDeviceItem
{

  public:
    /// Constructor
    CDeviceItem();

    /// Destructor
    virtual ~CDeviceItem();

    /*!
        Get driver info as string
        "bEnable,bActive,name,path,param,level,flags,guid,translation"
        @return Driver info
    */
    std::string getAsString(void);

    /*!
        Start driver
        @param Pointer to control object
        @return true on success, false on failure
    */
    bool startDriver(CControlObject *pCtrlObject);

    /*!
        Pause driver
        @return true on success, false on failure
    */
    bool pauseDriver(void); 

    /*!
        Resume driver
        @return true on success, false on failure
    */
    bool resumeDriver(void); 

    /*!
        Stop driver
        @return true on success, false on failure
    */
    bool stopDriver(void); 

 public:   

    // Name of device
    std::string m_strName;

    // Device configuration string
    std::string m_strParameter;

    // Driver DLL/DL path
    std::string m_strPath;

    // Canal/VSCP Driver Level
    uint8_t m_driverLevel;

    // True if driver should be started.
    bool m_bEnable;

    // Paused driver is inactive
    bool m_bActive;

    // termination control
    bool m_bQuit;

    /*!
        GUID to use for driver interface if set
        four msb should be zero for this GUID
    */
    cguid m_interface_guid;

    // Worker thread for device
    pthread_t m_deviceThreadHandle;
    pthread_mutex_t m_mutexdeviceThread;

    // Device flags for CANAL DLL open
    uint32_t m_DeviceFlags;

    // Client entry
    CClientItem *m_pClientItem;

    // Mutex handle that is used for sharing of the device.
    pthread_mutex_t m_deviceMutex;

    /*!
     *  Translation flags
     *  High 16-bits incoming.
     *  Low 16-bit outgoing.
     */
    uint32_t m_translation;

    // Handle for dll/dl driver interface
    long m_openHandle;

    // Level III driver pid
    long m_pid;

    // ------------------------------------------------------------------------
    //                     Start of driver worker thread data
    // ------------------------------------------------------------------------

    // Control object that invoked thread
    CControlObject *m_pCtrlObject;

    // Holder for CANAL receive thread
    pthread_t m_level1ReceiveThread;

    // Holder for CANAL write thread
    pthread_t m_level1WriteThread;

    // Holder for VSCP Level II receive thread
    pthread_t m_level2ReceiveThread;

    // Holder for VSCP Level II write thread
    pthread_t m_level2WriteThread;

    // ------------------------------------------------------------------------
    //                     End of driver worker thread data
    // ------------------------------------------------------------------------

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

    // Level II driver methods
    LPFNDLL_VSCPOPEN m_proc_VSCPOpen;
    LPFNDLL_VSCPCLOSE m_proc_VSCPClose;
    LPFNDLL_VSCPBLOCKINGSEND m_proc_VSCPBlockingSend;
    LPFNDLL_VSCPBLOCKINGRECEIVE m_proc_VSCPBlockingReceive;
    LPFNDLL_VSCPGETLEVEL m_proc_VSCPGetLevel;
    LPFNDLL_VSCPGETVERSION m_proc_VSCPGetVersion;
    LPFNDLL_VSCPGETDLLVERSION m_proc_VSCPGetDllVersion;
    LPFNDLL_VSCPGETVENDORSTRING m_proc_VSCPGetVendorString;
    LPFNDLL_VSCPGETDRIVERINFO m_proc_VSCPGetdriverInfo;
    LPFNDLL_VSCPGETWEBPAGETEMPLATE m_proc_VSCPGetWebPageTemplate;
    LPFNDLL_VSCPGETWEBPAGEINFO m_proc_VSCPGetWebPageInfo;
    LPFNDLL_VSCPWEBPAGEUPDATE m_proc_VSCPWebPageupdate;
};

class CDeviceList
{
  public:
    CDeviceList();
    virtual ~CDeviceList();

    /*!
        Add one driver item
        @param strName Driver name
        @param strParameters Driver configuration string
        @param flags Driver flags
        @param guid Interface GUID
        @param level Mark as Level I or Level II driver
        @param bEnable True to enable driver
        @param translation Bits to set translations to be performed.
        @return True is returned if the driver was successfully added.
    */
    bool addItem(const std::string &strName,
                 const std::string &strParameters,
                 const std::string &strPath,
                 uint32_t flags,
                 const cguid &guid,
                 uint8_t level        = VSCP_DRIVER_LEVEL1,
                 bool bEnable         = true,
                 uint32_t translation = NO_TRANSLATION);

    /*!
        Remove a driver item
        @param clientid for the driver to remove
        @return True if driver was removed successfully
                otherwise false.
    */
    bool removeItem(unsigned long id);

    /*!
        Get device item from GUID
        @param guid for device to look for
        @return Pointer to device item or NULL if not found
    */
    CDeviceItem *getDeviceItemFromGUID(cguid &guid);

    /*!
        Get device item from the client id
        @param guid for device to look for
        @return Pointer to device item or NULL if not found
    */
    CDeviceItem *getDeviceItemFromClientId(uint32_t id);

    /*!
        Get all drivers as a string
        @return String with device item info lines separated
        with \r\n
    */
    std::string getAllAsString(void);

    /*!
        Count number of drivers
        @param type Type of driver to count (1/2/3) or all (0)
        @param bOnlyActive True if only enabled drivers should be counted.
        @return number of drivers
    */
    uint16_t getCountDrivers(uint8_t type = 0, bool bOnlyActive = false);

  public:
    /*!
        List with devices
    */
    std::deque<CDeviceItem *> m_devItemList;
};

#endif // !defined(_DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_)

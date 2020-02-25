// canalif.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
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

#define _POSIX

#include <dlfcn.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>
#include <dlfcn.h>

#ifndef DWORD
#define DWORD unsigned long
#endif

#include "canal.h"
#include "canal_macro.h"
#include "canaldlldef.h"
#include "canalif.h"

void *deviceReceiveThread(void *pData);
void *deviceWriteThread(void *pData);

///////////////////////////////////////////////////////////////////////////////
// constructor
//

CCanalIf::CCanalIf()
{
    // Open syslog
    openlog("node-canal", LOG_CONS, LOG_LOCAL0);

    if (-1 == sem_init(&m_semClientOutputQueue, 0, 0)) {
        syslog(LOG_ERR, "Unable to init m_semClientOutputQueue");
        return;
    }

    if (-1 == sem_init(&m_semClientInputQueue, 0, 0)) {
        syslog(LOG_ERR, "Unable to init m_semClientInputQueue");
        return;
    }

    if (0 != pthread_mutex_init(&m_mutexClientOutputQueue, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutexClientOutputQueue");
        return;
    }

    if (0 != pthread_mutex_init(&m_mutexClientInputQueue, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutexClientInputQueue");
        return;
    }

    m_openHandle = 0;
    m_bQuit = false;
}

///////////////////////////////////////////////////////////////////////////////
// deconstructor
//

CCanalIf::~CCanalIf()
{
    if (0 != sem_destroy(&m_semClientOutputQueue)) {
        syslog(LOG_ERR, "Unable to destroy m_semClientOutputQueue");
    }

    if (0 != sem_destroy(&m_semClientInputQueue)) {
        syslog(LOG_ERR, "Unable to destroy m_semClientInputQueue");
    }

    if (0 != pthread_mutex_destroy(&m_mutexClientOutputQueue)) {
        syslog(LOG_ERR, "Unable to destroy m_mutexClientOutputQueue");
        return;
    }

    if (0 != pthread_mutex_destroy(&m_mutexClientInputQueue)) {
        syslog(LOG_ERR, "Unable to destroy m_mutexClientInputQueue");
        return;
    }

    // Close syslog
    closelog();
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int
CCanalIf::init(std::string strpath,
              std::string strparam,
              uint32_t flags,
              bool bAsync)
{
    const char *dlsym_error;

    m_openHandle = 0;

    // Save config data
    m_strPath      = strpath;
    m_strParameter = strparam;
    m_deviceFlags  = flags;

    // Load dynamic library
    m_hdll = dlopen(strpath.c_str(), RTLD_LAZY);
    if (!m_hdll) {
        syslog(LOG_ERR,
               "Devicethread: Unable to load dynamic library. path = %s",
               m_strPath.c_str());
        return CANAL_ERROR_PARAMETER;
    }

    // Now find methods in library
    syslog(LOG_INFO, "Loading level I driver: %s", strpath.c_str());

    // * * * * CANAL OPEN * * * *
    m_proc_CanalOpen        = (LPFNDLL_CANALOPEN)dlsym(m_hdll, "CanalOpen");
    dlsym_error = dlerror();

    if (dlsym_error) {
        // Free the library
        syslog(LOG_DEBUG,
               "%s : Unable to get dl entry for CanalOpen.",
               m_strPath.c_str());
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL CLOSE * * * *
    m_proc_CanalClose = (LPFNDLL_CANALCLOSE)dlsym(m_hdll, "CanalClose");
    dlsym_error       = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalClose.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GETLEVEL * * * *
    m_proc_CanalGetLevel =
      (LPFNDLL_CANALGETLEVEL)dlsym(m_hdll, "CanalGetLevel");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetLevel.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL SEND * * * *
    m_proc_CanalSend = (LPFNDLL_CANALSEND)dlsym(m_hdll, "CanalSend");
    dlsym_error      = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalSend.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL DATA AVAILABLE * * * *
    m_proc_CanalDataAvailable =
      (LPFNDLL_CANALDATAAVAILABLE)dlsym(m_hdll, "CanalDataAvailable");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalDataAvailable.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL RECEIVE * * * *
    m_proc_CanalReceive = (LPFNDLL_CANALRECEIVE)dlsym(m_hdll, "CanalReceive");
    dlsym_error         = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalReceive.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GET STATUS * * * *
    m_proc_CanalGetStatus =
      (LPFNDLL_CANALGETSTATUS)dlsym(m_hdll, "CanalGetStatus");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetStatus.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GET STATISTICS * * * *
    m_proc_CanalGetStatistics =
      (LPFNDLL_CANALGETSTATISTICS)dlsym(m_hdll, "CanalGetStatistics");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetStatistics.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL SET FILTER * * * *
    m_proc_CanalSetFilter =
      (LPFNDLL_CANALSETFILTER)dlsym(m_hdll, "CanalSetFilter");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalSetFilter.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL SET MASK * * * *
    m_proc_CanalSetMask = (LPFNDLL_CANALSETMASK)dlsym(m_hdll, "CanalSetMask");
    dlsym_error         = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalSetMask.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GET VERSION * * * *
    m_proc_CanalGetVersion =
      (LPFNDLL_CANALGETVERSION)dlsym(m_hdll, "CanalGetVersion");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetVersion.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GET DLL VERSION * * * *
    m_proc_CanalGetDllVersion =
      (LPFNDLL_CANALGETDLLVERSION)dlsym(m_hdll, "CanalGetDllVersion");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetDllVersion.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // * * * * CANAL GET VENDOR STRING * * * *
    m_proc_CanalGetVendorString =
      (LPFNDLL_CANALGETVENDORSTRING)dlsym(m_hdll, "CanalGetVendorString");
    dlsym_error = dlerror();
    if (dlsym_error) {
        // Free the library
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetVendorString.",
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_LIBRARY;
    }

    // ******************************
    //     Generation 2 Methods
    // ******************************

    // * * * * CANAL BLOCKING SEND * * * *
    m_proc_CanalBlockingSend =
      (LPFNDLL_CANALBLOCKINGSEND)dlsym(m_hdll, "CanalBlockingSend");
    dlsym_error = dlerror();
    if (dlsym_error) {
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalBlockingSend. Probably "
               "Generation 1 driver.",
               m_strPath.c_str());
        m_proc_CanalBlockingSend = NULL;
    }

    // * * * * CANAL BLOCKING RECEIVE * * * *
    m_proc_CanalBlockingReceive =
      (LPFNDLL_CANALBLOCKINGRECEIVE)dlsym(m_hdll, "CanalBlockingReceive");
    dlsym_error = dlerror();
    if (dlsym_error) {
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalBlockingReceive. "
               "Probably Generation 1 driver.",
               m_strPath.c_str());
        m_proc_CanalBlockingReceive = NULL;
    }

    // * * * * CANAL GET DRIVER INFO * * * *
    m_proc_CanalGetdriverInfo =
      (LPFNDLL_CANALGETDRIVERINFO)dlsym(m_hdll, "CanalGetDriverInfo");
    dlsym_error = dlerror();
    if (dlsym_error) {
        syslog(LOG_ERR,
               "%s: Unable to get dl entry for CanalGetDriverInfo. "
               "Probably Generation 1 driver.",
               m_strPath.c_str());
        m_proc_CanalGetdriverInfo = NULL;
    }

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalOpen
//

int
CCanalIf::CanalOpen()
{
    // Must NOT be open
    if (0 != m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    // Open the device
    m_openHandle =
      m_proc_CanalOpen((const char *)m_strParameter.c_str(), m_deviceFlags);

    // Check if the driver opened properly
    if (m_openHandle <= 0) {
        syslog(LOG_ERR,
               "Failed to open driver. Will not use it! %ld [%s] ",
               m_openHandle,
               m_strPath.c_str());
        dlclose(m_hdll);
        return CANAL_ERROR_NOT_OPEN;
    }

    // Get Driver Level
    m_driverLevel = m_proc_CanalGetLevel(m_openHandle);

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalClose
//

int
CCanalIf::CanalClose()
{

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalClose(m_openHandle);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSend
//

int
CCanalIf::CanalSend(std::string strMsg)
{
    canalMsg CanMsg;

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalSend(m_openHandle, &CanMsg);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalBlockingSend
//

int
CCanalIf::CanalBlockingSend(std::string &strCanMsg, uint32_t timeout)
{
    canalMsg CanMsg;

    // Check if generation 2
    if (NULL == m_proc_CanalBlockingSend) {
        return CANAL_ERROR_LIBRARY;
    }

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalBlockingSend(m_openHandle, &CanMsg, timeout);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalReceive
//

int
CCanalIf::CanalReceive(std::string &strCanMsg)
{
    canalMsg CanMsg;

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalReceive(m_openHandle, &CanMsg);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalBlockingReceive
//

int
CCanalIf::CanalBlockingReceive(std::string &strCanMsg, uint32_t timeout)
{
    canalMsg CanMsg;

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalBlockingReceive(m_openHandle, &CanMsg, timeout);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalDataAvailable
//

int
CCanalIf::CanalDataAvailable()
{
    int rv = m_proc_CanalDataAvailable(m_openHandle);
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatus
//

int
CCanalIf::CanalGetStatus(std::string &jsonStatus)
{
    int rv;
    canalStatus CanStatus;

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    rv = m_proc_CanalGetStatus(m_openHandle, &CanStatus);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatistics
//

int
CCanalIf::CanalGetStatistics(std::string &jsonStatistics)
{
    canalStatistics CanalStatistics;

    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalGetStatistics(m_openHandle, &CanalStatistics);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetFilter
//

int
CCanalIf::CanalSetFilter(uint32_t filter)
{
    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalSetFilter(m_openHandle, filter);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetMask
//

int
CCanalIf::CanalSetMask(uint32_t mask)
{
    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalSetMask(m_openHandle, mask);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetBaudrate
//

int
CCanalIf::CanalSetBaudrate(uint32_t baudrate)
{
    // Must be open
    if (0 == m_openHandle) {
        return CANAL_ERROR_NOT_OPEN;
    }

    int rv = m_proc_CanalSetBaudrate(m_openHandle, baudrate);
    if (CANAL_ERROR_SUCCESS != rv) {
        return rv;
    }
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVersion
//

uint32_t
CCanalIf::CanalGetVersion(void)
{
    return  m_proc_CanalGetVersion();
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetDllVersion
//

uint32_t
CCanalIf::CanalGetDllVersion(void)
{
    return  m_proc_CanalGetDllVersion();
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVendorString
//

const char *
CCanalIf::CanalGetVendorString(void)
{
    return  m_proc_CanalGetVendorString();
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetDriverInfo
//

const char *
CCanalIf::CanalGetDriverInfo(void)
{
    // Check if generation 2
    if (NULL == m_proc_CanalGetdriverInfo) {
        return NULL;
    }

    return  m_proc_CanalGetdriverInfo();
}

// ------------------------------------------------------------------------------------

//pif))
// {
//     syslog(LOG_ERR,
//            "%s: Unable to run the device Level II write worker thread.",
//            pif->m_strPath.c_str());
//     dlclose(m_hdll);
//     return NULL; // TODO close dll
// }

// if (pCtrlObj->m_debugFlags[0] & VSCP_DEBUG1_DRIVER) {
//     syslog(LOG_DEBUG,
//            "%s: [Device tread] Level II Write thread created.",
//            pif->m_strPath.c_str());
// }

/////////////////////////////////////////////////////////////////////////////
// Device read worker thread
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// initThreads
//

// int
// initThreads(void *pData)
// {
//     if (pthread_create(&pif->m_level2ReceiveThread,
//                    NULL,
//                    deviceReceiveThread,
//                    pif)) {
//     syslog(LOG_ERR,
//            "%s: Unable to run the device Level II read worker thread.",
//            pif->m_strPath.c_str());
//     pif->m_bQuit = true;
//     pthread_join(pif->m_level2WriteThread, NULL);
//     dlclose(m_hdll);
//     return NULL; // TODO close dll, kill other thread
// }

// if (pCtrlObj->m_debugFlags[0] & VSCP_DEBUG1_DRIVER) {
//     syslog(LOG_DEBUG,
//            "%s: [Device tread] Level II Write thread created.",
//            pif->m_strPath.c_str());
// }

// Just sit and wait until the end of the world as we know it...
// while (!pif->m_bQuit) {
//     sleep(1);
// }

// if (pCtrlObj->m_debugFlags[0] & VSCP_DEBUG1_DRIVER) {
//     syslog(LOG_DEBUG,
//            "%s: [Device tread] Level II Closing.",
//            pif->m_strPath.c_str());
// }

// Close channel
// m_proc_VSCPClose(pif->m_openHandle);

// if (pCtrlObj->m_debugFlags[0] & VSCP_DEBUG1_DRIVER) {
//     syslog(LOG_DEBUG,
//            "%s: [Device tread] Level II Closed.",
//            pif->m_strPath.c_str());
// }


//   pif->m_bQuit = true;
// pthread_join(pif->m_level2WriteThread, NULL);
// pthread_join(pif->m_level2ReceiveThread, NULL);

// Unload dll
// dlclose(m_hdll);

// if (pCtrlObj->m_debugFlags[0] & VSCP_DEBUG1_DRIVER) {
//     syslog(LOG_DEBUG,
//            "%s: [Device tread] Level II Done waiting for threads.",
//            pif->m_strPath.c_str());
// }

// Remove messages in the client queues
// pthread_mutex_lock(&pCtrlObj->m_clientList.m_mutexItemList);
// pCtrlObj->removeClient(pClientItem);
// pthread_mutex_unlock(&pCtrlObj->m_clientList.m_mutexItemList);

// return NULL;
// }

// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// deviceReceiveThread
//

void *
deviceReceiveThread(void *pData)
{
    canalMsg msg;
 
    CCanalIf *pif = (CCanalIf *)pData;
    if (NULL == pif) {
        syslog(
          LOG_ERR,
          "deviceLevel1ReceiveThread quitting due to NULL DevItem object.");
        return NULL;
    }

    // Blocking receive method must have been found
    if (NULL == pif->m_proc_CanalBlockingReceive ) {
        return NULL;
    }

    while (!pif->m_bQuit) {

        if (CANAL_ERROR_SUCCESS == pif->m_proc_CanalBlockingReceive(
                                     pif->m_openHandle, &msg, 500)) {

            // There must be room in the receive queue
            if (pif->m_clientOutputQueue.size() < MAX_CAN_MESSAGES) {

                canalMsg *pmsg = new canalMsg;
                if (NULL != pmsg) {

                    memset(pmsg, 0, sizeof(canalMsg));

                    // Set driver GUID if set
                    /*if ( pif->m_interface_guid.isNULL()
                    ) { pif->m_interface_guid.writeGUID(
                    pvscpEvent->GUID );
                    }
                    else {
                        // If no driver GUID set use interface GUID
                        pif->m_guid.writeGUID(
                    pvscpEvent->GUID );
                    }*/

                    // If if is set to zero use interface id
                    
                    pthread_mutex_lock(&pif->m_mutexClientOutputQueue);
                    pif->m_clientOutputQueue.push_back(pmsg);
                    sem_post(&pif->m_semClientOutputQueue);
                    pthread_mutex_unlock(&pif->m_mutexClientOutputQueue);
                }
            }
        }
    }

    return NULL;
}

// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// deviceWriteThread
//

void *
deviceWriteThread(void *pData)
{
    CCanalIf *pif = (CCanalIf *)pData;
    if (NULL == pif) {
        syslog(
          LOG_ERR,
          "deviceLevel1ReceiveThread quitting due to NULL DevItem object.");
        return NULL;
    }

    // Blocking send method must have been found
    if (NULL == pif->m_proc_CanalBlockingSend) return NULL;

    while (!pif->m_bQuit) {

        // Wait until there is something to send
        if ((-1 == sem_wait(
                     &pif->m_semClientInputQueue)) &&
            errno == ETIMEDOUT) {
            continue;
        }

        if (pif->m_clientInputQueue.size()) {

            pthread_mutex_lock(&pif->m_mutexClientInputQueue);
            canalMsg *pmsg =
              pif->m_clientInputQueue.front();
            pif->m_clientInputQueue.pop_front();
            pthread_mutex_unlock(&pif->m_mutexClientInputQueue);

            if (CANAL_ERROR_SUCCESS ==
                pif->m_proc_CanalBlockingSend(
                  pif->m_openHandle, pmsg, 300)) {
                //vscp_deleteVSCPevent(pqueueEvent);
            } else {
                // Give it another try
                sem_post(&pif->m_semClientOutputQueue);
            }

        } // events in queue

    } // while

    return NULL;
}

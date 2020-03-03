[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)
[![Travis Build Status](https://api.travis-ci.org/grodansparadis/node-red-contrib-vscp-tcp.svg?branch=master)](https://travis-ci.org/grodansparadis/node-red-contrib-vscp)

<img src="https://vscp.org/images/logo.png" width="100">

# node-canal

__Tested in node v11.8.0 and below__

CANAL module interface for node.js. CANAL stands for [CAN Abstraction Layer](https://docs.vscp.org/#canal) and is the least common denominator for low level drivers for the [Very Simple Control Protocol, VSCP](https://www.vscp.org). In the VSCP world CANAL driver is also called level I drivers. 

The CANAL interface is documented [here](https://docs.vscp.org/#canal)

There are plenty of CANAL drivers available. Some of them are documented [here](https://docs.vscp.org/vscpd/13.1/#/level_i_drivers)

You always send and receive CAN messages through a CANAL interface. This is just the abstraction and used and for example VSCP pack VSCP events into CAN packages for soem types of events. You can actually pack anything you like and anything the driver expect. The format is CAN but the meaning for each packet is up to you or at least the CANAL driver maker.

## Install

There are two options for installing node-canal:

### Clone / download node-can from GitHub, then:

```bash
    npm i
    npm run configure
    npm run build
```

### Install via npm

```bash
    npm install node-canal
```

## Usage

Generally below we use the [Socketcan CANAL driver](https://docs.vscp.org/vscpd/13.1/#/level1_driver_socketcan) to illustrated examples. This driver is chosen because it is easy to use on a system without any extra hardware which means everyone can take it for a test. candump/cansend from the [can-utils](https://github.com/linux-can/can-utils) package is useful tools. Install can-utils with 

```
sudo apt update
sudo apt install can-utils
```

To [set up a test CAN interface](https://en.wikipedia.org/wiki/SocketCAN)(_vcan0_) use

```bash
modprobe can
modprobe can_raw
modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
ip link show vcan0
```

### init

Before you can use the node-canal functionality, you must first call the init method. 

The init method specifies the path to the CANAL driver you want to use, a string  and a 32-bit flags value for configuration of it.  

The configurations string consist of a list of configuration values separated by semicolons. The flags value is a bit fields where each bit or groups of bits represent interface configuration. What values to use for a specific CANAL drivers is documented in the specific drivers documentation.

You can use node-canal either in polling mode, where you poll for messages, or in asynchronous mode where you get messages delivered to a function of your choice when they are received by the CANAL driver.

#### polling init

To use polling call init like this.

```javascript
const CANAL = require('node-canal');
const can = new CANAL.CNodeCanal();

var rv = can.init("/drivers/vscpl1drv-socketcan.so.1.1.0",
                  "vcan0",
                  0 ));
if ( CANAL.CANAL_ERROR_SUCCESS) {
  console.log("Initialization OK");
}                 
```

The arguments are obvious. First the path to the CANAL driver (here on a Linux system), then the driver configuration string. Here "vcan0", we accept defaults for the rest of the parameters. And last the flags byte which is set to zero.

#### asynchronous init

```javascript
const CANAL = require('node-canal');
const can = new CANAL.CNodeCanal();

const callback = (canmsg) => { 
  console.log(new Date, canmsg); 
  if ( canmsg.id == 0x999 ) {
    console.log('CNodeCanal close : ',can.close());
  }
};

can.init("/drivers/vscpl1drv-socketcan.so.1.1.0",
          "vcan0",
          0,
          callback ));
if ( CANAL.CANAL_ERROR_SUCCESS) {
  console.log("Initialization OK");
}          
```

Here a callback function is added both in itself and as a parameter to init. All other parameters are the same (see description in the polling init).

#### Return value

Is zero on success or on failure one of the [CANAL error codes](https://docs.vscp.org/canal/latest/#/errors).

###  open

After you initialized the driver you need to open the interface. The **open** method will do this for you

```javascript
if ( CANAL.CANAL_ERROR_SUCCESS != can.open() ) {
    console.log("There was an error opening CAN interface");
}
```

#### Return value

Is zero on success or on failure one of the [CANAL error codes](https://docs.vscp.org/canal/latest/#/errors).

###  close

Close the interface. This should be done when you are ready with the driver.

```javascript
if ( CANAL.CANAL_ERROR_SUCCESS != can.close() ) {
    console.log("There was an error opening CAN interface");
}
```

#### Return value

Is zero on success or on failure one of the [CANAL error codes](https://docs.vscp.org/canal/latest/#/errors) is returned.

###  send

Send a CAN message. You have two options. Either you send messages as a bunch of command arguments (flags, obid, timestamp, CAN id and an array for CAN data). Like this

```javascript
var hrTime = process.hrtime();

can.send(0x2020,0,(hrTime[0] * 1000000 + hrTime[1] / 1000),123,[1,2,3,4,5] ) );
```

The **flags** argument is defined [here](https://docs.vscp.org/canal/latest/#/canalMsg) in the CANAL specification.

The htTime is just an optional timestamp in microseconds. You can set this argument to zero if you don't need it.

The other alternative is to use an object on this form

```javascript
var hrTime = process.hrtime();

rv = can.send({
        id: 0x7f,
        flags: CANAL.CANAL_IDFLAG_EXTENDED,
        obid: 33,
        timestamp: (hrTime[0] * 1000000 +
                      hrTime[1] / 1000),
        data: [11,22,33,44,55,66,77,88],
        ext: true,
        rtr: false
    }));
if (CANAL.CANAL_ERROR_SUCCESS != rv ) {
  console.log("There was an error sending message.");
}    
```

The **flags** argument is defined [here](https://docs.vscp.org/canal/latest/#/canalMsg) in the CANAL specification. 

The **obid** (Object ID) can be used by application programs freely. 

**ext** should be true for an extended message and false otherwise. Setting this value to true is the same as setting bit one of **flags**. 

**rts** specifies a remote transmission request and is the same as setting bit 2 in **flags**.

#### Return value

Is zero on success or on failure one of the [CANAL error codes](https://docs.vscp.org/canal/latest/#/errors) is returned.

### receive

Use the _receive_ method to synchronously poll for messages. If you use a callback when initializing receive will not work for you.

Use code like this to receive messages.

```javascript
if ( count = can.dataAvailable() ) {
    can.receive( (canmsg) => {
        console.log("CAN message received:", canmsg)
    });
```

This is a synchronous method so the function argument will be called on return.

#### Return value

Is zero on success or on failure one of the [CANAL error codes](https://docs.vscp.org/canal/latest/#/errors) is returned.

### dataAvailable
Check how many message there are waiting to be received from the CANAL driver.

#### Return value
Return the number of messages waiting to be receive from the CANAL driver.

### getStatus
Get status for a CANAL channel.

Call it like this

```javascript
var rv = can.getStatus((status) => {
  console.log(status);
});
console.log("rv="+rv);
```
The [status object](https://docs.vscp.org/canal/latest/#/canalStatus) contains

  * **channel_status** - Current state for channel. 
  * **lasterrorcode** - Last error code.
  * **lasterrorsubcode** - Last error sub code.
  * **lasterrorstr** - Last error string.

Not all CANAL drivers use this structure and one must check the documentation for the driver before interpreting received data.  

### getStatistics

Get statistics for a CANAL channel.

Call it like this

```javascript
var rv = can.getStatistics((stat) => {
  console.log(stat);
});
console.log("rv="+rv);
```

The [statistics object](https://docs.vscp.org/canal/latest/#/canalStatistics) contains

  * **cntReceiveFrames** - # of receive frames
  * **cntTransmitFrames** - # of transmitted frames
  * **cntReceiveData** - # of received data bytes
  * **cntTransmitData** - # of transmitted data bytes
  * **cntOverruns** - # of overruns
  * **cntBusWarnings** -  # of bys warnings
  * **cntBusOff** - # of bus off's


#### Return value

Is zero on success or on failure one of the [CANAL error codes](https://docs.vscp.org/canal/latest/#/errors).

### setFilter

Set filter for a CANAL interface. Use to limit the messages that are received.

setFilter and setMask should normally be used together in a pair with each other. 

Use like this

```javascript
rv = can.setFilter(0x00000001):
rv = can.setMask(0x000000ff):
```

where the parameter is a 32-bit integer with the value that should be checked and the mask tell which bits that should be checked. A one is check, a zero is a don't care.

#### Return value

Is zero on success or on failure one of the [CANAL error codes](https://docs.vscp.org/canal/latest/#/errors). 

CANAL_ERROR_NOT_SUPPORTED (17) is returned if the interface does not support filtering.

### setMask

Set mask for a CANAL interface. Use to limit the messages that are received.

setFilter and setMask should normally be used together in a pair with each other.

```javascript
rv = can.setFilter(0x00000001):
rv = can.setMask(0x000000ff):
```
where the parameter is a 32-bit integer with the value that should be checked and the mask tell which bits that should be checked. A one is check, a zero is a don't care.

#### Return value

Is zero on success or on failure one of the [CANAL error codes](https://docs.vscp.org/canal/latest/#/errors) is returned. 

CANAL_ERROR_NOT_SUPPORTED (17) is returned if the interface does not support filtering.

### setBaudrate

Set the baudrate/bitrate for the interface. This method is seldom used. Check your driver documentation. 

The single parameter is the badrate/bitrate to set for the interface.

#### Return value

Is zero on success or on failure one of the [CANAL error codes](https://docs.vscp.org/canal/latest/#/errors) is returned. 

### getLevel

Get the driver level. This is a VSCP related command and a normal driver will return one 

#### Return value
Returns CANAL_LEVEL_STANDARD (1) except in rare cases. Check the documentation for the CANAL driver.

### getVersion

Get the version for the CANAL driver. 

```javascript
console.log("Version : ", can.getVersion().toString(16));
```

#### Return value
The version is returned packed in a 32-bit unsigned integer. MSB is major version, MSB + 1 is minor version, MSB+2 is release version and LSB is build version.

### getDllVersion

Get the version of the interface implementation. This is the version of the code designed to implement Canal for some specific hardware.

```javascript
console.log("DL(L) Version : ", can.getDllVersion().toString(16));
```

#### Return value
CANAL dll version expressed as an unsigned long with MAJOR_VERSION in first byte, MINOR_VERSION in second byte, RELEASE_VERSION in third byte and BUILD_VERSION in fourth byte. All stored on big endian.

### getVendorString

Get a pointer to a null terminated UTF8 vendor string for the maker of the interface implementation. This is a string that identifies the constructor of the interface implementation and can hold copyright and other valid information.

```javascript
console.log("Vendor : ", can.getVendorString());
```

#### Return value
Pointer to a null terminated UTF8 vendor string.

### getDriverInfo

This call returns a documentation object in XML form of the configuration string for the driver. This string describes configuration settings and flags setting and can be used to guide users to enter the configuration data in an application which allows for this.

See [the docs of CanalGetDriverInfo](https://docs.vscp.org/canal/latest/#/canalgetdriverinfo) for a full description.

## Constants

Most constants from the CANAL header is defined including errors, can-flag.bits, communication speeds. See [this page](https://docs.vscp.org/canal/latest/#/errors) for a complete list of error codes. The rtest of the constants can be found in the [canal.h header](https://github.com/grodansparadis/vscp/blob/master/src/vscp/common/canal.h).

## Samples

You have a couple of samples [here](https://github.com/grodansparadis/node-canal/tree/master/samples).

---

## VSCP & friends
The VSCP subsystem consist of many system components. 

### VSCP Daemon
The VSCP daemon is a central piece of software that act as a hub for VSCP based hardware or hardware that abstract as VSCP hardware, You can find the documentation for the VSCP daemon [here](https://docs.vscp.org/#vscpd).

### VSCP Works
VSCP works is a tool that make sinteraction with VSCP system components easy. VSCP Works is documented [here](https://docs.vscp.org/#vscpworks).

### VSCP Helper library
The VSCP helper library is a c/c++ library with common VSCP functionality. It is available for Python to and will be available for PHP and node.js. It is documented [here](https://docs.vscp.org/#vscphelper);  

### More
There is plenty of other tools available in the VSCP subsystem. Check the docs and the downloads.

### Other VSCP node-red nodes

There are other node-red parts in development or already available that makes it possible to easily connect to websocket interfaces on remote VSCP daemons/servers or hosts.

Checkout [node-red-contrib-vscp-tcp](https://www.npmjs.com/package/node-red-contrib-vscp-tcp) that contains nopes that connect to a remote VSCP tcp/ip host interface and send/receive events.

If you work with CAN, and especially CAN4VSCP, you might find [node-red-contrib-socketcan](https://www.npmjs.com/package/node-red-contrib-socketcan) and  [node-red-contrib-canal](https://www.npmjs.com/package/node-red-contrib-canal) useful.

---
Copyright © 2000-2020 Åke Hedman, Grodans Paradis AB


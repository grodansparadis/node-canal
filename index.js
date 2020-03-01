///////////////////////////////////////////////////////////////////////////
// index.js
//
// VSCP to CAN conversion node.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2020 Ake Hedman, Grodans Paradis AB
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

"use strict";

const CANAL = require('./build/Debug/nodecanal.node');
const can = new CANAL.CNodeCanal();

var rv;

const callback = (canmsg) => { 
  console.log(new Date, canmsg); 
  if ( canmsg.id == 0x999 ) {
    console.log('CNodeCanal close : ',can.close());
  }
};

console.log('CNodeCanal init : ',
can.init("/home/akhe/development/VSCP/vscpl1drv-socketcan/linux/vscpl1drv-socketcan.so.1.1.0",
          "vcan0",
          0,
          callback ));

console.log('CNodeCanal open : ',can.open());

var hrTime = process.hrtime()
console.log(hrTime[0] * 1000000 + hrTime[1] / 1000)
    
console.log('CNodeCanal send : ',
can.send(0x2020,0,(hrTime[0] * 1000000 + hrTime[1] / 1000),123,[1,2,3,4,5] ) );     
    
console.log('CNodeCanal send : ',
can.send({
        canid: 0x7f,
        flags: 0,
        obid: 33,
        timestamp: (hrTime[0] * 1000000 + hrTime[1] / 1000),
        data: [11,22,33,44,55,66,77,88],
        ext: true,
        rtr: false
    }));

console.log("Version : ", can.getVersion().toString(16));
console.log("DLL version : ", can.getDllVersion().toString(16));
console.log("Vendor : ", can.getVendorString());

console.log("CANAL status : ");
rv = can.getStatus((status) => {
  console.log(status);
});
console.log("rv="+rv);

console.log("CANAL statistics : ");
rv = can.getStatistics((stat) => {
  console.log(stat);
});
console.log("rv="+rv);


    // var brun = true;
    // var count;
    //  while (brun) {
    //       if ( count = can.dataAvailable() ) {
    //          console.log("count = ", count); 
    // // //         // can.receive( (canmsg) => {
    // // //         //     console.log("Message received:", canmsg)
    // // //         // });
    //       }
    // }    
    //console.log('CNodeCanal close : ', can.close());

module.exports = CANAL;
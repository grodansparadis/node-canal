///////////////////////////////////////////////////////////////////////////
// send.js
//
// node-canal send example.
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

const CANAL = require('bindings')('nodecanal');
const can = new CANAL.CNodeCanal();

var rv;

console.log('send.js');
console.log('=======');

const callback = (canmsg) => { 
    console.log(new Date, canmsg); 
    if ( canmsg.id == 0x999 ) {
      console.log('CNodeCanal close : ',can.close());
    }
  };

console.log('CNodeCanal init : ',
rv = can.init("/home/akhe/development/VSCP/vscpl1drv-socketcan/linux/vscpl1drv-socketcan.so.1.1.0",
          "vcan0",
          0, callback ));

if ( CANAL.CANAL_ERROR_SUCCESS != rv ) {
  console.log("Failed to initialized CANAL driver. Return code=",rv);
  process.exit();
}

console.log('CNodeCanal open : ',can.open());

for ( var i=0; i<100; i++) {
    var hrTime = process.hrtime();
    can.send({
        id: i,
        flags: CANAL.CANAL_IDFLAG_EXTENDED,
        obid: 33,
        timestamp: (hrTime[0] * 1000000 + hrTime[1] / 1000),
        data: [i,i,i,i,i,i,i,i],
        ext: true,
        rtr: false
    });
}

var id = setInterval( () => {
    console.log('CNodeCanal close : ',can.close());
    process.exit();
}, 2000 );
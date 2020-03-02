///////////////////////////////////////////////////////////////////////////
// polling.js
//
// node-canal polling example..
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

console.log('polling.js');
console.log('===========');

console.log('CNodeCanal init : ',
rv = can.init("/home/akhe/development/VSCP/vscpl1drv-socketcan/linux/vscpl1drv-socketcan.so.1.1.0",
                "vcan0",
                0 ));

if ( CANAL.CANAL_ERROR_SUCCESS != rv ) {
    console.log("Failed to initialized CANAL driver. Return code=",rv);
    process.exit();
}

console.log('CNodeCanal open : ',can.open());

var id = setInterval(checkMessage, 1000 );

function checkMessage()
{
    var count;
    if ( count = can.dataAvailable() ) {
        console.log("count = ", count); 
        var rv = can.receive( (canmsg) => {
            console.log("Message received:", canmsg)
            if ( canmsg.id == 0x999 ) {
                console.log('CNodeCanal close : ',can.close());
                process.exit();
              }
        });
    }
    else {
        console.log("No messages " + count);
    }
}


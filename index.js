//index.js
const CANAL = require('./build/Debug/nodecanal.node');

console.log("=======================================================");

const ccc = new CANAL.CNodeCanal();

console.log('CNodeCanal init : ',
ccc.init("/home/akhe/development/VSCP/vscpl1drv-socketcan/linux/vscpl1drv-socketcan.so.1.1.0",
          "vcan0",
          0));

console.log('CNodeCanal open : ',
ccc.open());

var hrTime = process.hrtime()
console.log(hrTime[0] * 1000000 + hrTime[1] / 1000)
    
console.log('CNodeCanal send : ',
ccc.send(0x2020,(hrTime[0] * 1000000 + hrTime[1] / 1000),123,[1,2,3,4,5] ) );     
    
console.log('CNodeCanal send : ',
ccc.send({
        canid: 0x7f,
        flags: 0,
        obid: 33,
        timestamp: hrTime,
        data: [11,22,33,44,55,66,77,88],
        ext: true,
        rtr: false
    }));

console.log("Version : ", ccc.getVersion().toString(16));
console.log("DLL version : ", ccc.getDllVersion().toString(16));
console.log("Vendor : ", ccc.getVendorString());

const callback = (...args) => { 
      console.log(new Date, ...args); 
    };

    var brun = true;
    // ccc.asyncReceive( function () {
    //     //console.log("JavaScript callback called with arguments", Array.from(arguments));
    //     console.log("Event ->>>>>>>>>>>>>>>>>>>>>>>>>");
    //     brun = false;
    // }, 2);

    void async function() {
      console.log(await ccc.addListener(callback));
    }();
    
    // var count;
    //  while (brun) {
    //       if ( count = ccc.dataAvailable() ) {
    //          console.log("count = ", count); 
    // // //         // ccc.receive( (msg) => {
    // // //         //     console.log("Message received:", msg)
    // // //         // });
    //       }
    // }    

    // readline.emitKeypressEvents(process.stdin);
    // process.stdin.setRawMode(true);
    // console.log('Press any key...');

    //console.log('CNodeCanal close : ', ccc.close());

    console.log("******************** EXIT ***********************");

module.exports = CANAL;
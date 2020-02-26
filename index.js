//index.js
const CANAL = require('./build/Debug/nodecanal.node');
console.log('addon :',CANAL);
console.log(CANAL.hello());
console.log('Saying :', CANAL.hello());
console.log('add ', CANAL.add(5, 10));

const classInstance = new CANAL.ClassExample(4.3);
console.log('Testing class initial value : ',classInstance.getValue());
console.log('After adding 3.3 : ',classInstance.add(3.3));

const ccc = new CANAL.CNodeCanal();
console.log('CNodeCanal init : ',
    ccc.init("/home/akhe/development/VSCP/vscpl1drv-socketcan/linux/vscpl1drv-socketcan.so.1.1.0",
    "vcan0",0));
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
        timestamp: 22,
        data: [11,22,33,44,55,66,77,88],
        ecxt: true,
        rtr: false
    }));
    while (true) {
        if ( ccc.dataAvailable() ) {
            ccc.receive( (msg) => {
                console.log("Message received:", msg)
            });
        }
    }

    console.log('CNodeCanal close : ',
    ccc.close());

module.exports = CANAL;
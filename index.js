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
    ccc.init("/home/akhe/development/VSCP/vscpl1drv-logger/linux/vscpl1drv-logger.so.1.1.0",
    "/tmp/testlog.txt",0));
console.log('CNodeCanal open : ',
    ccc.open());
    console.log('CNodeCanal send : ',
    ccc.send(0,123,[1,2,3,4,5] ) );    
console.log('CNodeCanal close : ',
    ccc.close());    

module.exports = CANAL;
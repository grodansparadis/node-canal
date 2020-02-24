//index.js
const CANAL = require('./build/Release/nodecanal.node');
console.log('addon :',CANAL);
console.log(CANAL.hello());
console.log('Saying :', CANAL.hello());
console.log('add ', CANAL.add(5, 10));

const classInstance = new CANAL.ClassExample(4.3);
console.log('Testing class initial value : ',classInstance.getValue());
console.log('After adding 3.3 : ',classInstance.add(3.3));

module.exports = CANAL;
var Dynamic = require('../build/Release/dynamic');
var d = new Dynamic();

console.log("Dynamic = ", Dynamic);
console.log("d = ", d);

console.log("d.get = ", d.get);
console.log("d.set = ", d.set);

d.get = function(name){
	return 10;
};
d.set = function(name, value){
};

console.log("d.get = ", d.get);
console.log("d.a = ", d.a);
console.log("d.prototype = ", d.prototype);

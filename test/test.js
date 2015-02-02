var Dynamic = require('../build/Release/dynamic');
var d = new Dynamic();
var p = {};

console.log("Dynamic = ", Dynamic);
console.log("d = ", d);

console.log("d.get = ", d.get);
console.log("d.set = ", d.set);

d.get = function(name){
	if(name == "prototype") return p;
	return 10;
};
d.set = function(name, value){
};

d.prototype.hoge = function(){};

console.log("d.get = ", d.get);
console.log("d.a = ", d.a);
console.log("d.prototype = ", d.prototype);
d.prototype.hoge();

var p = {
  a: function() { return 20; },
  get: function() { return 30; },
};
var d = new Dynamic(p);

console.log("d = ", d);
console.log("p = ", p);
console.log("d.a = ", d.a);
console.log("d.a() = ", d.a());
console.log("d.b = ", d.b);


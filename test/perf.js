var Dynamic = require('../build/Release/dynamic');

// Case.1
var d = new Dynamic();
d.get = function(name){ return 10; };
d.set = function(name, value){ };

var t1 = Date.now();
for(var i = 0; i < 100000; i++){
	d.a;
}
console.log((Date.now() - t1) / 100000);

// Case.2
var p = {
    get:  function() { return 10; }
};
var d = new Dynamic(p);
var t1 = Date.now();
for(var i = 0; i < 100000; i++){
	d.a;
}
console.log((Date.now() - t1) / 100000);

// Case.3
var p = { };
var d = new Dynamic(p);
d.get = function(name){ return 10; };
var t1 = Date.now();
for(var i = 0; i < 100000; i++){
	d.a;
}
console.log((Date.now() - t1) / 100000);

// Case.4
var d = {a: 10};
var t1 = Date.now();
for(var i = 0; i < 100000; i++){
	d.a;
}
console.log((Date.now() - t1) / 100000);

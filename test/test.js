/* test methods */
function assertTrue(expr){
	if(!expr) throw new Error("expr = false");
}

function assertEquals(lhs, rhs){
	if(lhs!==rhs) throw new Error("lhs != rhs");
}

function assertEqualsToType(lhs, type){
	if(typeof(lhs) !== type) throw new Error("expect " + type + ", but " + typeof(lhs));
}

function assertUndefined(lhs){
	assertEqualsToType(lhs, "undefined");
}
function assertObject(lhs){
	assertEqualsToType(lhs, "object");
}
function assertBoolean(lhs){
	assertEqualsToType(lhs, "boolean");
}
function assertNumber(lhs){
	assertEqualsToType(lhs, "number");
}
function assertString(lhs){
	assertEqualsToType(lhs, "string");
}
function assertFunction(lhs){
	assertEqualsToType(lhs, "function");
}

var Dynamic = require('..');
var d = new Dynamic();
var p = {};

console.log("Dynamic = ", Dynamic);
assertFunction(Dynamic);
console.log("d = ", d);
assertObject(d);

console.log("d.get = ", d.get);
assertUndefined(d.get);
console.log("d.set = ", d.set);
assertUndefined(d.set);

d.get = function(name){
	if(name == "prototype") return p;
	return 10;
};
d.set = function(name, value){ };

console.log("d.get = ", d.get);
assertFunction(d.get);
console.log("d.set = ", d.set);
assertFunction(d.set);
console.log("d.a = ", d.a);
assertEquals(10, d.a);

var p = {
  a: function() { return 20; },
  get: function() { return 30; },
};
var d = new Dynamic(p);

console.log("d = ", d);
assertObject(d);
console.log("p = ", p);
assertObject(p);
console.log("d.a = ", d.a);
assertFunction(d.a);
console.log("d.a() = ", d.a());
assertEquals(d.a(), 20);
console.log("d.b = ", d.b);
assertEquals(d.b, 30);

var d = new Dynamic();
var v = 0;
d.get = function(){ return v; }
d.set = function(key,value) { v = value; }
console.log("d.a = ", d.a);
assertEquals(d.a, 0);
d.a = 10;
console.log("d.a = ", d.a);
assertEquals(d.a, 10);
assertEquals(d.b, 10);

var d = new Dynamic();
d.set = function(k,v){ assertEquals("arg", k); }
d.arg = 10;
d.set = function(k,v){ assertEquals("gra", k); }
d.gra = 10;

var d = new Dynamic();
d.enumerate = function(){
  return ['a', 'b', 'c'];
}
var members= [];
for(k in d){members.push(k);}
assertEquals(3, members.length);
assertEquals(members[0], 'a');
assertEquals(members[1], 'b');
assertEquals(members[2], 'c');


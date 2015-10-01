node-dynamic
===

- master: [![Build Status](https://travis-ci.org/tmyt/node-dynamic.svg?branch=master)](https://travis-ci.org/tmyt/node-dynamic)

## overview

This library enables to declare property getter and setter by JavaScript.

## example

### declare getter by property
```javascript
var Dynamic = require('dynamic');
var d = new Dynamic();
d.get = function(n){ return 10; }
// expect: d.prop = 10
console.log("d.prop = " + d.prop);
// actual: d.prop = 10
```

### declare getter by base object
```javascript
var Dynamic = require('dynamic');
var d = new Dynamic({
    get: function(n){ return 10; }
});
// expect: d.prop = 10
console.log("d.prop = " + d.prop);
// actual: d.prop = 10
```

## constructor

### new Dynamic()

create new dynamic object instance.

### new Dyamic(object)

create new dynamic object baseed on 'object'.

## special members

these members reserved for internaly use.

- get
- set
- enumerate
- query
- delete

## handlers

### get(name)

return value for 'name'.

### set(name, value)

set value for 'name', and return setted 'value'.

### enumerate()

return array for property names for current object.

### query(name)

return property attribute for 'name'.

### delete(name)

delete property 'name'.


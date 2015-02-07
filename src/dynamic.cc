#include<node.h>
#include<cstring>

#include"dynamic.h"

using namespace v8;

#define CASTFN(x, y) Local<Function> x = Local<Function>::Cast(y)

template<typename T>
Local<T> CallHandler(const Local<Value>& thiz, const Local<Value>& fn_)
{
	CASTFN(fn, fn_);
	Local<Value> ret = fn->Call(thiz, 0, 0);
	return Handle<T>::Cast(ret);
}

template<typename T>
Local<T> CallHandler(const Local<Value>& thiz, const Local<Value>& fn_, const Local<Value>& arg1)
{
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1 };
	Local<Value> ret = fn->Call(thiz, 1, argv);
	return Handle<T>::Cast(ret);
}

template<typename T>
Local<T> CallHandler(const Local<Value>& thiz, const Local<Value>& fn_, const Local<Value>& arg1, const Local<Value>& arg2)
{
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1, arg2 };
	Local<Value> ret = fn->Call(thiz, 2, argv);
	return Handle<T>::Cast(ret);
}

template<>
Local<Value> CallHandler(const Local<Value>& thiz, const Local<Value>& fn_, const Local<Value>& arg1)
{
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1 };
	return fn->Call(thiz, 1, argv);
}

template<>
Local<Boolean> CallHandler(const Local<Value>& thiz, const Local<Value>& fn_, const Local<Value>& arg1)
{
	Isolate* isolate = Isolate::GetCurrent();
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1 };
	Local<Value> ret = fn->Call(thiz, 1, argv);
	return Boolean::New(isolate, ret->BooleanValue());
}

#define NC_(x, xx) !strcmp(x, (#xx)+1)
#define NC(x) NC_(prop+1, x)
#define NC2(x, xx) (*prop == x && NC(xx))
#define GET_HANDLER_PROP(name) \
	if(NC_(prop, name)) { info.GetReturnValue().Set(Local<Value>::New(isolate, obj->name##_)); return; }
#define GET_HANDLER_PROP_STATIC(name, value) \
	if(NC_(prop, name)) { info.GetReturnValue().Set(value); return; }
#define SET_HANDLER_PROP(name, value) \
	if(NC_(prop, name)) { obj->name##_.Reset(isolate, value); info.GetReturnValue().Set(value); return; }
#define IS_INTERNAL_PROP \
	NC2('g', get) || NC2('s', set) || NC2('q', query) || NC2('d', delete) || NC2('e', enumerate)
#define PRECHECK() if(*prop == 'g' || *prop == 's' || *prop == 'd' || *prop == 'e' || *prop == 'q')

#define CHECK(x) \
	LocalFunc(x); \
	if( !fn->IsFunction() && \
	    (!obj->has_super_ || !(fn = super->Get(Local<String>::New(isolate, property_names_[K_##x])))->IsFunction()) )
#define HANDLE(x) info.GetReturnValue().Set(x)

#define Undef(x) x##_.Reset(isolate, Undefined(isolate))
#define LocalValue(x, y) Local<Value> x = Local<Value>::New(isolate, y)
#define LocalFunc(x) LocalValue(fn, obj->x##_)

#define Prologue(x) \
	Isolate* isolate = Isolate::GetCurrent();\
	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(x.This());\
	Local<Object> super = Local<Object>::New(isolate, obj->super_)
#define Prop() \
	String::Utf8Value u8prop(property); \
	const char* prop = *u8prop
#define Each(x) x(get); x(set); x(query); x(delete); x(enumerate)

enum{
	K_get = 0,
	K_set,
	K_query,
	K_delete,
	K_enumerate,
};

Persistent<String> DynamicObject::property_names_[5];
Persistent<Function> DynamicObject::constructor;

DynamicObject::DynamicObject() : has_super_(false)
{
	Isolate* isolate = Isolate::GetCurrent();
	Each(Undef);
}

DynamicObject::~DynamicObject() { }

/* static */ void DynamicObject::Init(Handle<Object> exports, Handle<Object> module)
{
	Isolate* isolate = Isolate::GetCurrent();
#define n(x) property_names_[K_##x].Reset(isolate, String::NewFromUtf8(isolate, #x))
	Each(n);
#undef n
	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "DynamicObject"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	tpl->InstanceTemplate()->SetNamedPropertyHandler(Get, Set, Query, Delete, Enumerate);

	// Prototype
	constructor.Reset(isolate, tpl->GetFunction());
	module->Set(String::NewFromUtf8(isolate, "exports"),
				 tpl->GetFunction());
}

/* static */ void DynamicObject::New(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = Isolate::GetCurrent();

	if(!args.IsConstructCall()) return;
	// Construct object
	DynamicObject* obj = new DynamicObject();
	if(args[0]->IsObject()){
		obj->has_super_ = true;
		obj->super_.Reset(isolate, args[0]->ToObject());
	}
	obj->Wrap(args.This());
	args.GetReturnValue().Set(args.This());
}

/* static */ void DynamicObject::Get(Local<String> property, const PropertyCallbackInfo<Value>& info)
{
	Prologue(info);
	Prop();
	// check super-class members
	if(obj->has_super_ && super->Has(property)){
		info.GetReturnValue().Set(super->Get(property));
		return;
	}
	// check internal properties
	switch(*prop++){
		case 'g': GET_HANDLER_PROP(get); break;
		case 's': GET_HANDLER_PROP(set); break;
		case 'q': GET_HANDLER_PROP(query); break;
		case 'd': GET_HANDLER_PROP(delete); break;
		case 'e': GET_HANDLER_PROP(enumerate); break;
		case 'v': GET_HANDLER_PROP_STATIC(valueOf, info.This()); break;
		case 'i': GET_HANDLER_PROP_STATIC(inspect, Undefined(isolate)); break;
	}
	CHECK(get){
		info.GetReturnValue().Set(Undefined(isolate));
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Value>(info.This(), fn, property));
}

/* static */ void DynamicObject::Set(Local<String> property, Local<Value> value, const PropertyCallbackInfo<Value>& info)
{
	Prologue(info);
	Prop();
	// check super-class members
	if(obj->has_super_ && super->Has(property)){
		super->Set(property, value);
		info.GetReturnValue().Set(value);
		return;
	}
	// check internal properties
	switch(*prop++){
		case 'g': SET_HANDLER_PROP(get, value); break;
		case 's': SET_HANDLER_PROP(set, value); break;
		case 'q': SET_HANDLER_PROP(query, value); break;
		case 'd': SET_HANDLER_PROP(delete, value); break;
		case 'e': SET_HANDLER_PROP(enumerate, value); break;
	}
	CHECK(set){
		info.GetReturnValue().Set(Undefined(isolate));
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Value>(info.This(), fn, property, value));
}

/* static */ void DynamicObject::Query(Local<String> property, const PropertyCallbackInfo<Integer>& info)
{
	Prologue(info);
	Prop();
	// check internal properties
	if(IS_INTERNAL_PROP){
		// internal properties always return static value
		info.GetReturnValue().Set(DontEnum | DontDelete);
		return;
	}
	CHECK(query){
		info.GetReturnValue().Set(None);
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Integer>(info.This(), fn, property));
}

/* static */ void DynamicObject::Delete(Local<String> property, const PropertyCallbackInfo<Boolean>& info)
{
	Prologue(info);
	Prop();
	// check internal properties
	if(IS_INTERNAL_PROP){
		// internal properties always return static value
		info.GetReturnValue().Set(false);
		return;
	}
	CHECK(delete){
		info.GetReturnValue().Set(false);
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Boolean>(info.This(), fn, property));
}

/* static */ void DynamicObject::Enumerate(const PropertyCallbackInfo<Array>& info)
{
	Prologue(info);
	CHECK(enumerate){
		info.GetReturnValue().Set(Array::New(isolate));
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Array>(info.This(), fn));
}


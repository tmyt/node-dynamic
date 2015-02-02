#include<node.h>
#include<cstring>

#include"dynamic.h"

using namespace v8;

#define ISOLATE(x,y) Isolate* x = Isolate::GetCurrent()
#define CASTFN(x, y) Local<Function> x = Local<Function>::Cast(y)

template<typename T>
Local<T> CallHandler(Local<Value> thiz, Local<Value> fn_)
{
	CASTFN(fn, fn_);
	Local<Value> argv[] = { };
	Local<Value> ret = fn->Call(thiz, 0, argv);
	return Handle<T>::Cast(ret);
}

template<typename T>
Local<T> CallHandler(Local<Value> thiz, Local<Value> fn_, const Local<Value>& arg1)
{
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1 };
	Local<Value> ret = fn->Call(thiz, 1, argv);
	return Handle<T>::Cast(ret);
}

template<typename T>
Local<T> CallHandler(Local<Value> thiz, Local<Value> fn_, const Local<Value>& arg1, const Local<Value>& arg2)
{
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1, arg2 };
	Local<Value> ret = fn->Call(thiz, 2, argv);
	return Handle<T>::Cast(ret);
}

template<>
Local<Boolean> CallHandler(Local<Value> thiz, Local<Value> fn_, const Local<Value>& arg1)
{
	ISOLATE(isolate, context);
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1 };
	Local<Value> ret = fn->Call(thiz, 1, argv);
	return Boolean::New(isolate, ret->BooleanValue());
}

#define NC(x) !strcmp(*u8prop, #x)
#define GET_HANDLER_PROP(name) \
	if(NC(name)) { info.GetReturnValue().Set(Local<Value>::New(isolate, obj->name##_)); return; }
#define SET_HANDLER_PROP(name, value) \
	if(NC(name)) { obj->name##_.Reset(isolate, value); info.GetReturnValue().Set(value); return; }
#define IS_INTERNAL_PROP \
	NC(get)||NC(set)||NC(query)||NC(delete)||NC(enumerate)
#define GET_HANDLER_PROP_STATIC(name, value) \
	if(NC(name)) { info.GetReturnValue().Set(value); return; }

#define SafeCast(_Ty, _Val) 

#define CHECK(x) \
	LocalFunc(x); \
	if( (fn->IsUndefined() || !fn->IsFunction()) && \
	    (!super->IsObject() || !(fn = super->ToObject()->Get(String::NewFromUtf8(isolate, #x)))->IsFunction()) )
#define HANDLE(x) info.GetReturnValue().Set(x)

#define Undef(x) x##_.Reset(isolate, Undefined(isolate))
#define LocalValue(x, y) Local<Value> x = Local<Value>::New(isolate, y)
#define LocalFunc(x) LocalValue(fn, obj->x##_)

#define Prologue(x) \
	Isolate* isolate = Isolate::GetCurrent();\
	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(x.This());\
	LocalValue(super, obj->super_)

Persistent<Function> DynamicObject::constructor;

DynamicObject::DynamicObject()
{
	Isolate* isolate = Isolate::GetCurrent();
	Undef(super);
	Undef(get);
	Undef(set);
	Undef(query);
	Undef(delete);
	Undef(enumerate);
}

DynamicObject::~DynamicObject() { }

/* static */ void DynamicObject::Init(Handle<Object> exports, Handle<Object> module)
{
	Isolate* isolate = Isolate::GetCurrent();

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
		obj->super_.Reset(isolate, args[0]);
	}
	obj->Wrap(args.This());
	args.GetReturnValue().Set(args.This());
}

/* static */ void DynamicObject::Get(Local<String> property, const PropertyCallbackInfo<Value>& info)
{
	Prologue(info);
	String::Utf8Value u8prop(property);
	// check super-class members
	if(super->IsObject()){
		Local<Object> s = super->ToObject();
		if(s->Has(property)){
			info.GetReturnValue().Set(s->Get(property));
			return;
		}
	}
	// check internal properties
	GET_HANDLER_PROP_STATIC(valueOf, info.This());
	GET_HANDLER_PROP_STATIC(inspect, Undefined(isolate));
	GET_HANDLER_PROP(get);
	GET_HANDLER_PROP(set);
	GET_HANDLER_PROP(query);
	GET_HANDLER_PROP(delete);
	GET_HANDLER_PROP(enumerate);
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
	String::Utf8Value u8prop(property);
	// check super-class members
	if(super->IsObject()){
		Local<Object> s = super->ToObject();
		if(s->Has(property)){
			s->Set(property, value);
			info.GetReturnValue().Set(value);
			return;
		}
	}
	// check internal properties
	SET_HANDLER_PROP(get, value);
	SET_HANDLER_PROP(set, value);
	SET_HANDLER_PROP(query, value);
	SET_HANDLER_PROP(delete, value);
	SET_HANDLER_PROP(enumerate, value);
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
	String::Utf8Value u8prop(property);
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
	String::Utf8Value u8prop(property);
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


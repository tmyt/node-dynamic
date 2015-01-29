#include<node.h>
#include<cstring>

#include"dynamic.h"

using namespace v8;

#define ISOLATE(x,y) Isolate* x = Isolate::GetCurrent(); Local<Context> y = Context::New(x)
#define CASTFN(x, y) Local<Function> x = Local<Function>::Cast(y)

template<typename T>
Local<T> CallHandler(Local<Value> fn_)
{
	ISOLATE(isolate, context);
	CASTFN(fn, fn_);
	Local<Value> argv[] = { };
	Local<Value> ret = fn->Call(context->Global(), 0, argv);
	return Handle<T>::Cast(ret);
}

template<typename T>
Local<T> CallHandler(Local<Value> fn_, const Local<Value>& arg1)
{
	ISOLATE(isolate, context);
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1 };
	Local<Value> ret = fn->Call(context->Global(), 1, argv);
	return Handle<T>::Cast(ret);
}

template<typename T>
Local<T> CallHandler(Local<Value> fn_, const Local<Value>& arg1, const Local<Value>& arg2)
{
	ISOLATE(isolate, context);
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1, arg2 };
	Local<Value> ret = fn->Call(context->Global(), 2, argv);
	return Handle<T>::Cast(ret);
}

template<>
Local<Value> CallHandler(Local<Value> fn_)
{
	ISOLATE(isolate, context);
	CASTFN(fn, fn_);
	Local<Value> argv[] = { };
	Local<Value> ret = fn->Call(context->Global(), 0, argv);
	return ret;
}

template<>
Local<Value> CallHandler(Local<Value> fn_, const Local<Value>& arg1)
{
	ISOLATE(isolate, context);
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1 };
	Local<Value> ret = fn->Call(context->Global(), 1, argv);
	return ret;
}

template<>
Local<Value> CallHandler(Local<Value> fn_, const Local<Value>& arg1, const Local<Value>& arg2)
{
	ISOLATE(isolate, context);
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1, arg2 };
	Local<Value> ret = fn->Call(context->Global(), 2, argv);
	return ret;
}

template<>
Local<Boolean> CallHandler(Local<Value> fn_, const Local<Value>& arg1)
{
	ISOLATE(isolate, context);
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1 };
	Local<Value> ret = fn->Call(context->Global(), 1, argv);
	return Boolean::New(isolate, ret->BooleanValue());
}

#define GET_HANDLER_PROP(name) \
	if(strcmp(*u8prop, #name) == 0) { info.GetReturnValue().Set(Local<Value>::New(isolate, obj->name##_)); return; }

#define SET_HANDLER_PROP(name, value) \
	if(strcmp(*u8prop, #name) == 0) { obj->name##_.Reset(isolate, value); info.GetReturnValue().Set(value); return; }

#define IS_INTERNAL_PROP \
	property->Equals(s_get) || property->Equals(s_set) \
	|| property->Equals(s_query) || property->Equals(s_delete) \
	|| property->Equals(s_enumerate)

#define CHECK(x) lfn(x); if(fn->IsUndefined() || !fn->IsFunction())
#define HANDLE(x) info.GetReturnValue().Set(x)

#define Undef(x) x##_.Reset(isolate, Undefined(isolate))
#define local(x, y) Local<Value> x = Local<Value>::New(isolate, y)
#define lfn(x) local(fn, obj->x##_)

Persistent<Function> DynamicObject::constructor;
Handle<String> DynamicObject::s_get;
Handle<String> DynamicObject::s_set;
Handle<String> DynamicObject::s_query;
Handle<String> DynamicObject::s_delete;
Handle<String> DynamicObject::s_enumerate;

DynamicObject::DynamicObject()
{
	Isolate* isolate = Isolate::GetCurrent();
	Undef(get);
	Undef(set);
	Undef(query);
	Undef(delete);
	Undef(enumerate);
}

DynamicObject::~DynamicObject()
{
}

void DynamicObject::Init(Handle<Object> exports, Handle<Object> module)
{
	Isolate* isolate = Isolate::GetCurrent();

	// Property names
	s_get = String::NewFromUtf8(isolate, "get");
	s_set = String::NewFromUtf8(isolate, "set");
	s_query = String::NewFromUtf8(isolate, "query");
	s_delete = String::NewFromUtf8(isolate, "delete");
	s_enumerate = String::NewFromUtf8(isolate, "enumerate");

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

void DynamicObject::New(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if(!args.IsConstructCall()) return;

	// Construct object
	DynamicObject* obj = new DynamicObject();
	obj->Wrap(args.This());
	args.GetReturnValue().Set(args.This());
}

void DynamicObject::Get(Local<String> property, const PropertyCallbackInfo<Value>& info)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(info.This());

	// check internal properties
	String::Utf8Value u8prop(property);
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
	HANDLE(CallHandler<Value>(fn, property));
}

void DynamicObject::Set(Local<String> property, Local<Value> value, const PropertyCallbackInfo<Value>& info)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(info.This());

	// check internal properties
	String::Utf8Value u8prop(property);
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
	HANDLE(CallHandler<Value>(fn, property, value));
}

void DynamicObject::Query(Local<String> property, const PropertyCallbackInfo<Integer>& info)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	// check internal properties
	if(IS_INTERNAL_PROP){
		// internal properties always return static value
		info.GetReturnValue().Set(DontEnum | DontDelete);
		return;
	}

	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(info.This());
	CHECK(query){
		info.GetReturnValue().Set(None);
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Integer>(fn, property));
}

void DynamicObject::Delete(Local<String> property, const PropertyCallbackInfo<Boolean>& info)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	// check internal properties
	if(IS_INTERNAL_PROP){
		// internal properties always return static value
		info.GetReturnValue().Set(false);
		return;
	}

	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(info.This());
	CHECK(delete){
		info.GetReturnValue().Set(false);
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Boolean>(fn, property));
}

void DynamicObject::Enumerate(const PropertyCallbackInfo<Array>& info)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(info.This());
	CHECK(enumerate){
		info.GetReturnValue().Set(Array::New(isolate));
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Array>(fn));
}


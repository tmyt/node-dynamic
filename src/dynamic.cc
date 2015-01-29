#include<iostream>
#include<node.h>

#include"dynamic.h"

using namespace v8;

#define ISOLATE(x,y) Isolate* x = Isolate::GetCurrent(); Local<Context> y = Context::New(x);
#define CASTFN(x, y) Local<Function> x = Local<Function>::Cast(y);

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
Local<v8::Value> CallHandler(Local<Value> fn_)
{
	ISOLATE(isolate, context);
	CASTFN(fn, fn_);
	Local<Value> argv[] = { };
	Local<Value> ret = fn->Call(context->Global(), 0, argv);
	return ret;
}

template<>
Local<v8::Value> CallHandler(Local<Value> fn_, const Local<Value>& arg1)
{
	ISOLATE(isolate, context);
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1 };
	Local<Value> ret = fn->Call(context->Global(), 1, argv);
	return ret;
}

template<>
Local<v8::Value> CallHandler(Local<Value> fn_, const Local<Value>& arg1, const Local<Value>& arg2)
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
	if(property->Equals(s_##name)) { info.GetReturnValue().Set(obj->name##_); return; }

#define SET_HANDLER_PROP(name, value) \
	if(property->Equals(s_##name)) { obj->name##_ = value; info.GetReturnValue().Set(value); return; }

#define IS_INTERNAL_PROP \
	property->Equals(s_get) || property->Equals(s_set) \
	|| property->Equals(s_query) || property->Equals(s_delete) \
	|| property->Equals(s_enumerate)


Persistent<Function> DynamicObject::constructor;
Handle<String> DynamicObject::s_get;
Handle<String> DynamicObject::s_set;
Handle<String> DynamicObject::s_query;
Handle<String> DynamicObject::s_delete;
Handle<String> DynamicObject::s_enumerate;

DynamicObject::DynamicObject()
{
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

void DynamicObject::Get(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(info.This());

	// check internal properties
	GET_HANDLER_PROP(get);
	GET_HANDLER_PROP(set);
	GET_HANDLER_PROP(query);
	GET_HANDLER_PROP(delete);
	GET_HANDLER_PROP(enumerate);

	if(obj->get_->IsUndefined() || !obj->get_->IsFunction()){
		info.GetReturnValue().Set(Undefined(isolate));
		return;
	}
	// call javascript handler
	CallHandler<Value>(obj->get_, property);
}

void DynamicObject::Set(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(info.This());

	// check internal properties
	SET_HANDLER_PROP(get, value);
	SET_HANDLER_PROP(set, value);
	SET_HANDLER_PROP(query, value);
	SET_HANDLER_PROP(delete, value);
	SET_HANDLER_PROP(enumerate, value);

	if(obj->set_->IsUndefined() || !obj->set_->IsFunction()){
		info.GetReturnValue().Set(Undefined(isolate));
		return;
	}
	// call javascript handler
	CallHandler<Value>(obj->set_, property, value);
}

void DynamicObject::Query(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Integer>& info)
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
	if(obj->query_->IsUndefined() || !obj->query_->IsFunction()){
		info.GetReturnValue().Set(None);
		return;
	}
	// call javascript handler
	CallHandler<Integer>(obj->query_, property);
}

void DynamicObject::Delete(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Boolean>& info)
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
	if(obj->delete_->IsUndefined() || !obj->delete_->IsFunction()){
		info.GetReturnValue().Set(false);
		return;
	}
	// call javascript handler
	CallHandler<Boolean>(obj->delete_, property);
}

void DynamicObject::Enumerate(const v8::PropertyCallbackInfo<v8::Array>& info)
{
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(info.This());
	if(obj->enumerate_->IsUndefined() || !obj->enumerate_->IsFunction()){
		info.GetReturnValue().Set(Array::New(isolate));
		return;
	}
	// call javascript handler
	CallHandler<Array>(obj->enumerate_);
}


#include<node.h>

#include"dynamic.h"

#define CALL_JSHANDLER_0(Ty_, fn_) \
{ \
	Local<Context> context = Context::New(isolate); \
	Local<Function> fn = Local<Function>::Cast(obj->fn_); \
	const unsigned argc = 0; \
	Local<Value> argv[argc] = { }; \
	info.GetReturnValue().Set(Handle<Ty_>::Cast(fn->Call(context->Global(), argc, argv))); \
}

#define CALL_JSHANDLER_1(Ty_, fn_, arg_) \
{ \
	Local<Context> context = Context::New(isolate); \
	Local<Function> fn = Local<Function>::Cast(obj->fn_); \
	const unsigned argc = 1; \
	Local<Value> argv[argc] = { arg_ }; \
	info.GetReturnValue().Set(Handle<Ty_>::Cast(fn->Call(context->Global(), argc, argv))); \
}

#define CALL_JSHANDLER_2(fn_, arg1_, arg2_) \
{ \
	Local<Context> context = Context::New(isolate); \
	Local<Function> fn = Local<Function>::Cast(obj->fn_); \
	const unsigned argc = 2; \
	Local<Value> argv[argc] = { arg1_, arg2_ }; \
	info.GetReturnValue().Set(fn->Call(context->Global(), argc, argv)); \
}

#define GET_HANDLER_PROP(name) \
	if(property->Equals(s_##name)) { info.GetReturnValue().Set(obj->name##_); return; }

#define SET_HANDLER_PROP(name, value) \
	if(property->Equals(s_##name)) { obj->name##_ = value; info.GetReturnValue().Set(value); return; }

#define IS_INTERNAL_PROP \
	property->Equals(s_get) || property->Equals(s_set) \
	|| property->Equals(s_query) || property->Equals(s_delete) \
	|| property->Equals(s_enumerate)

using namespace v8;

Persistent<Function> DynamicObject::constructor;
Handle<String> s_get;
Handle<String> s_set;
Handle<String> s_query;
Handle<String> s_delete;
Handle<String> s_enumerate;

DynamicObject::DynamicObject()
{
}

DynamicObject::~DynamicObject()
{
}

void DynamicObject::Init(Handle<Object> exports)
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
	exports->Set(String::NewFromUtf8(isolate, "DynamicObject"),
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
	CALL_JSHANDLER_1(Value, get_, property);
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
	CALL_JSHANDLER_2(set_, property, value);
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
	CALL_JSHANDLER_1(Integer, query_, property);
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
	CALL_JSHANDLER_1(Boolean, delete_, property);
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
	CALL_JSHANDLER_0(Array, enumerate_);
}


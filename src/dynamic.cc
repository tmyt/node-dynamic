#include<node.h>
#include<cstring>

#include<nan.h>

#include"dynamic.h"

using namespace v8;

#define CASTFN(x, y) Local<Function> x = Local<Function>::Cast(y)

template<typename T>
Local<T> Cast(Local<Value> value) { return value.As<T>(); }
template<>
Local<Value> Cast<Value>(Local<Value> value) { return value; }
template<>
Local<Boolean> Cast<Boolean>(Local<Value> value) { return Nan::New<Boolean>(Nan::To<bool>(value).FromJust()); }

template<typename T>
Local<T> CallHandler(const Local<Object>& thiz, const Local<Value>& fn_)
{
	CASTFN(fn, fn_);
	Local<Value> ret = fn->Call(thiz, 0, 0);
	return Cast<T>(ret);
}

template<typename T, typename U>
Local<T> CallHandler(const Local<Object>& thiz, const Local<Value>& fn_, const U& arg1)
{
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1 };
	Local<Value> ret = fn->Call(thiz, 1, argv);
	return Cast<T>(ret);
}

template<typename T, typename U, typename V>
Local<T> CallHandler(const Local<Object>& thiz, const Local<Value>& fn_, const U& arg1, const V& arg2)
{
	CASTFN(fn, fn_);
	Local<Value> argv[] = { arg1, arg2 };
	Local<Value> ret = fn->Call(thiz, 2, argv);
	return Cast<T>(ret);
}

#define NC_(x, xx) !strcmp(x, (#xx)+1)
#define NC(x) NC_(prop+1, x)
#define NC2(x, xx) (*prop == x && NC(xx))
#define GET_HANDLER_PROP(name) \
	if(NC_(prop, name)) { info.GetReturnValue().Set(Nan::New(obj->name##_)); return; }
#define GET_HANDLER_PROP_STATIC(name, value) \
	if(NC_(prop, name)) { info.GetReturnValue().Set(value); return; }
#define SET_HANDLER_PROP(name, value) \
	if(NC_(prop, name)) { obj->name##_.Reset(value); info.GetReturnValue().Set(value); return; }
#define IS_INTERNAL_PROP \
	NC2('g', get) || NC2('s', set) || NC2('q', query) || NC2('d', delete) || NC2('e', enumerate)
#define PRECHECK() if(*prop == 'g' || *prop == 's' || *prop == 'd' || *prop == 'e' || *prop == 'q')

#define CHECK(x) \
	LocalFunc(x); \
	if( !fn->IsFunction() && \
	    (!obj->has_super_ || !(fn = super->Get(Nan::New(property_names_[K_##x])))->IsFunction()) )
#define HANDLE(x) info.GetReturnValue().Set(x)

#define Undef(x) x##_.Reset(Nan::Undefined())
#define LocalValue(x, y) Local<Value> x = Nan::New(y)
#define LocalFunc(x) LocalValue(fn, obj->x##_)

#define Prologue(x) \
	Nan::HandleScope scope; \
	DynamicObject* obj = ObjectWrap::Unwrap<DynamicObject>(x.This());\
	Local<Object> super = Nan::New(obj->super_)
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

Nan::Persistent<String> DynamicObject::property_names_[5];
Nan::Persistent<Function> DynamicObject::constructor;

DynamicObject::DynamicObject() : has_super_(false)
{
	Nan::HandleScope scope;
	Each(Undef);
}

DynamicObject::~DynamicObject() { }

/* static */ void DynamicObject::Init(Handle<Object> exports, Handle<Object> module)
{
	Nan::HandleScope scope;
#define n(x) property_names_[K_##x].Reset(Nan::New<v8::String>(#x).ToLocalChecked())
	Each(n);
#undef n
	// Prepare constructor template
	Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("DynamicObject").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	Nan::SetNamedPropertyHandler(tpl->InstanceTemplate(), Get, Set, Query, Delete, Enumerate);

	// Prototype
	constructor.Reset(tpl->GetFunction());
	module->Set(Nan::New("exports").ToLocalChecked(), tpl->GetFunction());
}

/* static */ NAN_METHOD(DynamicObject::New)
{
	Nan::HandleScope scope;
	if(!info.IsConstructCall()) return;
	// Construct object
	DynamicObject* obj = new DynamicObject();
	if(info[0]->IsObject()){
		obj->has_super_ = true;
		obj->super_.Reset(info[0]->ToObject());
	}
	obj->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

/* static */ NAN_PROPERTY_GETTER(DynamicObject::Get)
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
		case 'v': GET_HANDLER_PROP_STATIC(valueOf, Nan::New<FunctionTemplate>(ValueOf)->GetFunction()); break;
		case 'i': GET_HANDLER_PROP_STATIC(inspect, Nan::Undefined()); break;
	}
	CHECK(get){
		info.GetReturnValue().Set(Nan::Undefined());
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Value>(info.This(), fn, property));
}

/* static */ NAN_PROPERTY_SETTER(DynamicObject::Set)
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
		info.GetReturnValue().Set(Nan::Undefined());
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Value>(info.This(), fn, property, value));
}

/* static */ NAN_PROPERTY_QUERY(DynamicObject::Query)
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

/* static */ NAN_PROPERTY_DELETER(DynamicObject::Delete)
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

/* static */ NAN_PROPERTY_ENUMERATOR(DynamicObject::Enumerate)
{
	Prologue(info);
	CHECK(enumerate){
		info.GetReturnValue().Set(Nan::New<v8::Array>());
		return;
	}
	// call javascript handler
	HANDLE(CallHandler<Array>(info.This(), fn));
}

/* static */ NAN_METHOD(DynamicObject::ValueOf)
{
	info.GetReturnValue().Set(info.This());
}


#ifndef SRC_DYNAMIC_H
#define SRC_DYNAMIC_H

#include<node.h>
#include<node_object_wrap.h>

#include<nan.h>

class DynamicObject : public node::ObjectWrap
{
public:
	static void Init(v8::Handle<v8::Object> exports, v8::Handle<v8::Object> module);

private:
	explicit DynamicObject();
	~DynamicObject();

	static Nan::Persistent<v8::Function> constructor;

	/// Prototype methods
	static NAN_METHOD(New);

	/// Object handlers
	static NAN_PROPERTY_GETTER(Get);
	static NAN_PROPERTY_SETTER(Set);
	static NAN_PROPERTY_QUERY(Query);
	static NAN_PROPERTY_DELETER(Delete);
	static NAN_PROPERTY_ENUMERATOR(Enumerate);
	static NAN_METHOD(ValueOf);

	/// Static fields
	static Nan::Persistent<v8::String> property_names_[5];

	/// Instance fields
	Nan::Persistent<v8::Object> super_;
	Nan::Persistent<v8::Value> get_;
	Nan::Persistent<v8::Value> set_;
	Nan::Persistent<v8::Value> query_;
	Nan::Persistent<v8::Value> delete_;
	Nan::Persistent<v8::Value> enumerate_;
	bool has_super_;
};

#endif


#ifndef SRC_DYNAMIC_H
#define SRC_DYNAMIC_H

#include<node.h>
#include<node_object_wrap.h>

class DynamicObject : public node::ObjectWrap
{
public:
	static void Init(v8::Handle<v8::Object> exports, v8::Handle<v8::Object> module);

private:
	explicit DynamicObject();
	~DynamicObject();

	static v8::Persistent<v8::Function> constructor;
	static v8::Handle<v8::String> s_get;
	static v8::Handle<v8::String> s_set;
	static v8::Handle<v8::String> s_query;
	static v8::Handle<v8::String> s_delete;
	static v8::Handle<v8::String> s_enumerate;

	/// Prototype methods
	static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

	/// Object handlers
	static void Get(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void Set(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void Query(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Integer>& info);
	static void Delete(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Boolean>& info);
	static void Enumerate(const v8::PropertyCallbackInfo<v8::Array>& info);

	/// Instance fields
	v8::Persistent<v8::Value> get_;
	v8::Persistent<v8::Value> set_;
	v8::Persistent<v8::Value> query_;
	v8::Persistent<v8::Value> delete_;
	v8::Persistent<v8::Value> enumerate_;
};

#endif


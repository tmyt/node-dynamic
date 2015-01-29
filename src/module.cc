#include<node.h>

#include"dynamic.h"

using namespace v8;

void init(Handle<Object> exports, Handle<Object> module)
{
	DynamicObject::Init(exports, module);
}

NODE_MODULE(dynamic, init)

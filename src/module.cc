#include<node.h>

#include<nan.h>

#include"dynamic.h"

using namespace v8;

NAN_MODULE_INIT(init)
{
	DynamicObject::Init(target);
}

NODE_MODULE(dynamic, init)

// Copyright GNC Project. All Rights Reserved.

#include "DObject.h"

int64 UDObject::NextInstanceId = 0;

UDObject::UDObject()
{
	InstanceId = ++NextInstanceId;
}

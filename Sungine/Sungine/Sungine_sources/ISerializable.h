#pragma once

#include "jsoncpp/json/json.h"
#include "SerializeUtils.h"

class ISerializable {
public:
	virtual void save(Json::Value& entityRoot) const = 0;
	virtual void load(const Json::Value& entityRoot) = 0;

};
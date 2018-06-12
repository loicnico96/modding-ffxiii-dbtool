
#include <algorithm>

#include "include/AttributeValue.hpp"

using namespace dbtool; 
	
AttributeValue::AttributeValue()
	:m_value(), m_type(AttributeType::Unsigned) {
		this->m_value.u = 0; 
	}
AttributeValue::AttributeValue(bool b)
	:m_value(), m_type(AttributeType::Boolean) {
		this->m_value.b = b; 
	}
AttributeValue::AttributeValue(unsigned u)
	:m_value(), m_type(AttributeType::Unsigned) {
		this->m_value.u = u; 
	}
AttributeValue::AttributeValue(int i)
	:m_value(), m_type(AttributeType::Signed) {
		this->m_value.i = i; 
	}
AttributeValue::AttributeValue(float f)
	:m_value(), m_type(AttributeType::Float) {
		this->m_value.f = f; 
	}
AttributeValue::AttributeValue(const std::string& s)
	:m_value(), m_type(AttributeType::String) {
		this->m_value.s = new std::string(s); 
	}
AttributeValue::AttributeValue(const AttributeValue& value)
	:m_value(), m_type(value.m_type) {
		if (this->m_type == AttributeType::String) {
			this->m_value.s = new std::string(*value.m_value.s); 
		} else {
			this->m_value = value.m_value; 
		}
	}
AttributeValue::AttributeValue(AttributeValue&& value)
	:m_value(value.m_value), m_type(value.m_type) {
		value.m_type = AttributeType::Unsigned; 
	}
AttributeValue::~AttributeValue() {
	if (this->m_type == AttributeType::String) {
		delete this->m_value.s;
	}
}

AttributeValue& AttributeValue::operator = (const AttributeValue& value) {
	if (this != &value) {
		if (this->m_type == AttributeType::String) {
			delete this->m_value.s; 
		}
		this->m_type = value.m_type; 
		if (this->m_type == AttributeType::String) {
			this->m_value.s = new std::string(*value.m_value.s); 
		} else {
			this->m_value = value.m_value; 
		}
	}
	return *this; 
}

AttributeValue& AttributeValue::operator = (AttributeValue&& value) {
	std::swap(this->m_type, value.m_type); 
	std::swap(this->m_value, value.m_value); 
	return *this; 
}

AttributeType AttributeValue::getType() const {
	return this->m_type; 
}

bool AttributeValue::getBoolean() const {
	if (this->m_type != AttributeType::Boolean) {
		throw std::logic_error("Attribute value is not a Boolean."); 
	}
	return this->m_value.b; 
}

unsigned AttributeValue::getUnsigned() const {
	if (this->m_type != AttributeType::Unsigned) {
		throw std::logic_error("Attribute value is not a Unsigned."); 
	}
	return this->m_value.u; 
}

int AttributeValue::getSigned() const {
	if (this->m_type != AttributeType::Signed) {
		throw std::logic_error("Attribute value is not a Signed."); 
	}
	return this->m_value.i; 
}

float AttributeValue::getFloat() const {
	if (this->m_type != AttributeType::Float) {
		throw std::logic_error("Attribute value is not a Float."); 
	}
	return this->m_value.f; 
}

std::string AttributeValue::getString() const {
	if (this->m_type != AttributeType::String) {
		throw std::logic_error("Attribute value is not a String."); 
	}
	return *this->m_value.s; 
}

void AttributeValue::setBoolean(bool b) {
	if (this->m_type == AttributeType::String) {
		delete this->m_value.s;
	}
	this->m_type = AttributeType::Boolean; 
	this->m_value.b = b; 
}

void AttributeValue::setUnsigned(unsigned u) {
	if (this->m_type == AttributeType::String) {
		delete this->m_value.s;
	}
	this->m_type = AttributeType::Unsigned; 
	this->m_value.u = u; 
}

void AttributeValue::setSigned(int i) {
	if (this->m_type == AttributeType::String) {
		delete this->m_value.s;
	}
	this->m_type = AttributeType::Signed; 
	this->m_value.i = i; 
}

void AttributeValue::setFloat(float f) {
	if (this->m_type == AttributeType::String) {
		delete this->m_value.s;
	}
	this->m_type = AttributeType::Float; 
	this->m_value.f = f; 
}

void AttributeValue::setString(const std::string& s) {
	if (this->m_type == AttributeType::String) {
		delete this->m_value.s;
	}
	this->m_type = AttributeType::String; 
	this->m_value.s = new std::string(s); 
}

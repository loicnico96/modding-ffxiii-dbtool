
#include <cstring>

#include "include/Enum.hpp"
#include "tinyxml2/tinyxml2.h"

using namespace dbtool; 

EnumMap Enum::s_enums; 

Enum::Enum()
	:m_values(), m_type(AttributeType::Unsigned), m_name(""), m_strict(false) {}
Enum::~Enum() {} 

AttributeType Enum::getType() const {
	return this->m_type; 
}

bool Enum::getStrict() const {
	return this->m_strict; 
}

unsigned Enum::getUnsigned(const std::string& name) const {
	AttributeValueMap::const_iterator it = this->m_values.find(name); 
	if (it == this->m_values.end()) {
		throw std::logic_error(strfmt("Key \"%s\" is not defined in enumeration %s.", name.c_str(), this->m_name.c_str())); 
	}
	return it->second.getUnsigned(); 
}

int Enum::getSigned(const std::string& name) const {
	AttributeValueMap::const_iterator it = this->m_values.find(name); 
	if (it == this->m_values.end()) {
		throw std::logic_error(strfmt("Key \"%s\" is not defined in enumeration %s.", name.c_str(), this->m_name.c_str())); 
	}
	return it->second.getSigned(); 
}

float Enum::getFloat(const std::string& name) const {
	AttributeValueMap::const_iterator it = this->m_values.find(name); 
	if (it == this->m_values.end()) {
		throw std::logic_error(strfmt("Key \"%s\" is not defined in enumeration %s.", name.c_str(), this->m_name.c_str())); 
	}
	return it->second.getFloat(); 
}

std::string Enum::getString(const std::string& name) const {
	AttributeValueMap::const_iterator it = this->m_values.find(name); 
	if (it == this->m_values.end()) {
		throw std::logic_error(strfmt("Key \"%s\" is not defined in enumeration %s.", name.c_str(), this->m_name.c_str())); 
	}
	return it->second.getString(); 
}

std::string Enum::getName(unsigned u) const {
	AttributeValueMap::const_iterator it; 
	for (it = this->m_values.begin() ; it != this->m_values.end() ; it++) {
		if (it->second.getUnsigned() == u)
			return it->first; 
	}
	throw std::logic_error(strfmt("Value %u is not defined in enumeration %s.", u, this->m_name.c_str())); 
}

std::string Enum::getName(int i) const {
	AttributeValueMap::const_iterator it; 
	for (it = this->m_values.begin() ; it != this->m_values.end() ; it++) {
		if (it->second.getSigned() == i)
			return it->first; 
	}
	throw std::logic_error(strfmt("Value %d is not defined in enumeration %s.", i, this->m_name.c_str())); 
}

std::string Enum::getName(float f) const {
	AttributeValueMap::const_iterator it; 
	for (it = this->m_values.begin() ; it != this->m_values.end() ; it++) {
		if (it->second.getFloat() == f)
			return it->first; 
	}
	throw std::logic_error(strfmt("Value %f is not defined in enumeration %s.", f, this->m_name.c_str())); 
}

std::string Enum::getName(const std::string& s) const {
	AttributeValueMap::const_iterator it; 
	for (it = this->m_values.begin() ; it != this->m_values.end() ; it++) {
		if (it->second.getString() == s)
			return it->first; 
	}
	throw std::logic_error(strfmt("Value \"%s\" is not defined in enumeration %s.", s.c_str(), this->m_name.c_str())); 
}

Enum* Enum::GetEnum(const std::string& name) {
	EnumMap::iterator it = Enum::s_enums.find(name); 
	if (it == Enum::s_enums.end()) {
		Print("Loading enumeration %s...", name.c_str()); 
		PrintStart(); 
		
		tinyxml2::XMLDocument xml; 
		std::string filename = strfmt("xml/enum/%s.xml", name.c_str()); 
		xml.LoadFile(filename.c_str()); 
		if (xml.Error()) {
			Print("Couldn't open XML file \"%s\".", filename.c_str()); 
			PrintAbort(); 
			return nullptr; 
		}
		
		tinyxml2::XMLElement* xmlenum = xml.FirstChildElement("enum"); 
		if (xmlenum == nullptr) {
			Print("Couldn't find \"%s\" element in XML file \"%s\".", "enum", filename.c_str()); 
			PrintAbort(); 
			return nullptr; 
		}
		
		Enum& enumInfo = Enum::s_enums[name]; 
		enumInfo.m_name = name; 
		
		const char* enumType = xmlenum->Attribute("type"); 
		if (enumType == nullptr) {
			Print("Missing \"%s\" attribute.", "type"); 
		} else if (strcmp(enumType, "Unsigned") == 0) {
			enumInfo.m_type = AttributeType::Unsigned; 
		} else if (strcmp(enumType, "Signed") == 0) {
			enumInfo.m_type = AttributeType::Signed; 
		} else if (strcmp(enumType, "Float") == 0) {
			enumInfo.m_type = AttributeType::Float; 
		} else if (strcmp(enumType, "String") == 0) {
			enumInfo.m_type = AttributeType::String; 
		} else {
			Print("Unexpected \"%s\" attribute value (\"%s\").", "type", enumType); 
		}
		
		const char* enumStrict = xmlenum->Attribute("strict"); 
		if ((enumStrict != nullptr) && (strcmp(enumStrict, "true") == 0)) {
			enumInfo.m_strict = true; 
		}
		
		bool enumHexa = false; 
		const char* enumFormat = xmlenum->Attribute("format"); 
		if ((enumFormat != nullptr) && (strcmp(enumFormat, "hexa") == 0)) {
			enumHexa = true; 
		}
		
		const char* enumExtends = xmlenum->Attribute("extends"); 
		if (enumExtends != nullptr) {
			Enum* enumParent = Enum::GetEnum(enumExtends); 
			if (enumParent != nullptr) {
				if (enumInfo.m_type != enumParent->m_type) {
					Print("Enumerations %s and %s do not have the same type.", enumInfo.m_name.c_str(), enumExtends); 
				} else {
					enumInfo.m_values = enumParent->m_values; 
				}
			}
		}
		
		tinyxml2::XMLElement* xmloption; 
		for (xmloption = xmlenum->FirstChildElement("option") ; xmloption != nullptr ; xmloption = xmloption->NextSiblingElement("option")) {
			const char* optionName = xmloption->Attribute("name"); 
			if (optionName == nullptr) {
				Print("Missing option \"%s\" attribute.", "name"); 
				continue; 
			}
			
			const char* optionValue = xmloption->Attribute("value"); 
			if (optionName == nullptr) {
				Print("Missing option \"%s\" attribute.", "value"); 
				continue; 
			}
			
			switch (enumInfo.m_type) {
				case AttributeType::Unsigned: 
					if (enumHexa) {
						try {
							enumInfo.m_values[optionName].setUnsigned(lexical_cast<unsigned>(optionValue, std::hex)); 
						} catch (const std::logic_error& e) {
							Print("Unexpected hexadecimal value (\"%s\").", optionValue); 
						}
					} else {
						try {
							enumInfo.m_values[optionName].setUnsigned(lexical_cast<unsigned>(optionValue)); 
						} catch (const std::logic_error& e) {
							Print("Unexpected unsigned value (\"%s\").", optionValue); 
						}
					}
					break; 
				case AttributeType::Signed: 
					try {
						enumInfo.m_values[optionName].setSigned(lexical_cast<int>(optionValue)); 
					} catch (const std::logic_error& e) {
						Print("Unexpected signed value (\"%s\").", optionValue); 
					}
					break; 
				case AttributeType::Float: 
					try {
						enumInfo.m_values[optionName].setFloat(lexical_cast<float>(optionValue)); 
					} catch (const std::logic_error& e) {
						Print("Unexpected float value (\"%s\").", optionValue); 
					}
					break; 
				case AttributeType::String: 
					enumInfo.m_values[optionName].setString(optionValue); 
			}
		}
		
		PrintDone(); 
		return &enumInfo; 
	} else {
		return &it->second; 
	}
}

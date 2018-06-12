
#include "include/Enum.hpp"
#include "include/Format.hpp"
#include "tinyxml2/tinyxml2.h"

using namespace dbtool; 

Format::Formats Format::s_formats; 

Format::Attribute::Attribute()
	:name(""), type(AttributeType::Unsigned), format(AttributeFormat::Decimal), enumName(""), offset(0), bit(0), size(32), hidden(false) {}
Format::Attribute::Attribute(const Format::Attribute& attribute)
	:name(attribute.name), type(attribute.type), format(attribute.format), enumName(attribute.enumName), offset(attribute.offset), bit(attribute.bit), size(attribute.size), hidden(attribute.hidden) {}
Format::Attribute::~Attribute() {}
Format::Attribute& Format::Attribute::operator = (const Format::Attribute& attribute) {
	if (this != &attribute) {
		this->name		= attribute.name; 
		this->type		= attribute.type; 
		this->format	= attribute.format; 
		this->enumName	= attribute.enumName; 
		this->offset	= attribute.offset; 
		this->bit		= attribute.bit; 
		this->size		= attribute.size; 
		this->hidden	= attribute.hidden; 
	}
}

Format::Format()
	:m_attributes(), m_size(0), m_name("") {}
Format::~Format() {}

unsigned Format::getSize() const {
	return this->m_size; 
}

const Format::Attributes& Format::getAttributes() const {
	return this->m_attributes; 
}

const Format::Attribute& Format::getAttribute(const std::string& name) const {
	Format::Attributes::const_iterator it; 
	for (it = this->m_attributes.begin() ; it != this->m_attributes.end() ; it++) {
		if (it->name == name) {
			return *it; 
		}
	}
	throw std::logic_error(strfmt("Attribute \"%s\" is not defined in format \"%s\".", name.c_str(), this->m_name.c_str())); 
}

Format* Format::GetFormat(const std::string& name) {
	Format::Formats::iterator it = Format::s_formats.find(name); 
	if (it == Format::s_formats.end()) {
		Print("Loading format \"%s\"...", name.c_str()); 
		PrintStart(); 
		
		tinyxml2::XMLDocument xml; 
		std::string filename = strfmt("xml/fmt/%s", name.c_str()); 
		xml.LoadFile(filename.c_str()); 
		if (xml.Error()) {
			Print("Couldn't open XML file \"%s\".", filename.c_str()); 
			PrintAbort(); 
			return nullptr; 
		}
		
		tinyxml2::XMLElement* xmlstruct = xml.FirstChildElement("struct"); 
		if (xmlstruct == nullptr) {
			Print("Couldn't find \"%s\" element in XML file \"%s\".", "struct", filename.c_str()); 
			PrintAbort(); 
			return nullptr; 
		}
		
		Format& formatInfo = Format::s_formats[name]; 
		formatInfo.m_name = name; 
		
		const char* formatSize = xmlstruct->Attribute("size"); 
		if (formatSize == nullptr) {
			Print("Missing \"%s\" attribute.", "size"); 
		} else {
			try {
				formatInfo.m_size = lexical_cast<unsigned>(formatSize); 
			} catch (const std::logic_error& e) {
				Print(e.what()); 
				Print("Unexpected \"%s\" attribute value (\"%s\").", "size", formatSize); 
			}
		}
		
		tinyxml2::XMLElement* xmldata; 
		for (xmldata = xmlstruct->FirstChildElement("data") ; xmldata != nullptr ; xmldata = xmldata->NextSiblingElement("data")) {
			Attribute attribute; 
			
			const char* dataType = xmldata->Attribute("type"); 
			if (dataType == nullptr) {
				Print("Missing data \"%s\" attribute.", "type"); 
				continue; 
			} else if (strcmp(dataType, "Boolean") == 0) {
				attribute.type = AttributeType::Boolean; 
			} else if (strcmp(dataType, "Unsigned") == 0) {
				attribute.type = AttributeType::Unsigned; 
			} else if (strcmp(dataType, "Signed") == 0) {
				attribute.type = AttributeType::Signed; 
			} else if (strcmp(dataType, "Float") == 0) {
				attribute.type = AttributeType::Float; 
			} else if (strcmp(dataType, "String") == 0) {
				attribute.type = AttributeType::String; 
			} else {
				Print("Unexpected \"%s\" attribute value (\"%s\").", "type", dataType); 
			}
			
			const char* dataOffset = xmldata->Attribute("offset"); 
			if (dataOffset == nullptr) {
				Print("Missing data \"%s\" attribute.", "offset"); 
				continue; 
			} else {
				try {
					attribute.offset = lexical_cast<unsigned>(dataOffset, std::hex); 
				} catch (const std::logic_error& e) {
					Print(e.what()); 
					Print("Unexpected \"%s\" attribute value (\"%s\").", "offset", dataOffset); 
					continue; 
				}
			}
			
			if ((attribute.type == AttributeType::Boolean) || (attribute.type == AttributeType::Unsigned) || (attribute.type == AttributeType::Signed)) {
				const char* dataBit	= xmldata->Attribute("bit"); 
				if (dataBit != nullptr) {
					try {
						attribute.bit = lexical_cast<unsigned>(dataBit); 
					} catch (const std::logic_error& e) {
						Print("Unexpected \"%s\" attribute value (\"%s\").", "bit", dataBit); 
						continue; 
					}
				} else if (attribute.type == AttributeType::Boolean) {
					Print("Missing data \"%s\" attribute.", "bit"); 
					continue; 
				}
				if (attribute.bit > 31) {
					attribute.offset += attribute.bit / 32; 
					attribute.bit = attribute.bit % 32; 
				}
			}
			
			if ((attribute.type == AttributeType::Unsigned) || (attribute.type == AttributeType::Signed)) {
				const char* dataSize = xmldata->Attribute("size"); 
				if (dataSize != nullptr) {
					try {
						attribute.size = lexical_cast<unsigned>(dataSize); 
					} catch (const std::logic_error& e) {
						Print("Unexpected \"%s\" attribute value (\"%s\").", "size", dataSize); 
						continue; 
					}
				}
				if (attribute.bit+attribute.size > 32) {
					Print("Unexpected \"%s\" attribute value (%u).", "size", attribute.size); 
					attribute.size = 32 - attribute.bit; 
				}
			} else if (attribute.type == AttributeType::Boolean) {
				attribute.size = 1; 
			}
			
			if ((attribute.type == AttributeType::Unsigned) || (attribute.type == AttributeType::Signed) || (attribute.type == AttributeType::Float)) {
				const char* dataFormat = xmldata->Attribute("format"); 
				if (dataFormat != nullptr) {
					if (strcmp(dataFormat, "hexa") == 0) {
						attribute.format = AttributeFormat::Hexadecimal; 
					} else if (strcmp(dataFormat, "percent") == 0) {
						attribute.format = AttributeFormat::Percentage; 
					} else if (strcmp(dataFormat, "decimal") != 0) {
						Print("Unexpected \"%s\" attribute value (\"%s\").", "format", dataFormat); 
					}
				}
			}
			
			const char* dataName = xmldata->Attribute("name"); 
			if (dataName != nullptr) {
				attribute.name = dataName; 
			} else {
				attribute.name = strfmt("[0x%04X|%02d|%02d]", attribute.offset, attribute.bit, attribute.size); 
				attribute.hidden = true; 
			}
			
			try {
				Print("Duplicate attribute name (\"%s\").", formatInfo.getAttribute(dataName).name.c_str()); 
				continue; 
			} catch (const std::logic_error& e) {}
			
			const char* dataHidden = xmldata->Attribute("hide"); 
			if ((dataHidden != nullptr) && (strcmp(dataHidden, "true") == 0)) {
				attribute.hidden = true; 
			}
			
			if (attribute.type != AttributeType::Boolean) {
				const char* dataEnumName = xmldata->Attribute("enum"); 
				if (dataEnumName != nullptr) {
					Enum* dataEnum = Enum::GetEnum(dataEnumName); 
					if (dataEnum != nullptr) {
						if (dataEnum->getType() == attribute.type) {
							attribute.enumName = dataEnumName; 
						} else {
							Print("Data type does not match with enumeration %s.", dataEnumName); 
						}
					}
				}
			}
			
			formatInfo.m_attributes.push_back(attribute); 
		}
		
		PrintDone(); 
		return &formatInfo; 
	} else {
		return &it->second; 
	}
}

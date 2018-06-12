
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <regex>

#include "include/Enum.hpp"
#include "include/Format.hpp"
#include "include/WPDFile.hpp"

using namespace dbtool; 
	
WPDFile::WPDFile ()
	:m_entryList(), m_modified(false), m_fileAttributes() {}
WPDFile::~WPDFile () {}

bool WPDFile::load (const std::string& filename) {
	Print("Loading WPD file \"%s\"...", filename.c_str()); 
	PrintStart(); 
	this->m_entryList.clear(); 
	this->m_modified = false; 
	
	if (GetFileAttributesEx(filename.c_str(), GetFileExInfoStandard, &this->m_fileAttributes) == 0) {
		Print("File \"%s\" does not exist.", filename.c_str()); 
		PrintAbort(); 
		return false; 
	}
	
	std::ifstream in(filename, std::ofstream::in | std::ofstream::binary); 
	if (!in.is_open()) {
		Print("Couldn't open file \"%s\".", filename.c_str()); 
		PrintAbort(); 
		return false; 
	}
	
	Chunk header(in, 16); 
	if (header.getString(0) != "WPD") {
		Print("Magic word does not match \"%s\".", "WPD"); 
		PrintAbort(); 
		return false; 
	}
	
	unsigned count = header.getUnsigned(4); 
	Print("%u entries found.", count); 
	for (int i = 0 ; i < count ; i++) {
		Chunk entry(in, 32); 
		Chunk& data = this->getEntryData(entry.getString(0)); 
		data.resize(entry.getUnsigned(20)); 
		data.read(in, entry.getUnsigned(16)); 
	}
	
	PrintDone(); 
	return true; 
}

bool WPDFile::save (const std::string& filename) const {
	Print("Building WPD file \"%s\"...", filename.c_str()); 
	PrintStart(); 
	
	CreateFolderForFile(filename); 
	std::ofstream out(filename, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc); 
	if (!out.is_open()) {
		Print("Couldn't open file \"%s\".", filename.c_str()); 
		PrintAbort(); 
		return false; 
	}
	
	Chunk header(16); 
	header.setString(0, "WPD"); 
	header.setUnsigned(4, this->getEntryCount()); 
	header.write(out); 
	
	unsigned count = this->getEntryCount(); 
	Print("%u entries saved.", count); 
	unsigned dataOffset = 16 + count * 32; 
	for (auto it = this->m_entryList.begin() ; it != this->m_entryList.end() ; it++) {
		Chunk entry(32); 
		entry.setString(0, it->first); 
		entry.setUnsigned(16, dataOffset); 
		entry.setUnsigned(20, it->second.size()); 
		entry.write(out); 
		dataOffset += it->second.size(); 
	}
	
	for (auto it = this->m_entryList.begin() ; it != this->m_entryList.end() ; it++) {
		it->second.write(out); 
	}
	
	PrintDone(); 
	return true; 
}

bool WPDFile::patch (const std::string& filename, const std::string& format) {
	WIN32_FILE_ATTRIBUTE_DATA fileAttributes; 
	if (GetFileAttributesEx(filename.c_str(), GetFileExInfoStandard, &fileAttributes) == 0) {
		Print("File \"%s\" does not exist.", filename.c_str()); 
		return false; 
	} else if (CompareFileTime(&this->m_fileAttributes.ftLastWriteTime, &fileAttributes.ftLastWriteTime) >= 0) {
		return true; 
	}
		
	Print("Applying patch file \"%s\"...", filename.c_str()); 
	PrintStart(); 
	
	std::ifstream in(filename, std::ofstream::in); 
	if (!in.is_open()) {
		Print("Couldn't open file \"%s\".", filename.c_str()); 
		PrintAbort(); 
		return false; 
	}
	
	Format* fmt = Format::GetFormat(format); 
	if (fmt == nullptr) {
		Print("Couldn't load format \"%s\".", format.c_str()); 
		PrintAbort(); 
		return false; 
	}
	
	this->m_modified = true; 
	const Chunk& strings = this->getEntryData("!!string"); 
	std::regex regexEmpty("\\s*"); 
	std::regex regexComment("//\\s*(.*)"); 
	std::regex regexEntryName("@([^:]{1,15}):\\s*"); 
	std::regex regexEntryData(">\\s*((?:.(?!\\s*=))*.)\\s*=\\s*((?:.(?=\\s*\\S))*.)"); 
	std::regex regexDataTrue("(true)", std::regex::ECMAScript | std::regex::icase); 
	std::regex regexDataFalse("(false)", std::regex::ECMAScript | std::regex::icase); 
	std::regex regexDataHex("(0x[[:xdigit:]]+)"); 
	std::regex regexDataUnsigned("([[:digit:]]+)%?"); 
	std::regex regexDataSigned("(-?[[:digit:]]+)%?"); 
	std::regex regexDataFloat("(-?[[:digit:]]+(?:\\.[[:digit:]]+)?)%?"); 
	std::regex regexDataString("\"([^\"]*)\""); 
	std::smatch match; 
	
	std::string dataName; 
	std::string comment; 
	Chunk* data = nullptr; 
	while (!in.eof()) {
		std::string line; 
		std::getline(in, line); 
		
		if (regex_match(line, match, regexComment)) {
			comment = match[1]; 
			PrintVerbose("Comment: %s", comment.c_str()); 
			continue; 
			
		} else if (regex_match(line, match, regexEntryName)) {
			dataName = match[1]; 
			
			PrintVerbose("Patching entry %s...", dataName.c_str()); 
			data = &this->getEntryData(dataName); 
			if (data->size() != fmt->getSize()) {
				data->resize(fmt->getSize()); 
			}
			
		} else if (regex_match(line, match, regexEntryData)) {
			if (data == nullptr) {
				Print("Missing entry name."); 
				continue; 
			}
			
			std::string name = match[1]; 
			std::string value = match[2]; 
			
			const Format::Attribute* attribute; 
			try {
				attribute = &fmt->getAttribute(name); 
			} catch (const std::logic_error& e) {
				Print("In entry %s:", dataName.c_str()); 
				PrintStart(); 
				Print(e.what()); 
				PrintDone(); 
				continue; 
			}
			
			Enum* enumInfo = nullptr; 
			if (attribute->enumName != "") {
				enumInfo = Enum::GetEnum(attribute->enumName); 
			}
			
			switch (attribute->type) {
				case AttributeType::Boolean:
					if (regex_match(value, match, regexDataTrue)) {
						if (data->getBoolean(attribute->offset, attribute->bit) != true) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print("false -> true"); 
							PrintDone(); 
							data->setBoolean(attribute->offset, attribute->bit, true); 
						}
					} else if (regex_match(value, match, regexDataFalse)) {
						if (data->getBoolean(attribute->offset, attribute->bit) != false) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print("true -> false"); 
							PrintDone(); 
							data->setBoolean(attribute->offset, attribute->bit, false); 
						} 
					} else {
						Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
						PrintStart(); 
						Print("Unexpected value for boolean attribute (%s).", value.c_str()); 
						PrintDone(); 
						continue; 
					}
					break; 
				case AttributeType::Unsigned:
					if (regex_match(value, match, regexDataUnsigned)) {
						unsigned u = lexical_cast<unsigned>(match[1]); 
						if (data->getUnsignedMask(attribute->offset, attribute->bit, attribute->size) != u) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print("%u -> %u", data->getUnsignedMask(attribute->offset, attribute->bit, attribute->size), u); 
							PrintDone(); 
							data->setUnsignedMask(attribute->offset, attribute->bit, attribute->size, u); 
						}
					} else if (regex_match(value, match, regexDataHex)) {
						unsigned u = lexical_cast<unsigned>(match[1], std::hex); 
						if (data->getUnsignedMask(attribute->offset, attribute->bit, attribute->size) != u) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print("0x%0*X -> 0x%0*X", (attribute->size+3) / 4, data->getUnsignedMask(attribute->offset, attribute->bit, attribute->size), (attribute->size+3) / 4, u); 
							PrintDone(); 
							data->setUnsignedMask(attribute->offset, attribute->bit, attribute->size, u); 
						}
					} else if (enumInfo != nullptr) {
						try {
							unsigned u = enumInfo->getUnsigned(value); 
							if (data->getUnsignedMask(attribute->offset, attribute->bit, attribute->size) != u) {
								Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
								PrintStart(); 
								Print("%u -> %u", data->getUnsignedMask(attribute->offset, attribute->bit, attribute->size), u); 
								PrintDone(); 
								data->setUnsignedMask(attribute->offset, attribute->bit, attribute->size, u); 
							}
						} catch (const std::logic_error& e) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print(e.what()); 
							PrintDone(); 
						}
					} else {
						Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
						PrintStart(); 
						Print("Unexpected value for unsigned attribute (%s).", value.c_str()); 
						PrintDone(); 
						continue; 
					}
					break; 
				case AttributeType::Signed:
					if (regex_match(value, match, regexDataSigned)) {
						int i = lexical_cast<int>(match[1]); 
						if (data->getSignedMask(attribute->offset, attribute->bit, attribute->size) != i) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print("%d -> %d", data->getSignedMask(attribute->offset, attribute->bit, attribute->size), i); 
							PrintDone(); 
							data->setSignedMask(attribute->offset, attribute->bit, attribute->size, i); 
						}
					} else if (enumInfo != nullptr) {
						try {
							int i = enumInfo->getSigned(value); 
							if (data->getSignedMask(attribute->offset, attribute->bit, attribute->size) != i) {
								Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
								PrintStart(); 
								Print("%d -> %d", data->getSignedMask(attribute->offset, attribute->bit, attribute->size), i); 
								PrintDone(); 
								data->setSignedMask(attribute->offset, attribute->bit, attribute->size, i); 
							}
						} catch (const std::logic_error& e) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print(e.what()); 
							PrintDone(); 
						}
					} else {
						Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
						PrintStart(); 
						Print("Unexpected value for signed attribute (%s).", value.c_str()); 
						PrintDone(); 
						continue; 
					}
					break; 
				case AttributeType::Float:
					if (regex_match(value, match, regexDataFloat)) {
						float f = lexical_cast<float>(match[1]); 
						if (data->getFloat(attribute->offset) != f) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print("%.2f -> %.2f", data->getFloat(attribute->offset), f); 
							PrintDone(); 
							data->setFloat(attribute->offset, f); 
						}
					} else if (enumInfo != nullptr) {
						try {
							float f = enumInfo->getFloat(value); 
							if (data->getFloat(attribute->offset) != f) {
								Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
								PrintStart(); 
								Print("%.2f -> %.2f", data->getFloat(attribute->offset), f); 
								PrintDone(); 
								data->setFloat(attribute->offset, f); 
							}
						} catch (const std::logic_error& e) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print(e.what()); 
							PrintDone(); 
						}
					} else {
						Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
						PrintStart(); 
						Print("Unexpected value for float attribute (%s).", value.c_str()); 
						PrintDone(); 
						continue; 
					}
					break; 
				case AttributeType::String:
					if (regex_match(value, match, regexDataString)) {
						std::string s = match[1]; 
						if (strings.getString(data->getUnsigned(attribute->offset)) != s) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print("\"%s\" -> \"%s\"", strings.getString(data->getUnsigned(attribute->offset)).c_str(), s.c_str()); 
							PrintDone(); 
							data->setUnsigned(attribute->offset, this->getStringReference(s)); 
						}
					} else if (enumInfo != nullptr) {
						try {
							std::string s = enumInfo->getString(value); 
							if (strings.getString(data->getUnsigned(attribute->offset)) != s) {
								Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
								PrintStart(); 
								Print("\"%s\" -> \"%s\"", strings.getString(data->getUnsigned(attribute->offset)).c_str(), s.c_str()); 
								PrintDone(); 
								data->setUnsigned(attribute->offset, this->getStringReference(s)); 
							}
						} catch (const std::logic_error& e) {
							Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
							PrintStart(); 
							Print(e.what()); 
							PrintDone(); 
						}
					} else {
						Print("In entry %s, attribute %s:", dataName.c_str(), attribute->name.c_str()); 
						PrintStart(); 
						Print("Unexpected value for string attribute (%s).", value.c_str()); 
						PrintDone(); 
						continue; 
					}
			}
		} else if (!regex_match(line, regexEmpty)) {
			Print("Unexpected character in line \"%s\".", line.c_str()); 
		}
	}
	
	PrintDone(); 
	return true; 
}

bool WPDFile::convert (const std::string& filename, const std::string& filter, bool showHidden) const {
	Print("Building patch file \"%s\"...", filename.c_str()); 
	PrintStart(); 
	
	// Opening patch file
	CreateFolderForFile(filename); 
	std::ofstream out(filename, std::ofstream::out | std::ofstream::trunc); 
	if (!out.is_open()) {
		Print("Couldn't open file \"%s\".", filename.c_str()); 
		PrintAbort(); 
		return false; 
	}
	
	const Chunk& entryStrTypeList = this->getEntryData("!!strtypelist"); 
	const Chunk& entryString = this->getEntryData("!!string"); 
	
	// Converting entries
	unsigned count = 0; 
	for (auto it = this->m_entryList.begin() ; it != this->m_entryList.end() ; it++) {
		if ((it->first[0] == '!') || (strmatch(filter, it->first) == false)) {
			continue; 
		}
		
		// Writing entry header
		count++; 
		out << std::endl; 
		out << strfmt("@%s:", it->first.c_str()) << std::endl; 
		PrintVerbose("Converting entry %s...", it->first.c_str()); 
		
		// Converting attributes
		for (unsigned offset = 0 ; offset < entryStrTypeList.size() ; offset += 4) {
			std::string name = strfmt("[0x%04X|%02d|%02d]", offset, 0, 32); 
			out << strfmt("> %-30s = ", name.c_str()); 
			switch (entryStrTypeList.getUnsigned(offset)) {
				case 1:
					out << strfmt("%.2f", it->second.getFloat(offset)); 
					break; 
				case 2:
					out << strfmt("\"%s\"", entryString.getString(it->second.getUnsigned(offset)).c_str()); 
					break; 
				default:
					out << strfmt("0x%08X", it->second.getUnsigned(offset)); 
			}
			out << std::endl; 
		}
	}
	
	Print("%u entries converted.", count); 
	PrintDone(); 
	return true; 
}

bool WPDFile::convert (const std::string& filename, const std::string& format, const std::string& filter, bool showHidden) const {
	Print("Building patch file \"%s\"...", filename.c_str()); 
	PrintStart(); 
	
	// Opening patch file
	CreateFolderForFile(filename); 
	std::ofstream out(filename, std::ofstream::out | std::ofstream::trunc); 
	if (!out.is_open()) {
		Print("Couldn't open file \"%s\".", filename.c_str()); 
		PrintAbort(); 
		return false; 
	}
	
	const Chunk& entryString = this->getEntryData("!!string"); 
	Format* fmt = Format::GetFormat(format); 
	if (fmt == nullptr) {
		Print("Couldn't load format %s.", format.c_str()); 
		PrintAbort(); 
		return false; 
	}
	
	// Converting entries
	unsigned count = 0; 
	for (auto it = this->m_entryList.begin() ; it != this->m_entryList.end() ; it++) {
		if ((it->first[0] == '!') || (strmatch(filter, it->first) == false)) {
			continue; 
		}
		
		
		// Writing entry header
		count++; 
		out << std::endl; 
		out << strfmt("@%s:", it->first.c_str()) << std::endl; 
		PrintVerbose("Converting entry %s...", it->first.c_str()); 
		
		// Converting attributes
		const Format::Attributes& attributes = fmt->getAttributes(); 
		for (auto attribute = attributes.begin() ; attribute != attributes.end() ; attribute++) {
			if ((attribute->hidden == true) && (showHidden == false)) {
				continue; 
			}
			
			out << strfmt("> %-30s = ", attribute->name.c_str()); 
	
			Enum* enumInfo = nullptr; 
			if (attribute->enumName != "") {
				enumInfo = Enum::GetEnum(attribute->enumName); 
			}
			
			std::string str; 
			AttributeValue value; 
			switch (attribute->type) {
				case AttributeType::Boolean: 
					value.setBoolean(it->second.getBoolean(attribute->offset, attribute->bit)); 
					out << ((value.getBoolean() == true)? "true" : "false"); 
					break; 
				case AttributeType::Unsigned: 
					value.setUnsigned(it->second.getUnsignedMask(attribute->offset, attribute->bit, attribute->size)); 
					if (enumInfo != nullptr) {
						try {
							out << enumInfo->getName(value.getUnsigned()); 
							break; 
						} catch (const std::logic_error& e) {
							if (enumInfo->getStrict() == true) {
								Print("In entry %s, attribute %s:", it->first.c_str(), attribute->name.c_str()); 
								PrintStart(); 
								Print(e.what()); 
								PrintDone(); 
							}
						}
					}
					switch (attribute->format) {
						case AttributeFormat::Hexadecimal: 
							out << strfmt("0x%0*X", (attribute->size+3)/4, value.getUnsigned());
							break; 
						case AttributeFormat::Percentage: 
							out << strfmt("%u%%", value.getUnsigned());
							break; 
						default: 
							out << value.getUnsigned();
					}
					break; 
				case AttributeType::Signed: 
					value.setSigned(it->second.getSignedMask(attribute->offset, attribute->bit, attribute->size)); 
					if (enumInfo != nullptr) {
						try {
							out << enumInfo->getName(value.getSigned()); 
							break; 
						} catch (const std::logic_error& e) {
							if (enumInfo->getStrict() == true) {
								Print("In entry %s, attribute %s:", it->first.c_str(), attribute->name.c_str()); 
								PrintStart(); 
								Print(e.what()); 
								PrintDone(); 
							}
						}
					}
					switch (attribute->format) {
						case AttributeFormat::Percentage: 
							out << strfmt("%d%%", value.getSigned());
							break; 
						default: 
							out << value.getSigned();
					}
					break; 
				case AttributeType::Float: 
					value.setFloat(it->second.getFloat(attribute->offset)); 
					if (enumInfo != nullptr) {
						try {
							out << enumInfo->getName(value.getFloat()); 
							break; 
						} catch (const std::logic_error& e) {
							if (enumInfo->getStrict() == true) {
								Print("In entry %s, attribute %s:", it->first.c_str(), attribute->name.c_str()); 
								PrintStart(); 
								Print(e.what()); 
								PrintDone(); 
							}
						}
					}
					switch (attribute->format) {
						case AttributeFormat::Percentage: 
							out << strfmt("%.2f%%", value.getFloat());
							break; 
						default: 
							out << strfmt("%.2f", value.getFloat());
					}
					break; 
				case AttributeType::String: 
					value.setString(entryString.getString(it->second.getUnsigned(attribute->offset))); 
					if (enumInfo != nullptr) {
						try {
							out << enumInfo->getName(value.getString()); 
							break; 
						} catch (const std::logic_error& e) {
							if (enumInfo->getStrict() == true) {
								Print("In entry %s, attribute %s:", it->first.c_str(), attribute->name.c_str()); 
								PrintStart(); 
								Print(e.what()); 
								PrintDone(); 
							}
						}
					}
					out << strfmt("\"%s\"", value.getString().c_str());
			}
			out << std::endl; 
		}
	}
	
	Print("%u entries converted.", count); 
	PrintDone(); 
	return true; 
}

unsigned WPDFile::getStringReference (const std::string& str) {
	Chunk& strings = this->getEntryData("!!string"); 

	int i = 0; 
	int offset; 
	for (offset = 0 ; offset < strings.size() ; offset++) {
		if (strings[offset] == str[i]) {
			if (i == str.size()) {
				return offset-i; 
			} else {
				i++; 
			}
		} else {
			i = 0; 
		}
	}
	
	strings.resize(offset+str.size()+1); 
	strings.setString(offset, str); 
	return offset; 
}

unsigned WPDFile::getEntryCount () const {
	return this->m_entryList.size(); 
}

const Chunk& WPDFile::getEntryData (const std::string& id) const {
	return this->m_entryList.at(id); 
}

Chunk& WPDFile::getEntryData (const std::string& id) {
	return this->m_entryList[id]; 
}

bool WPDFile::getModified () const {
	return this->m_modified; 
}

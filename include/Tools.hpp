
#ifndef DBTOOL_HEADER_TOOLS
#define DBTOOL_HEADER_TOOLS

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace dbtool {

	void CreateFolderForFile(const std::string& filename); 
	
	bool strmatch(const char* format, const char* string); 
	bool strmatch(const char* format, const char* string, const char* end); 
	bool strmatch(const std::string& format, const std::string& string); 
	
	std::string strfmt(const std::string& format, ...); 
	
	template<typename T, typename S>
	inline T lexical_cast(const S& in) {
		T out; 
		std::stringstream str; 
		str << in; 
		str >> out; 
		if (str.fail()) {
			throw std::logic_error("Bad lexical cast."); 
		}
		return out; 
	}
	
	template<typename T, typename S>
	inline T lexical_cast(const S& in, std::ios_base&(*format)(std::ios_base&)) {
		T out; 
		std::stringstream str; 
		str << format << in; 
		str >> format >> out; 
		if (str.fail()) {
			throw std::logic_error("Bad lexical cast."); 
		}
		return out; 
	}
	
	void SetVerboseMode(bool verbose); 
	
	void Print(); 
	void Print(const std::string& format, ...); 
	void PrintVerbose(const std::string& format, ...); 
	void PrintStart(); 
	void PrintAbort(); 
	void PrintDone(); 
	
}

#endif

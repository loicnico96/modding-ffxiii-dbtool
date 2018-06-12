
#include <cstdarg>
#include <windows.h>

#include "include/Tools.hpp"

using namespace dbtool; 

int PrintIndent = 0; 
bool PrintVerboseMode = false; 

void dbtool::CreateFolderForFile(const std::string& filename) {
	std::string::size_type end; 
	for (end = filename.find('/') ; end != std::string::npos ; end = filename.find('/', end+1)) {
		std::string folder = filename.substr(0, end); 
		if (CreateDirectory(folder.c_str(), nullptr)) {
			Print("Creating directory %s...", folder.c_str()); 
		}
	}
}

bool dbtool::strmatch(const char* format, const char* string, const char* end) {
	if ((format == end) || (*format == '\0')) {
		return (*string == '\0'); 
	} else if (*string == '\0') {
		return (*format == '*') && ((format+1 == end) || (*(format+1) == '\0')); 
	} else if ((*format == *string) || (*format == '?')) {
		return strmatch(format+1, string+1, end); 
	} else if (*format == '*') {
		return strmatch(format+1, string, end) || strmatch(format, string+1, end); 
	} else {
		return false; 
	}
}

bool dbtool::strmatch(const char* format, const char* string) {
	const char* end; 
	while (true) {
		end = strchr(format, ';'); 
		if (strmatch(format, string, end)) {
			return true; 
		} else if (end == nullptr) {
			return false; 
		}
		format = end+1; 
	}
}

bool dbtool::strmatch(const std::string& format, const std::string& string) {
	return strmatch(format.c_str(), string.c_str()); 
}

std::string dbtool::strfmt(const std::string& format, ...) {
	va_list args; 
	va_start(args, format); 
	unsigned size = vsnprintf(NULL, 0, format.c_str(), args) + 1; 
	char* buf = (char*)malloc(size*sizeof(char)); 
	vsnprintf(buf, size, format.c_str(), args); 
	std::string str(buf); 
	free(buf); 
	va_end(args); 
	return str; 
}

void dbtool::SetVerboseMode(bool verbose) {
	PrintVerboseMode = verbose; 
}

void dbtool::Print() {
	printf("\n"); 
}

void dbtool::Print(const std::string& format, ...) {
	if (PrintIndent > 0) {
		for (int i = 0 ; i < PrintIndent ; i++) {
			printf(">"); 
		}
		printf(" "); 
	}
	va_list args; 
	va_start(args, format); 
	vprintf(format.c_str(), args); 
	va_end(args); 
	printf("\n"); 
}

void dbtool::PrintVerbose(const std::string& format, ...) {
	if (PrintVerboseMode == true) {
		if (PrintIndent > 0) {
			for (int i = 0 ; i < PrintIndent ; i++) {
				printf(">"); 
			}
			printf(" "); 
		}
		va_list args; 
		va_start(args, format); 
		vprintf(format.c_str(), args); 
		va_end(args); 
		printf("\n"); 
	}
}

void dbtool::PrintStart() {
	PrintIndent++; 
}

void dbtool::PrintDone() {
	if (PrintIndent > 0) {
		PrintIndent--; 
	}
	if (PrintIndent == 0) {
		Print(); 
	}
}

void dbtool::PrintAbort() {
	Print("ABORTED"); 
	PrintDone(); 
}

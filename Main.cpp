
#include <cstdlib>
#include <fstream>
#include <list>
#include <string>
#include <windows.h>

#include "include/Tools.hpp"
#include "include/WPDFile.hpp"
#include "tinyxml2/tinyxml2.h"

using namespace dbtool; 

int main (int argc, char** argv) {

	// Commands: 
	// -h or -?				Display help. 
	// -P filelist			Patch all files in the filelist. 
	// -G filelist			Generate all patch files for files in the filelist. 
	
	// Options: 
	// -v					Verbose (show more information)
	// -s					Show hidden values
	// -c					Import modified files to white_imgc
	
	if (argc == 1) {
		goto ShowHelp; 
	} else {
		std::string command = argv[1]; 
		
		bool importc = false; 
		bool verbose = false; 
		bool showAll = false; 
		for (int i = 2 ; i < argc ; i++) {
			std::string arg = argv[i]; 
			if (arg[0] == '-') {
				if (arg == "-v") {
					verbose = true; 
				} else if (arg == "-s") {
					showAll = true; 
				} else if (arg == "-c") {
					importc = true; 
				} else {
					Print("Unknown option (\"%s\").", arg.c_str()); 
					goto ShowHelp; 
				}
			}
		}
		SetVerboseMode(verbose); 
		
		// -h or -? = Help
		if ((command == "-h") || (command == "-?")) {
			goto ShowHelp; 
			
		// -G = Generate all
		} else if (command == "-G") {
			for (int i = 2 ; i < argc ; i++) {
				std::string filelist = argv[i]; 
				if (filelist[0] != '-') {
					if (filelist == "") {
						Print("Missing filelist argument."); 
						continue; 
					}
					
					Print("Reading filelist \"%s\"...", filelist.c_str()); 
					PrintStart(); 
					
					tinyxml2::XMLDocument xml; 
					xml.LoadFile(filelist.c_str()); 
					if (xml.Error()) {
						Print("Couldn't open XML filelist \"%s\".", filelist.c_str()); 
						PrintAbort(); 
						continue; 
					}
					
					tinyxml2::XMLElement* xmlfilelist = xml.FirstChildElement("filelist"); 
					if (xmlfilelist == nullptr) {
						Print("Missing filelist element."); 
						PrintAbort(); 
						continue; 
					}
					
					PrintDone(); 
					
					tinyxml2::XMLElement* xmlfile; 
					for (xmlfile = xmlfilelist->FirstChildElement("file") ; xmlfile != nullptr ; xmlfile = xmlfile->NextSiblingElement("file")) {
						WPDFile file; 
						
						const char* fileName = xmlfile->Attribute("name"); 
						if (fileName == nullptr) {
							Print("Missing file \"%s\" attribute.", "name"); 
							continue; 
						} else if (!file.load(strfmt("sys/%s", fileName))) {
							continue; 
						}
						
						std::string format = "default"; 
						const char* fileFormat = xmlfile->Attribute("format"); 
						if (fileFormat != nullptr) {
							format = fileFormat; 
						}
						
						tinyxml2::XMLElement* xmlpatch;
						for (xmlpatch = xmlfile->FirstChildElement("patch") ; xmlpatch != nullptr ; xmlpatch = xmlpatch->NextSiblingElement("patch")) {
							std::string filter = "*"; 
							const char* patchFilter = xmlpatch->Attribute("filter"); 
							if (patchFilter != nullptr) {
								filter = patchFilter; 
							}
							
							const char* patchName = xmlpatch->Attribute("name"); 
							if (patchName == nullptr) {
								Print("Missing patch \"%s\" attribute.", "name"); 
								continue; 
							} else if (format == "default") {
								file.convert(strfmt("patch/%s", patchName), filter, showAll); 
							} else {
								file.convert(strfmt("patch/%s", patchName), format, filter, showAll); 
							}
						}
						
						std::ofstream out(strfmt("sys/%s", fileName), std::ofstream::in | std::ofstream::out | std::ofstream::binary); 
						if (out.is_open()) {
							out.seekp(0, std::ofstream::beg); 
							out.write("WPD", 3); 
						}
					}
				}
			}
			
			goto ExitSuccess; 
		} else if (command == "-P") {
			std::list<std::string> files; 
			
			for (int i = 2 ; i < argc ; i++) {
				std::string filelist = argv[i]; 
				if (filelist[0] != '-') {
					if (filelist == "") {
						Print("Missing filelist argument."); 
						continue; 
					}
					
					Print("Reading filelist \"%s\"...", filelist.c_str()); 
					PrintStart(); 
					
					tinyxml2::XMLDocument xml; 
					xml.LoadFile(filelist.c_str()); 
					if (xml.Error()) {
						Print("Couldn't open XML filelist \"%s\".", filelist.c_str()); 
						PrintAbort(); 
						continue; 
					}
					
					tinyxml2::XMLElement* xmlfilelist = xml.FirstChildElement("filelist"); 
					if (xmlfilelist == nullptr) {
						Print("Missing filelist element."); 
						PrintAbort(); 
						continue; 
					}
					
					PrintDone(); 
					
					tinyxml2::XMLElement* xmlfile; 
					for (xmlfile = xmlfilelist->FirstChildElement("file") ; xmlfile != nullptr ; xmlfile = xmlfile->NextSiblingElement("file")) {
						WPDFile file; 
						
						const char* fileName = xmlfile->Attribute("name"); 
						if (fileName == nullptr) {
							Print("Missing file \"%s\" attribute.", "name"); 
							continue; 
						} else if (!file.load(strfmt("sys/%s", fileName))) {
							continue; 
						}
						
						const char* fileFormat = xmlfile->Attribute("format"); 
						if (fileFormat == nullptr) {
							Print("Missing file \"%s\" attribute.", "format"); 
							continue; 
						}
						
						tinyxml2::XMLElement* xmlpatch;
						for (xmlpatch = xmlfile->FirstChildElement("patch") ; xmlpatch != nullptr ; xmlpatch = xmlpatch->NextSiblingElement("patch")) {
							const char* patchName = xmlpatch->Attribute("name"); 
							if (patchName == nullptr) {
								Print("Missing patch \"%s\" attribute.", "name"); 
								continue; 
							} else {
								file.patch(strfmt("patch/%s", patchName), fileFormat); 
							}
						}
						
						if (file.getModified()) {
							file.save(strfmt("sys/%s", fileName)); 
							files.push_back(fileName); 
						}
					}
				}
			}
			
			if (importc == true) {
				for (auto it = files.begin() ; it != files.end() ; it++) {
					Print("Importing file \"%s\"...", it->c_str()); 
					PrintStart(); 
					std::string processName = "sys\\ff13tool.exe"; 
					std::string processLine = strfmt("%s -i -ff13 filelistc.win32.bin white_imgc.win32.bin %s", processName.c_str(), it->c_str()); 
					STARTUPINFO startupInfo = { sizeof(STARTUPINFO), NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, NULL, NULL, NULL, NULL }; 
					PROCESS_INFORMATION processInfo; 
					if (CreateProcess(NULL, const_cast<char*>(processLine.c_str()), nullptr, nullptr, 0, 0, NULL, ".\\sys", &startupInfo, &processInfo)) {
						WaitForSingleObject(processInfo.hProcess, INFINITE); 
						CloseHandle(processInfo.hProcess); 
						CloseHandle(processInfo.hThread); 
						PrintDone(); 
					} else {
						Print("Couldn't create process \"%s\" (error %d).", processName.c_str(), GetLastError()); 
						PrintAbort(); 
					}
				}
			}
			
			goto ExitSuccess; 
		// Unknown command
		} else {
			Print("Unknown command (\"%s\").", command.c_str()); 
			goto ShowHelp; 
		}
	}
	
ShowHelp:
	Print(); 
	Print("Commands:");
	Print("-h or -?"); 
	Print("\tDisplay a list of commands."); 
	Print("-G filelist"); 
	Print("\tGenerate all files indicated in the filelist."); 
	Print("-P filelist"); 
	Print("\tPatch all files indicated in the filelist."); 
	Print(); 
	Print("Options:");
	Print("-v"); 
	Print("\tVerbose mode (show more information)."); 
	Print("-s"); 
	Print("\tShow hidden values."); 
	Print(); 
ExitSuccess:
	return EXIT_SUCCESS; 
ExitFailure:
	return EXIT_FAILURE; 
}

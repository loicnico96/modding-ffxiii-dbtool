
#ifndef DBTOOL_HEADER_WPD_FILE
#define DBTOOL_HEADER_WPD_FILE

#include <map>
#include <string>
#include <windows.h>

#include "Chunk.hpp"
#include "Tools.hpp"

namespace dbtool {
	
	class WPDFile {
		private: 
			typedef std::map<std::string, Chunk> WPDFileEntryList; 
			WPDFileEntryList m_entryList; 
			bool m_modified; 
			WIN32_FILE_ATTRIBUTE_DATA m_fileAttributes; 
		
		public:
			WPDFile (); 
			~WPDFile (); 
			
			bool load (const std::string& filename); 
			bool save (const std::string& filename) const; 
			bool patch (const std::string& filename, const std::string& format); 
			bool convert (const std::string& filename, const std::string& filter, bool showHidden) const; 
			bool convert (const std::string& filename, const std::string& format, const std::string& filter, bool showHidden) const; 
			
			unsigned getEntryCount () const; 
			unsigned getStringReference (const std::string& str); 
			const Chunk& getEntryData (const std::string& id) const; 
			Chunk& getEntryData (const std::string& id); 
			bool getModified () const; 
	}; 
	
}

#endif

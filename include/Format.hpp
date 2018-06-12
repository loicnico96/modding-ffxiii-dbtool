
#ifndef DBTOOL_HEADER_FORMAT
#define DBTOOL_HEADER_FORMAT

#include <list>
#include <unordered_map>

#include "AttributeFormat.hpp"
#include "AttributeType.hpp"
#include "Tools.hpp"

namespace dbtool {
	
	class Format {
		public: 
			struct Attribute {
				std::string		name; 
				AttributeType	type; 
				AttributeFormat	format; 
				std::string		enumName; 
				unsigned		offset; 
				unsigned		bit; 
				unsigned		size; 
				bool			hidden; 
				Attribute(); 
				Attribute(const Attribute& attribute); 
				Attribute& operator = (const Attribute& attribute); 
				~Attribute(); 
			}; 
			
			typedef std::list<Attribute> Attributes; 
			
		private:
			Attributes		m_attributes; 
			unsigned		m_size; 
			std::string		m_name; 
		
			typedef std::unordered_map<std::string, Format> Formats; 
			static Formats	s_formats; 
			
		public: 
			Format(); 
			~Format(); 
			
			unsigned getSize() const; 
			
			const Attributes& getAttributes() const; 
			
			const Attribute& getAttribute(const std::string& name) const; 
			
			static Format* GetFormat(const std::string& name); 
	}; 
}

#endif


#ifndef DBTOOL_HEADER_ENUM
#define DBTOOL_HEADER_ENUM

#include <unordered_map>

#include "AttributeValue.hpp"
#include "Tools.hpp"

namespace dbtool {
	
	class Enum; 
	typedef std::unordered_map<std::string, AttributeValue> AttributeValueMap; 
	typedef std::unordered_map<std::string, Enum> EnumMap; 
	
	class Enum {
		private: 
			AttributeValueMap	m_values; 
			AttributeType		m_type; 
			std::string			m_name; 
			bool				m_strict; 
			static EnumMap		s_enums; 
			
		public: 
			Enum(); 
			~Enum(); 
			
			AttributeType getType() const; 
			bool getStrict() const; 
			
			unsigned getUnsigned(const std::string& name) const; 
			int getSigned(const std::string& name) const; 
			float getFloat(const std::string& name) const; 
			std::string getString(const std::string& name) const; 
			
			std::string getName(unsigned u) const; 
			std::string getName(int i) const; 
			std::string getName(float f) const; 
			std::string getName(const std::string& s) const; 
			
			static Enum* GetEnum(const std::string& name); 
	}; 
	
}

#endif

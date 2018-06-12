
#ifndef DBTOOL_HEADER_ATTRIBUTE_VALUE
#define DBTOOL_HEADER_ATTRIBUTE_VALUE

#include <string>

#include "AttributeType.hpp"
#include "Tools.hpp"

namespace dbtool {
	
	class AttributeValue {
		private: 
			union {
				bool			b; 
				unsigned		u; 
				int				i; 
				float			f; 
				std::string*	s; 
			}					m_value; 
			AttributeType		m_type; 
		
		public: 
			AttributeValue(); 
			AttributeValue(bool b); 
			AttributeValue(unsigned u); 
			AttributeValue(int i); 
			AttributeValue(float f); 
			AttributeValue(const std::string& s); 
			AttributeValue(const AttributeValue& value); 
			AttributeValue(AttributeValue&& value); 
			AttributeValue& operator = (const AttributeValue& value); 
			AttributeValue& operator = (AttributeValue&& value); 
			~AttributeValue(); 
			
			AttributeType getType() const; 
			
			bool getBoolean() const; 
			unsigned getUnsigned() const; 
			int getSigned() const; 
			float getFloat() const; 
			std::string getString() const; 
			
			void setBoolean(bool b); 
			void setUnsigned(unsigned u); 
			void setSigned(int i); 
			void setFloat(float f); 
			void setString(const std::string& s); 
	}; 
	
}

#endif


#ifndef DBTOOL_HEADER_CHUNK
#define DBTOOL_HEADER_CHUNK

#include <string>

namespace dbtool {
	
	class Chunk {
		private: 
			char* m_data; 
			unsigned m_size; 
		
		public: 
			Chunk (); 
			Chunk (unsigned size); 
			Chunk (const Chunk& chunk); 
			Chunk (std::istream& in, int size); 
			Chunk (std::istream& in, int offset, int size); 
			Chunk (Chunk&& chunk); 
			~Chunk (); 
		
			Chunk& operator = (const Chunk& chunk); 
			Chunk& operator = (Chunk&& chunk); 
			
			void clear (); 
			void release (); 
			unsigned size () const; 
			void resize (unsigned size); 
			
			void read (std::istream& in); 
			void read (std::istream& in, int offset); 
			void write (std::ostream& out) const;  
			void write (std::ostream& out, int offset) const; 
			
			char operator [] (unsigned offset) const; 
			
			bool getBoolean (unsigned offset, unsigned bit) const; 
			void setBoolean (unsigned offset, unsigned bit, bool value); 
			
			int getSigned (unsigned offset) const; 
			void setSigned (unsigned offset, int value); 
			int getSignedMask (unsigned offset, unsigned bitStart, unsigned bitLength) const; 
			void setSignedMask (unsigned offset, unsigned bitStart, unsigned bitLength, int value); 
			
			unsigned getUnsigned (unsigned offset) const; 
			void setUnsigned (unsigned offset, unsigned value); 
			unsigned getUnsignedMask (unsigned offset, unsigned bitStart, unsigned bitLength) const; 
			void setUnsignedMask (unsigned offset, unsigned bitStart, unsigned bitLength, unsigned value); 
			
			float getFloat (unsigned offset) const; 
			void setFloat (unsigned offset, float value); 
			
			std::string getString (unsigned offset) const; 
			void setString (unsigned offset, const std::string& string); 
			
			Chunk getChunk (unsigned offset, unsigned size) const; 
			void setChunk (unsigned offset, const Chunk& chunk); 
	}; 
	
}

#endif

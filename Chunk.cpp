
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "include/Chunk.hpp"

using namespace dbtool; 

Chunk::Chunk ()
	:m_data(nullptr), m_size(0) {}
Chunk::Chunk (unsigned size)
	:m_data(nullptr), m_size(0) {
		if (size > 0) {
			try {
				if (size % 4 != 0) {
					size += 4 - size % 4; 
				}
				this->m_data = new char [size]; 
				this->m_size = size; 
				this->clear(); 
			} catch (const std::bad_alloc& e) {
				std::cout << "Failed to allocate " << size*sizeof(char) << " bytes." << std::endl; 
			}
		}
	}
Chunk::Chunk (std::istream& in, int size)
	:m_data(nullptr), m_size(0) {
		if (size > 0) {
			try {
				if (size % 4 != 0) {
					size += 4 - size % 4; 
				}
				this->m_data = new char [size]; 
				this->m_size = size; 
				this->read(in); 
			} catch (const std::bad_alloc& e) {
				std::cout << "Failed to allocate " << size*sizeof(char) << " bytes." << std::endl; 
			}
		}
	}
Chunk::Chunk (std::istream& in, int offset, int size)
	:m_data(nullptr), m_size(0) {
		if (size > 0) {
			try {
				if (size % 4 != 0) {
					size += 4 - size % 4; 
				}
				this->m_data = new char [size]; 
				this->m_size = size; 
				this->read(in, offset); 
			} catch (const std::bad_alloc& e) {
				std::cout << "Failed to allocate " << size*sizeof(char) << " bytes." << std::endl; 
			}
		}
	}
Chunk::Chunk (const Chunk& chunk)
	:m_data(nullptr), m_size(0) {
		if (chunk.m_size > 0) {
			try {
				this->m_data = new char [chunk.m_size]; 
				this->m_size = chunk.m_size; 
				std::memcpy(this->m_data, chunk.m_data, chunk.m_size); 
			} catch (const std::bad_alloc& e) {
				std::cout << "Failed to allocate " << chunk.m_size*sizeof(char) << " bytes." << std::endl; 
			}
		}
	}
Chunk::Chunk (Chunk&& chunk)
	:m_data(chunk.m_data), m_size(chunk.m_size) {
		chunk.m_data = nullptr; 
		chunk.m_size = 0; 
	}
Chunk::~Chunk () {
	this->release(); 
}

Chunk& Chunk::operator = (const Chunk& chunk) {
	if (this != &chunk) {
		this->release(); 
		try {
			this->m_data = new char [chunk.m_size]; 
			this->m_size = chunk.m_size; 
			std::memcpy(this->m_data, chunk.m_data, chunk.m_size); 
		} catch (const std::bad_alloc& e) {
			std::cout << "Failed to allocate " << chunk.m_size*sizeof(char) << " bytes." << std::endl; 
		}
	}
	return *this; 
}
Chunk& Chunk::operator = (Chunk&& chunk) {
	if (this != &chunk) {
		this->release(); 
		this->m_data = chunk.m_data; 
		this->m_size = chunk.m_size; 
		chunk.m_data = nullptr; 
		chunk.m_size = 0; 
	}
	return *this; 
}

void Chunk::clear () {
	if (this->m_data != nullptr) {
		std::memset(this->m_data, 0, this->m_size*sizeof(char)); 
	}
}
void Chunk::release () {
	if (this->m_data != nullptr) {
		delete [] this->m_data; 
		this->m_data = nullptr; 
		this->m_size = 0; 
	}
}

unsigned Chunk::size () const {
	return this->m_size; 
}
void Chunk::resize (unsigned size) {
	if (size == 0) {
		this->release(); 
	} else if (size != this->m_size) {
		try {
			if (size % 4 != 0) {
				size += 4 - size % 4; 
			}
			char* data = new char [size]; 
			std::memset(data, 0, size*sizeof(char)); 
			if (this->m_data != nullptr) {
				if (size > this->m_size) {
					std::memcpy(data, this->m_data, this->m_size); 
				} else {
					std::memcpy(data, this->m_data, size); 
				}
				delete [] this->m_data; 
			}
			this->m_data = data; 
			this->m_size = size; 
		} catch (const std::bad_alloc& e) {
			std::cout << "Failed to allocate " << size*sizeof(char) << " bytes." << std::endl; 
		}
	}
}
			
void Chunk::read (std::istream& in) {
	in.read(this->m_data, this->m_size); 
}
void Chunk::read (std::istream& in, int offset) {
	std::streampos pos = in.tellg(); 
	in.seekg(offset, std::istream::beg); 
	this->read(in); 
	in.seekg(pos); 
}
void Chunk::write (std::ostream& out) const {
	out.write(this->m_data, this->m_size); 
}
void Chunk::write (std::ostream& out, int offset) const {
	std::streampos pos = out.tellp(); 
	out.seekp(offset, std::ostream::beg); 
	this->write(out); 
	out.seekp(pos); 
}

char Chunk::operator [] (unsigned offset) const {
	if (offset > this->m_size-1)
		throw std::range_error("Index out of range."); 
	return this->m_data[offset]; 
}

bool Chunk::getBoolean (unsigned offset, unsigned bit) const {
	return (this->getUnsignedMask(offset, bit, 1) != 0); 
}
void Chunk::setBoolean (unsigned offset, unsigned bit, bool value) {
	this->setUnsignedMask(offset, bit, 1, value? 1 : 0); 
}

int Chunk::getSigned (unsigned offset) const {
	if (offset > this->m_size-4)
		throw std::range_error("Index out of range."); 
	int i = *(reinterpret_cast<int*>(&this->m_data[offset])); 
	return ((i >> 24) & 0x000000FF) | ((i >> 8) & 0x0000FF00) | ((i << 8) & 0x00FF0000) | ((i << 24) & 0xFF000000); 
}
void Chunk::setSigned (unsigned offset, int value) {
	if (offset > this->m_size-4)
		throw std::range_error("Index out of range."); 
	int i = ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000); 
	*(reinterpret_cast<int*>(&this->m_data[offset])) = i; 
}
int Chunk::getSignedMask (unsigned offset, unsigned bitStart, unsigned bitLength) const {
	if ((bitLength > 0) || (bitStart < 32)) {
		int i = this->getSigned(offset); 
		i = i << (32 - bitLength - bitStart); 
		i = i >> (32 - bitLength); 
		return i; 
	} else {
		return 0; 
	}
}
void Chunk::setSignedMask (unsigned offset, unsigned bitStart, unsigned bitLength, int value) {
	if ((bitLength > 0) || (bitStart < 32)) {
		int mask = ((bitLength < 32)? (1 << bitLength) - 1 : -1) << bitStart; 
		int i = this->getSigned(offset); 
		i = i & ~mask; 
		i = i | (mask & (value << bitStart)); 
		this->setSigned(offset, i); 
	}
}

unsigned Chunk::getUnsigned (unsigned offset) const {
	if (offset > this->m_size-4)
		throw std::range_error("Index out of range."); 
	unsigned u = *(reinterpret_cast<unsigned*>(&this->m_data[offset])); 
	return ((u >> 24) & 0x000000FF) | ((u >> 8) & 0x0000FF00) | ((u << 8) & 0x00FF0000) | ((u << 24) & 0xFF000000); 
}
void Chunk::setUnsigned (unsigned offset, unsigned value) {
	if (offset > this->m_size-4)
		throw std::range_error("Index out of range."); 
	unsigned u = ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000); 
	*(reinterpret_cast<unsigned*>(&this->m_data[offset])) = u; 
}
unsigned Chunk::getUnsignedMask (unsigned offset, unsigned bitStart, unsigned bitLength) const {
	if ((bitLength > 0) || (bitStart < 32)) {
		unsigned u = this->getUnsigned(offset); 
		u = u << (32 - bitLength - bitStart); 
		u = u >> (32 - bitLength); 
		return u; 
	} else {
		return 0; 
	}
}
void Chunk::setUnsignedMask (unsigned offset, unsigned bitStart, unsigned bitLength, unsigned value) {
	if ((bitLength > 0) || (bitStart < 32)) {
		unsigned mask = ((bitLength < 32)? (1 << bitLength) - 1 : -1) << bitStart; 
		unsigned u = this->getUnsigned(offset); 
		u = u & ~mask; 
		u = u | (mask & (value << bitStart)); 
		this->setUnsigned(offset, u); 
	}
}

float Chunk::getFloat (unsigned offset) const {
	unsigned u = this->getUnsigned(offset); 
	return *(reinterpret_cast<float*>(&u)); 
}
void Chunk::setFloat (unsigned offset, float value) {
	unsigned u = *(reinterpret_cast<unsigned*>(&value)); 
	this->setUnsigned(offset, u); 
}

std::string Chunk::getString (unsigned offset) const {
	if (offset > this->m_size-1)
		throw std::range_error("Index out of range."); 
	std::string s(&this->m_data[offset]); 
	return s; 
}
void Chunk::setString (unsigned offset, const std::string& string) {
	if (offset > this->m_size-string.size()-1)
		throw std::range_error("Index out of range."); 
	std::strcpy(&this->m_data[offset], string.c_str()); 
}

Chunk Chunk::getChunk (unsigned offset, unsigned size) const {
	if (offset > this->m_size-size)
		throw std::range_error("Index out of range."); 
	Chunk chunk(size); 
	std::memcpy(chunk.m_data, &this->m_data[offset], chunk.m_size); 
	return chunk; 
}
void Chunk::setChunk (unsigned offset, const Chunk& chunk) {
	if (chunk.m_size > 0) {
		if (offset > this->m_size-chunk.m_size)
			throw std::range_error("Index out of range."); 
		std::memcpy(&this->m_data[offset], chunk.m_data, chunk.m_size); 
	}
}

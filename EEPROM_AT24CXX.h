/*
 * EEPROM_AT24CXX.h
 * Copyright (C) wlkmanist. 
 * 04.12.2021
 */

#ifndef _EEPROM_AT24CXX_
#define _EEPROM_AT24CXX_

#include <Wire.h>
#include <Arduino.h>
#include <EEPROM.h>

static uint8_t AT24read(const uint8_t &i2c_addr, uint16_t idx)
{
	delayMicroseconds(2); // Tbuf + Thd.sta + Tsu.sta

	Wire.beginTransmission((uint8_t)i2c_addr);

	Wire.write(highByte(idx));
	Wire.write(lowByte(idx));

	Wire.endTransmission();
	delayMicroseconds(1);

	Wire.requestFrom((uint8_t)i2c_addr, (uint8_t)1);
	delayMicroseconds(1);

	return (uint8_t)Wire.read();
}

static void AT24write(const uint8_t &i2c_addr, uint16_t idx, uint8_t val)
{
	delayMicroseconds(2); // Tbuf + Thd.sta + Tsu.sta

	Wire.beginTransmission((uint8_t)i2c_addr);

	Wire.write(highByte(idx));
	Wire.write(lowByte(idx));
	Wire.write(val);
	delayMicroseconds(5000); // Twr

	Wire.endTransmission();
}

struct EERefAT24 {

	EERefAT24(const uint8_t &i2c_addr, const int index)
		: i2c_address(i2c_addr), index(index) {}

	//Access/read members.
	uint16_t operator*() const { return AT24read(i2c_address, (uint16_t)index); }
	operator uint16_t() const { return **this; }

	//Assignment/write members.
	EERefAT24 &operator=(const EERefAT24 &ref) { return *this = *ref; }
	EERefAT24 &operator=(uint8_t in) { AT24write(i2c_address, (uint16_t)index, in); return *this; }
	EERefAT24 &operator +=(uint8_t in) { return *this = **this + in; }
	EERefAT24 &operator -=(uint8_t in) { return *this = **this - in; }
	EERefAT24 &operator *=(uint8_t in) { return *this = **this * in; }
	EERefAT24 &operator /=(uint8_t in) { return *this = **this / in; }
	EERefAT24 &operator ^=(uint8_t in) { return *this = **this ^ in; }
	EERefAT24 &operator %=(uint8_t in) { return *this = **this % in; }
	EERefAT24 &operator &=(uint8_t in) { return *this = **this & in; }
	EERefAT24 &operator |=(uint8_t in) { return *this = **this | in; }
	EERefAT24 &operator <<=(uint8_t in) { return *this = **this << in; }
	EERefAT24 &operator >>=(uint8_t in) { return *this = **this >> in; }

	EERefAT24 &update(uint8_t in) { return  in != *this ? *this = in : *this; }

	/** Prefix increment/decrement **/
	EERefAT24 &operator++() { return *this += 1; }
	EERefAT24 &operator--() { return *this -= 1; }

	/** Postfix increment/decrement **/
	uint16_t operator++ (int) {
		uint16_t ret = **this;
		return ++(*this), ret;
	}

	uint16_t operator-- (int) {
		uint16_t ret = **this;
		return --(*this), ret;
	}

	int index; //Index of current EEPROM cell.
	uint8_t i2c_address;
};

struct EEPtrAT24 {

	EEPtrAT24(const uint8_t &i2c_addr, const int index)
		: i2c_address(i2c_addr), index(index) {}

	operator int() const { return index; }
	EEPtrAT24 &operator=(int in) { return index = in, *this; }

	//Iterator functionality.
	bool operator!=(const EEPtrAT24 &ptr) { return index != ptr.index; }
	EERefAT24 operator*() { return EERefAT24(i2c_address, index); }

	/** Prefix & Postfix increment/decrement **/
	EEPtrAT24& operator++() { return ++index, *this; }
	EEPtrAT24& operator--() { return --index, *this; }
	EEPtrAT24 operator++ (int) { index++; return *this;	}
	EEPtrAT24 operator-- (int) { index--; return *this;	}

	int index; //Index of current EEPROM cell.
	uint8_t i2c_address;
};

class AT24Cxx : public EEPROMClass
{
protected:
	uint32_t eeprom_size;
	uint8_t i2c_address;

public:
	AT24Cxx(const uint8_t i2c, uint32_t size) : eeprom_size(size), i2c_address(i2c) { Wire.begin(); }
	EERefAT24 operator[](const int idx) { return EERefAT24(this->i2c_address, idx); }
	uint8_t read(int idx) { return EERefAT24(this->i2c_address, idx); }
	void write(int idx, uint8_t val) { (EERefAT24(this->i2c_address, idx)) = val; }
	void update(int idx, uint8_t val) { EERefAT24(this->i2c_address, idx).update(val); }
	uint32_t length() { return this->eeprom_size * 0x400; }

	template< typename T > T &get(int idx, T &t) {
		EEPtrAT24 e(this->i2c_address, idx);
		uint8_t *ptr = (uint8_t*)&t;
		for (int count = sizeof(T); count; --count, ++e)  *ptr++ = *e;
		return t;
	}

	template< typename T > const T &put(int idx, const T &t) {
		EEPtrAT24 e(this->i2c_address, idx);
		const uint8_t *ptr = (const uint8_t*)&t;
		for (int count = sizeof(T); count; --count, ++e)  (*e).update(*ptr++);
		return t;
	}
};

#endif

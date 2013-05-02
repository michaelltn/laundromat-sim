#pragma once

class Customer
{
private:
	char _displayChar;
public:
	Customer(char displayChar)
	{
		_displayChar = displayChar;
	}

	char getDisplayChar() { return _displayChar; }
};
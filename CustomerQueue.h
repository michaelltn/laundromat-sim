#pragma once

#include <memory>
#include "Customer.h"

class CustomerQueue
{
private:
	Customer** customers;
	int _size;
	int _count;
	int _first;


public:
	CustomerQueue(int size)
	{
		if (size < 1)
		{
			_size = 1;
		}
		else
		{
			_size = size;
		}

		customers = (Customer**)malloc(_size * sizeof(Customer*));
		_count = 0;
		_first = 0;
	}
	virtual ~CustomerQueue()
	{
		free(customers);
	}

	int count() { return _count; }
	int size() { return _size; }

	bool enqueue(Customer* customer)
	{
		if (_count < _size && customer != NULL)
		{
			int index = (_first + _count) % _size;
			customers[index] = customer;
			_count++;
			return true;
		}
		return false;
	}

	Customer* dequeue()
	{
		if (_count > 0)
		{
			int index = _first;
			_first = (_first + 1) % _size;
			_count--;
			return customers[index];
		}
		return NULL;
	}

	Customer* peek()
	{
		if (_count > 0)
		{
			return customers[_first];
		}
		return NULL;
	}
	
	char* toCString()
	{
		char* queueString = (char*)malloc(sizeof(char) * (_size + 1));
		queueString[_size] = '\0';

		int c = _first;
		for (int i = 0; i < _size; i++)
		{
			if (i < _count && customers[c] != NULL)
				queueString[i] = customers[c]->getDisplayChar();
			else
				queueString[i] = '.';
			c = (c+1) % _size;
		}

		return queueString;
	}
};
#pragma once

#include <windows.h>
#include <process.h>
#include <memory>
#include <vector>
#include "Customer.h"


class LaundryMachine;
typedef void(*MachineDoneFunction)(LaundryMachine*, Customer*, void* listener);

class LaundryMachineListener
{
public:
	LaundryMachineListener(MachineDoneFunction f, void* o)
	{
		function = f;
		listeningObject = o;
	}

	MachineDoneFunction function;
	void* listeningObject;
};


class LaundryMachine
{
private:
	Customer* _customer;
	HANDLE _semaphore;
	int _useTime;
	std::vector<LaundryMachineListener*> listeners;


public:
	LaundryMachine(int useTime, HANDLE semaphore)
	{
		_semaphore = semaphore;
		_useTime = useTime;
		_customer = NULL;
	}
	virtual ~LaundryMachine()
	{
	}

	Customer* customer() { return _customer; }

	bool beginUse(Customer* customer)
	{
		if (_customer == NULL)
		{
			_customer = customer;
			_beginthread(&LaundryMachine::UseThreadLauncher, 0, static_cast<void*>(this));
		}
		return false;
	}

	bool inUse() { return _customer != NULL; }

	void addDoneListener(MachineDoneFunction callback, void* listeningObject)
	{
		if (callback == NULL) return;
		if (listeningObject == NULL) return;

		listeners.push_back( new LaundryMachineListener(callback, listeningObject) );
	}

	void removeDoneListener(void* listeningObject)
	{
		if (listeningObject == NULL) return;

		for (unsigned  i = 0; i < listeners.size(); )
		{
			if (listeningObject == listeners.at(i)->listeningObject)
			{
				listeners.erase(listeners.begin() + i);
			}
			else
			{
				i++;
			}
		}
	}


private:
	static void __cdecl UseThreadLauncher(void* o)
	{
		static_cast<LaundryMachine*>(o)->UseThread();
	}

	void UseThread()
	{
		Sleep(_useTime);

		WaitForSingleObject(_semaphore, INFINITE);

		Customer* customer = _customer;
		_customer = NULL;
		for (unsigned  i = 0; i < listeners.size(); i++)
		{
			if (listeners.at(i) != NULL)
			{
				listeners.at(i)->function(this, customer, listeners.at(i)->listeningObject);
			}
		}
		

		ReleaseSemaphore(_semaphore, 1, NULL);
	}

};
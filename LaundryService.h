#pragma once

#include <memory>
#include "LaundryMachine.h"
#include "CustomerQueue.h"
#include "Customer.h"


typedef void(*CustomerCompleteFunction)(Customer* customer);
typedef void(*CustomerDroppedFunction)(Customer* customer);

class LaundryService
{
private:
	CustomerQueue* customerQueue;
	LaundryMachine** machines;
	int _machineCount;

	std::vector<CustomerCompleteFunction> customerCompleteFunctions;
	std::vector<CustomerDroppedFunction> customerDroppedFunctions;

public:
	LaundryService* nextService;

public:
	LaundryService(int queueSize, int machineCount, int machineTime, HANDLE semaphore)
	{
		if (machineCount < 1)
		{
			_machineCount = 1;
		}
		else
		{
			_machineCount = machineCount;
		}

		nextService = NULL;

		customerQueue = new CustomerQueue(queueSize);

		machines = (LaundryMachine**)malloc(_machineCount * sizeof(LaundryMachine*));
		for (int i = 0; i < _machineCount; i++)
		{
			machines[i] = new LaundryMachine(machineTime, semaphore);
			machines[i]->addDoneListener(LaundryService::CustomerFinished, this);
		}
	}
	virtual ~LaundryService()
	{
		for (int i = 0; i < _machineCount; i++)
		{
			delete machines[i];
		}
		free(machines);
		machines = NULL;

		delete customerQueue;
		customerQueue = NULL;
	}

	int machineCount() { return _machineCount; }
	int customersInQueue() { return customerQueue->count(); }
	int customersAtMachines()
	{
		int total = 0;
		for (int  i = 0; i < _machineCount; i++)
		{
			if (machines[i]->inUse())
				total++;
		}
		return total;
	}

	LaundryMachine* getAvailableMachine()
	{
		for (int i = 0; i < _machineCount; i++)
		{
			if (!machines[i]->inUse())
			{
				return machines[i];
			}
		}
		return NULL;
	}
	
	bool addCustomer(Customer* customer)
	{
		LaundryMachine* machine = getAvailableMachine();
		if (machine == NULL)
		{
			if (!customerQueue->enqueue(customer))
			{
				for (unsigned int i = 0; i < customerDroppedFunctions.size(); i++)
				{
					customerDroppedFunctions.at(i)(customer);
				}
				return false;
			}
			return true;
		}
		else
		{
			//machine->addDoneListener(LaundryService::CustomerFinished, this);
			machine->beginUse(customer);
		}
		return true;
	}

	void addCustomerFinishedListener(CustomerCompleteFunction callback)
	{
		if (callback == NULL) return;
		for (unsigned int i = 0; i < customerCompleteFunctions.size(); i++)
		{
			if (callback == customerCompleteFunctions.at(i))
			{
				return;
			}
		}
		customerCompleteFunctions.push_back(callback);
	}

	void removeCustomerFinishedListener(CustomerCompleteFunction callback)
	{
		if (callback == NULL) return;
		for (unsigned int i = 0; i < customerCompleteFunctions.size(); i++)
		{
			if (callback == customerCompleteFunctions.at(i))
			{
				customerCompleteFunctions.erase(customerCompleteFunctions.begin() + i);
				return;
			}
		}
	}

	void addCustomerDroppedListener(CustomerDroppedFunction callback)
	{
		if (callback == NULL) return;
		for (unsigned int i = 0; i < customerDroppedFunctions.size(); i++)
		{
			if (callback == customerDroppedFunctions.at(i))
			{
				return;
			}
		}
		customerDroppedFunctions.push_back(callback);
	}

	void removeCustomerDroppedListener(CustomerDroppedFunction callback)
	{
		if (callback == NULL) return;
		for (unsigned int i = 0; i < customerDroppedFunctions.size(); i++)
		{
			if (callback == customerDroppedFunctions.at(i))
			{
				customerDroppedFunctions.erase(customerDroppedFunctions.begin() + i);
				return;
			}
		}
	}

	char* queueCString()
	{
		return customerQueue->toCString();
	}

	char* machinesCString()
	{
		char* machineString = (char*)malloc(sizeof(char) * (_machineCount + 1));
		machineString[_machineCount] = '\0';

		for (int i = 0; i < _machineCount; i++)
		{
			if (machines[i]->inUse())
				machineString[i] = machines[i]->customer()->getDisplayChar();
			else
				machineString[i] = '.';
		}

		return machineString;
	}

private:
	static void CustomerFinished(LaundryMachine* machine, Customer* customer, void* listeningObject)
	{
		LaundryService* laundryService = (LaundryService*)listeningObject;

		if (laundryService != NULL)
		{
			laundryService->customerFinished(machine, customer);
		}
	}
	
	void customerFinished(LaundryMachine* machine, Customer* customer)
	{
		//machine->removeDoneListener(this);
		if (nextService != NULL)
		{
			nextService->addCustomer(customer);
		}
		else
		{
			for (unsigned int i = 0; i < customerCompleteFunctions.size(); i++)
			{
				customerCompleteFunctions.at(i)(customer);
			}
		}

		Customer* nextInLine = customerQueue->dequeue();
		if (nextInLine != NULL)
		{
			machine->beginUse(nextInLine);
		}
	}

};
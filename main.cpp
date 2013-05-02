#include <iostream>
#include <random>
#include <time.h>

#include "LaundryService.h"



// GLOBAL CONSTANTS
//   time is measure in ms

#define WASHER_ARRIVAL_TIME_MIN 200
#define WASHER_ARRIVAL_TIME_MAX 1000

#define DRYCLEAN_ARRIVAL_TIME_MIN 600
#define DRYCLEAN_ARRIVAL_TIME_MAX 1500

#define WASHER_QUEUE 10
#define WASHER_COUNT 10
#define WASHER_TIME 500

#define DRYER_QUEUE 10
#define DRYER_COUNT 10
#define DRYER_TIME 1000

#define DRYCLEAN_QUEUE 10
#define DRYCLEAN_COUNT 2
#define DRYCLEAN_TIME 1000

#define FOLDING_QUEUE 10
#define FOLDING_COUNT 3
#define FOLDING_TIME 300


// GLOBAL VARIABLES
char* CustomerCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
int CustomerCharacterCount = 26+26+10;

int customersServed = 0;
int washerQueueOverFlow = 0;
int dryerQueueOverFlow = 0;
int drycleanQueueOverFlow = 0;
int foldingQueueOverFlow = 0;
inline int totalOverflow() { return washerQueueOverFlow + dryerQueueOverFlow + drycleanQueueOverFlow + foldingQueueOverFlow; }

HANDLE mainSemaphore;

LaundryService* washers;
LaundryService* dryers;
LaundryService* folders;
LaundryService* dryClean;

bool PrintLock = false;

// GLOBAL FUNCTIONS
void PrintLaundryMat()
{
	if (!PrintLock)
	{
		PrintLock = true;

		system("cls");

		if (customersServed + totalOverflow() > 0)
			std::cout << "Customer Satisfaction: " <<
				(100 * customersServed / (totalOverflow() + customersServed))
				<< "%" << std::endl;
		else
			std::cout << "Customer Satisfaction: --%" << std::endl;
		std::cout << "Happy Customers Served:     " << customersServed << std::endl;
		std::cout << "Customers Who Left Early:   " << totalOverflow() << std::endl;
		std::cout << "  Washer Queue Overflow:    " << washerQueueOverFlow << std::endl;
		std::cout << "  Dryer Queue Overflow:     " << dryerQueueOverFlow << std::endl;
		std::cout << "  Dry Clean Queue Overflow: " << drycleanQueueOverFlow << std::endl;
		std::cout << "  Folding Queue Overflow:   " << foldingQueueOverFlow << std::endl;
		std::cout << std::endl;

		std::cout << "Washing Machines" << std::endl;
		std::cout << "  Customers in Queue:    " << washers->customersInQueue() << std::endl;
		std::cout << "        " << washers->queueCString() << std::endl;
		std::cout << "  Customers at Machines: " << washers->customersAtMachines() << std::endl;
		std::cout << "        " << washers->machinesCString() << std::endl;
		std::cout << std::endl;

		std::cout << "Dryers" << std::endl;
		std::cout << "  Customers in Queue:    " << dryers->customersInQueue() << std::endl;
		std::cout << "        " << dryers->queueCString() << std::endl;
		std::cout << "  Customers at Machines: " << dryers->customersAtMachines() << std::endl;
		std::cout << "        " << dryers->machinesCString() << std::endl;
		std::cout << std::endl;

		std::cout << "Dry Cleaning Service" << std::endl;
		std::cout << "  Customers in Queue:  " << dryClean->customersInQueue() << std::endl;
		std::cout << "        " << dryClean->queueCString() << std::endl;
		std::cout << "  Dry Cleaners in Use: " << dryClean->customersAtMachines() << std::endl;
		std::cout << "        " << dryClean->machinesCString() << std::endl;
		std::cout << std::endl;

		std::cout << "Folding Tables" << std::endl;
		std::cout << "  Customers in Queue: " << folders->customersInQueue() << std::endl;
		std::cout << "        " << folders->queueCString() << std::endl;
		std::cout << "  Tables in Use:      " << folders->customersAtMachines() << std::endl;
		std::cout << "        " << folders->machinesCString() << std::endl;
		std::cout << std::endl;

		PrintLock = false;
	}
}

void CustomerServiceComplete(Customer* customer)
{
	customersServed++;
	delete customer;
	PrintLaundryMat();
}

void WasherQueueOverFlow(Customer* customer)
{
	washerQueueOverFlow++;
	delete customer;
	PrintLaundryMat();
}

void DryerQueueOverFlow(Customer* customer)
{
	dryerQueueOverFlow++;
	delete customer;
	PrintLaundryMat();
}

void DrycleanQueueOverFlow(Customer* customer)
{
	drycleanQueueOverFlow++;
	delete customer;
	PrintLaundryMat();
}

void FoldingQueueOverFlow(Customer* customer)
{
	foldingQueueOverFlow++;
	delete customer;
	PrintLaundryMat();
}


class LaundryServiceThreadData
{
public:
	HANDLE semaphore;
	LaundryService* service;
	int minArrivalTime;
	int maxArrivalTime;
};

void LaundryServiceThread(void* threadData)
{
	LaundryServiceThreadData* data = (LaundryServiceThreadData*) threadData;
	if (data == NULL) return;

	Customer* newCustomer = NULL;
	while (true)
	{
		int waitTime = rand() % (data->maxArrivalTime - data->minArrivalTime) + data->minArrivalTime;
		Sleep(waitTime);

		WaitForSingleObject(data->semaphore, INFINITE);

		newCustomer = new Customer( CustomerCharacters[rand() % CustomerCharacterCount] );
		data->service->addCustomer(newCustomer);
		newCustomer = NULL;

		PrintLaundryMat();

		ReleaseSemaphore(data->semaphore, 1, NULL);
	}
}



int main()
{
	srand( time(NULL) );

	mainSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
	if (mainSemaphore == NULL)
	{
		printf("CreateSemaphore error: %d\n", GetLastError());
        return 1;
	}

	washers = new LaundryService(WASHER_QUEUE, WASHER_COUNT, WASHER_TIME, mainSemaphore);
	dryers = new LaundryService(DRYER_QUEUE, DRYER_COUNT, DRYER_TIME, mainSemaphore);
	dryClean = new LaundryService(DRYCLEAN_QUEUE, DRYCLEAN_COUNT, DRYCLEAN_TIME, mainSemaphore);
	folders = new LaundryService(FOLDING_QUEUE, FOLDING_COUNT, FOLDING_TIME, mainSemaphore);
	washers->nextService = dryers;
	dryers->nextService = folders;
	dryClean->nextService = folders;

	washers->addCustomerDroppedListener(WasherQueueOverFlow);
	dryers->addCustomerDroppedListener(DryerQueueOverFlow);
	dryClean->addCustomerDroppedListener(DrycleanQueueOverFlow);
	folders->addCustomerDroppedListener(FoldingQueueOverFlow);
	folders->addCustomerFinishedListener(CustomerServiceComplete);

	bool quit = false;

	Customer* newCustomer = NULL;

	/*
	while (!quit)
	{
		newCustomer = new Customer( CustomerCharacters[rand() % CustomerCharacterCount] );
		if (rand()%4 == 0)
		{
			if (!dryClean->addCustomer(newCustomer))
			{
				DrycleanQueueOverFlow(newCustomer);
			}
		}
		else
		{
			if (!washers->addCustomer(newCustomer))
			{
				WasherQueueOverFlow(newCustomer);
			}
		}
		newCustomer = NULL;
		PrintLaundryMat();

		int waitTime = rand() % (ARRIVAL_MAX_TIME - ARRIVAL_MIN_TIME) + ARRIVAL_MIN_TIME;
		Sleep(waitTime);
	}
	*/

	LaundryServiceThreadData* washerThreadData = new LaundryServiceThreadData();
	washerThreadData->minArrivalTime = WASHER_ARRIVAL_TIME_MIN;
	washerThreadData->maxArrivalTime = WASHER_ARRIVAL_TIME_MAX;
	washerThreadData->semaphore = mainSemaphore;
	washerThreadData->service = washers;
	_beginthread(&LaundryServiceThread, 0, static_cast<void*>(washerThreadData));

	LaundryServiceThreadData* drycleanThreadData = new LaundryServiceThreadData();
	drycleanThreadData->minArrivalTime = DRYCLEAN_ARRIVAL_TIME_MIN;
	drycleanThreadData->maxArrivalTime = DRYCLEAN_ARRIVAL_TIME_MAX;
	drycleanThreadData->semaphore = mainSemaphore;
	drycleanThreadData->service = dryClean;
	_beginthread(&LaundryServiceThread, 0, static_cast<void*>(drycleanThreadData));


	while (!quit);

	delete washers;
	delete dryers;
	delete dryClean;
	delete folders;
	
	return 0;
}
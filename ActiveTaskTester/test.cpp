#include "pch.h"
#include "ActiveTask.h"

class CActiveTest : ActiveTask {

public:
	CActiveTest()
	{
	};

	void Execute(std::function<void()> _func)
	{
		ExecuteOnMyTask(_func);
	}

	template<typename Function, typename... Arguments>
	void Execute(Function func, Arguments... parameters)
	{
		ExecuteOnMyTask(func, std::forward<Arguments>(parameters)...);
	}

	auto GetID()
	{
		return GetMyTaskID();
	}

	auto GetWorkCount()
	{
		return GetMyWorkCount();
	}

};

void ReSchedule()
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(1ms);
}

TEST(Logic, OwnSeperateTask)
{
	CActiveTest test;
	EXPECT_FALSE(std::this_thread::get_id() == test.GetID());
}

TEST(Logic, SeperateTaskWorks)
{
	CActiveTest test;

	std::thread::id initiallyMainThread = std::this_thread::get_id();
	test.Execute([&]() {
		initiallyMainThread = std::this_thread::get_id();
	});

	ReSchedule();
	EXPECT_FALSE(std::this_thread::get_id() == initiallyMainThread);
}

TEST(Logic, MainTaskDoesntWaitOnSeperateTask)
{
	auto start = std::chrono::high_resolution_clock::now();
	
	CActiveTest test;
	test.Execute([]() {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(1000ms);
		});
	
	std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - start;
	EXPECT_TRUE(elapsed.count() < 1000);
}

/*TEST(Logic, CreateDataRace)
{
	uint32_t sharedResource = 0;
	constexpr uint32_t activeObjectCount = 200;
	constexpr uint32_t iterateCount = 1000;

	CActiveTest activeObjects[activeObjectCount];

	for (uint32_t i = 0; i < activeObjectCount; i++)
	{
		for (uint32_t j = 0; j < iterateCount; j++)
		{
			activeObjects[i].Execute([&]() {
				uint32_t cache = sharedResource;
				cache++;
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(0ms);		//for context switch
				sharedResource = cache;
			});
		}
	}
	for (uint32_t i = 0; i < activeObjectCount; i++)
	{
		while (activeObjects[i].GetWorkCount() > 0)
		{
			ReSchedule();
		}
	}
	
	EXPECT_NE(sharedResource, (activeObjectCount * iterateCount));
}*/

TEST(Logic, SolvedDataRace)
{
	uint32_t sharedResource = 0;
	constexpr uint32_t activeObjectCount = 200;
	constexpr uint32_t iterateCount = 1000;

	std::mutex mtx;

	CActiveTest activeObjects[activeObjectCount];
	for (uint32_t i = 0; i < activeObjectCount; i++)
	{
		for (uint32_t j = 0; j < iterateCount; j++)
		{
			activeObjects[i].Execute([&]() {
				std::lock_guard<std::mutex> g(mtx);
				uint32_t cache = sharedResource;
				cache++;
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(0ms);		//for context switch
				sharedResource = cache;
			});
		}
	
	}
	for (uint32_t i = 0; i < activeObjectCount; i++)
	{
		while (activeObjects[i].GetWorkCount() > 0)
		{
			ReSchedule();
		}
	}

	EXPECT_EQ(sharedResource, (activeObjectCount * iterateCount));
}

uint32_t counter;
void FuncWithNoArg()
{
	counter++;
}
TEST(Functionality, ExecuteFuncWithNoParams)
{
	uint32_t before = counter;
	
	CActiveTest test;
	test.Execute(&FuncWithNoArg);

	ReSchedule();
	
	
	EXPECT_FALSE(before == counter);
}

uint32_t initiallyZero = 0;
void FuncWithArg(uint32_t rvalue)
{
	initiallyZero = rvalue;
}
TEST(Functionality, ExecuteLambda)
{
	CActiveTest test;
	test.Execute([]() {
		FuncWithArg(5);
	});

	ReSchedule();

	EXPECT_TRUE(initiallyZero == 5);
}

TEST(Functionality, ExecuteFuncWithArgs)
{
	CActiveTest test;
	test.Execute(FuncWithArg, 5);

	ReSchedule();

	EXPECT_TRUE(initiallyZero == 5);
}
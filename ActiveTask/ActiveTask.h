#pragma once
#include <mutex>
#include <queue>
#include <functional>

class ActiveTask
{
private:
	bool alive = true;
	std::mutex mtx;
	std::condition_variable cv;
	std::queue<std::function<void()>> queWork;
	std::thread thr;

protected:
	inline ActiveTask()
	{
		thr = std::thread([&]() {
			while (alive)
			{
				std::unique_lock<std::mutex> ul(mtx);
				cv.wait(ul, [&]() {return ((false == queWork.empty()) || (false == alive)); });
				if (alive == false)
				{
					ul.unlock();
					break;
				}
				std::function<void()> func = queWork.front();
				std::invoke(func);
				queWork.pop();
				ul.unlock();
			}
		});
	}

	inline virtual ~ActiveTask()
	{
		alive = false;
		cv.notify_one();
		if (thr.joinable())
		{
			thr.join();
		}
	}

	template<typename Function, typename... Arguments>
	inline void ExecuteOnMyTask(Function func, Arguments... parameters)
	{
		std::function<void()> _func{ std::bind(func, parameters...) };
		{
			std::lock_guard<std::mutex> lk(mtx);
			queWork.push(_func);
		}
		cv.notify_one();			//notifying doesnt require thread-safety
	}

	inline size_t GetMyWorkCount()
	{
		size_t workCount = 0;
		{
			std::lock_guard<std::mutex> lk(mtx);
			workCount = queWork.size();
		}
		return workCount;
	}

	inline std::thread::id GetMyTaskID()
	{
		return thr.get_id();
	}

};
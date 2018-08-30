#ifndef __GMASYNC_H__
#define __GMASYNC_H__
#include <gmcommon.h>
#include <future>
#include <type_traits>
BEGIN_NS

template <typename T>
using GMFuture = std::future<T>;

class GMAsync
{
public:
	enum LaunchPolicy
	{
		Async = 0x1,
		Deferred = 0x2
	};

	template <typename Function, typename... Args>
	static auto async(LaunchPolicy policy, Function&& function, Args&&... args)
	{
		return std::async(static_cast<std::launch>(policy), std::forward<Function>(function), std::forward<Args>(args)...);
	}

	template <typename Iter, typename Function>
	static void blockedAsync(LaunchPolicy policy, GMsize_t taskCount, Iter iterBegin, Iter iterEnd, Function&& function)
	{
		GM_STATIC_ASSERT(std::is_same<std::random_access_iterator_tag, std::iterator_traits<Iter>::iterator_category>::value, "Iterator must be a random access iterator");
		typedef decltype(async(Async, std::forward<Function>(function), Iter(), Iter())) FutureType;

		// 用于缓存Async状态的future，否则future析构时将会进行等待
		Vector<FutureType> futures;
		futures.reserve(taskCount);

		GMsize_t len = iterEnd - iterBegin;
		GMsize_t step = (iterEnd - iterBegin) / taskCount;
		if (taskCount > len)
			taskCount = len;

		Iter iter = iterBegin;
		for (GMsize_t i = 0; i < taskCount; ++i)
		{
			if (i < taskCount - 1)
			{
				futures.push_back(std::move(async(policy, std::forward<Function>(function), iter, iter + step)));
				iter += step;
			}
			else
			{
				futures.push_back(std::move(async(policy, std::forward<Function>(function), iter, iterEnd)));
			}
		}

		for (auto& future : futures)
		{
			future.wait();
		}
	}
};

GM_STATIC_ASSERT(GMAsync::Async == static_cast<GMint>(std::launch::async), "LaunchPolicy must be same with std::luanch");
GM_STATIC_ASSERT(GMAsync::Deferred == static_cast<GMint>(std::launch::deferred), "LaunchPolicy must be same with std::luanch");

// 同步
class GMAsyncResult
{
public:
	virtual void* state();
	virtual bool isComplete();
	virtual void wait();

public:
	void setFuture(GMFuture<void> future);
	void setComplete() GM_NOEXCEPT
	{
		complete = true;
	}

private:
	GMBuffer buffer;
	GMFuture<void> future;
	bool complete = false;
};

using GMAsyncCallback = std::function<void(GMAsyncResult*)>;

END_NS
#endif
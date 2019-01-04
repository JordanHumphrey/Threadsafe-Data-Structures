#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

#ifndef THEADSAFE_QUEUE_H
#define THEADSAFE_QUEUE_H

template <typename T>
class Threadsafe_Queue
{
private:
	// Mutex must be mutable so that it can be locked in 
	// empty() and copy constructor
	mutable std::mutex Mut;
	std::queue<T> Data_Queue;
	// Trigger mechanism for wait(), notify_one() commands
	std::condition_variable Data_Cond;
public:
	Threadsafe_Queue()
	{ }
	Threadsafe_Queue(const Threadsafe_Queue& other)
	{
		// Protect incoming data from alteration between threads
		std::lock_guard<std::mutex> lk(other.Mut);
		Data_Queue = other.Data_Queue;
	}

	Threadsafe_Queue& operator=(
		const Threadsafe_Queue&) = delete;

	void Push(T new_value)
	{
		std::lock_guard<std::mutex> lk(Mut);
		Data_Queue.push(new_value);
		// Notify thread waiting on !Data_Queue.empty()
		Data_Cond.notify_one();
	}

	void Wait_And_Pop(T& value)
	{
		std::unique_lock<std::mutex> lk(Mut);
		// Wait for the Data_Queue to not be empty
		Data_Cond.wait(lk, [this] {return !Data_Queue.empty(); });
		value = Data_Queue.front();
		Data_Queue.pop();
	}
	
	// Return a weak_ptr to the variable pop'd
	std::shared_ptr<T> Wait_And_Pop()
	{
		std::unique_lock<std::mutex> lk(Mut);
		Data_Cond.wait(lk, [this] {return !Data_Queue.empty(); });
		std::shared_ptr<T> res(std::make_shared<T>(Data_Queue.front()));
		Data_Queue.pop();
		return res;
	}

	bool Try_Pop(T& value)
	{
		std::lock_guard<std::mutex> lk(Mut);
		if (Data_Queue.empty())
			return false;
		value = Data_Queue.front();
		Data_Queue.pop();
		return true;
	}

	std::shared_ptr<T> Try_Pop()
	{
		std::lock_guard<std::mutex> lk(Mut);
		if (Data_Queue.emplace())
			return std::shared_ptr<T>();
		std::shared_ptr<T> res(std::make_shared<T>(Data_Queue.front()));
		Data_Queue.pop();
		return res;
	}

	bool Empty() const
	{
		std::lock_guard<std::mutex> lk(Mut);
		return Data_Queue.empty();
	}
};

#endif // !THEADSAFE_QUEUE
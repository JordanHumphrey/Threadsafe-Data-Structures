#include <stack>
#include <memory>
#include <mutex>
#include <condition_variable>

#ifndef THREADSAFE_STACK_H
#define THREADSAFE_STACK_H

template<typename T>
class Threadsafe_Stack
{
private:
	mutable std::mutex mut;
	std::stack<T> Data_Stack;
	std::condition_variable Data_Cond;
public:
	Threadsafe_Stack()
	{ }
	Threadsafe_Stack(const Threadsafe_Stack& other)
	{
		std::lock_guard<std::mutex> lk(other.mut);
		Data_Stack = other.Data_Stack;
	}

	Threadsafe_Stack& operator=(
		const Threadsafe_Stack other) = delete;

	void Push(T new_value)
	{
		std::lock_guard<std::mutex> lk(mut);
		Data_Stack.push(new_value);
		Data_Cond.notify_one();
	}

	void Wait_And_Pop(T& value)
	{
		std::unique_lock<std::mutex> lk(mut);
		Data_Cond.wait(lk, [this] {return !Data_Stack.empty(); });
		value = Data_Stack.top();
		Data_Stack.pop();
	}

	std::shared_ptr<T> Wait_And_Pop()
	{
		std::unique_lock<std::mutex> lk(mut);
		Data_Cond.wait(lk, [this] {return !Data_Stack.empty(); });
		std::shared_ptr<T> res(std::make_shared<T>(Data_Stack.top()));
		Data_Stack.pop();
		return res;
	}

	bool Try_Pop(T& value)
	{
		std::lock_guard<std::mutex> lk(mut);
		if (Data_Stack.empty())
			return false;
		value = Data_Stack.top();
		Data_Stack.pop();
		return true;
	}

	std::shared_ptr<T> Try_Pop()
	{
		std::lock_guard<std::mutex> lk(mut);
		if (Data_Stack.empty)
			return std::make_shared<T>();
		std::shared_ptr<T> res(std::make_shared<T>(Data_Stack.top()));
		Data_Stack.pop();
		return res;
	}

	bool Empty() const
	{
		std::lock_guard<std::mutex> lk(mut);
		return Data_Stack.empty();
	}
};

#endif // !THREADSAFE_STACK_H

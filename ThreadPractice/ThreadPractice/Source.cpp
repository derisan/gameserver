#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
using namespace std;
using namespace std::chrono;

volatile int sum = 0;
constexpr int LOOP_COUNT = 5000'0000;
constexpr int THREAD_COUNT = 2;
mutex g_mutex;

void ThreadFunc()
{
	int loop = LOOP_COUNT / THREAD_COUNT;

	volatile int local_sum = 0;
	for (int i = 0; i < loop; ++i)
	{
		local_sum += 2;
	}

	lock_guard<mutex> guard(g_mutex);
	sum += local_sum;
}

int main()
{
	auto start = high_resolution_clock::now();
	for (int i = 0; i < LOOP_COUNT; ++i)
	{
		sum += 2;
	}
	auto elapsed = high_resolution_clock::now() - start;

	cout << sum << "\n";
	cout << duration_cast<milliseconds>(elapsed).count() << "msecs.\n";

	sum = 0;

	start = high_resolution_clock::now();
	vector<thread> threads;
	for (int i = 0; i < THREAD_COUNT; ++i)
	{
		threads.emplace_back(ThreadFunc);
	}

	for (auto& th : threads)
	{
		if (th.joinable())
		{
			th.join();
		}
	}
	elapsed = high_resolution_clock::now() - start;

	cout << sum << "\n";
	cout << duration_cast<milliseconds>(elapsed).count() << "msecs.\n";

}
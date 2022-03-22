#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

constexpr int T_SIZE = 1000'0000;

#define abs(x) (((x) > 0 )? (x) : (-x))

int abs2(int x)
{
	int y = x >> 31;
	return (x ^ y) - y;
}

int rand_arr[T_SIZE];

int main()
{
	int sum;

	for (int i = 0; i < T_SIZE; ++i) rand_arr[i] = rand() - 16384;
	sum = 0;
	auto start_t = high_resolution_clock::now();
	for (int i = 0; i < T_SIZE; ++i) sum += abs(rand_arr[i]);
	auto du = high_resolution_clock::now() - start_t;
	cout << "[abs] Time " << duration_cast<milliseconds>(du).count() << " ms\n";
	cout << "Result : " << sum << endl;
	sum = 0;
	start_t = high_resolution_clock::now();
	for (int i = 0; i < T_SIZE; ++i) sum += abs2(rand_arr[i]);
	du = high_resolution_clock::now() - start_t;
	cout << "[abs2] Time " << duration_cast<milliseconds>(du).count() << " ms\n";
	cout << "Result : " << sum << endl;
}
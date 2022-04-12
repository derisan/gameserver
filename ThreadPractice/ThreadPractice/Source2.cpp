//#include <mutex>
//#include <iostream>
//
//using namespace std;
//
//recursive_mutex gLock;
//
//void Bar();
//
//void Foo()
//{
//	lock_guard<recursive_mutex> guard(gLock);
//
//	cout << "Foo()" << endl;
//
//	Bar();
//}
//
//void Bar()
//{
//	lock_guard<recursive_mutex> guard(gLock);
//
//	cout << "Bar()" << endl;
//}
//
//int main()
//{
//	Foo();
//}
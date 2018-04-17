#include "XORList.h"
#include "Utils.h"
#include <string>

int main() {
	LinkedList<double> *test_list = new LinkedList<double>();
	auto test  = test_list->begin();
	const double test1 = 10.0;
	//test1 = "xyi";
	test_list->push_back(test1);
	auto test3 = test_list->back();
	test_list->push_back(20.0);
	auto test31 = test_list->back();
	test_list->push_back(30.0);
	auto test32 = test_list->back();
	test_list->pop_back();
	auto test5 = test_list->back();
	auto test6 = test_list->front();
	return 0;
}
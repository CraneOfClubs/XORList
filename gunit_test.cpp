#define DEBUG_TEST

#include "XORList.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"
#include <memory>
#include <list>
#include <chrono>
#include <iostream>

class XOR : public testing::Test
{
protected:
	std::vector<int> *_test_vector;
	void SetUp()
	{
		_test_vector = new std::vector<int>({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 });
	}

	void TearDown()
	{
		delete _test_vector;
	}

};

template<typename T>
struct SwapAllocator : std::allocator<T>
{
public:
	struct propagate_on_container_move_assignment : std::false_type {};

	template<typename R>
	struct rebind
	{
		using other = SwapAllocator<R>;
	};


	using std::allocator<T>::allocator;


	bool operator==(const SwapAllocator &) const noexcept
	{
		return false;
	}
};

template<typename T>
struct TestAllocator1 : std::allocator<T>
{
public:
	struct propagate_on_container_move_assignment : std::false_type {};

	template<typename R>
	struct rebind
	{
		using other = TestAllocator1<R>;
	};


	using std::allocator<T>::allocator;
};


template<typename T>
struct SwapAllocator2 : std::allocator<T>
{
public:
	struct propagate_on_container_swap : std::true_type {};

	template<typename R>
	struct rebind
	{
		using other = SwapAllocator2<R>;
	};


	using std::allocator<T>::allocator;
};

//-------------------TESTS

TEST(XOR, Constructors)
{
	LinkedList<int> listDefault;

	ASSERT_TRUE(listDefault.empty());
	ASSERT_THAT(listDefault, testing::ElementsAre());

	LinkedList<int> listAlloc(std::allocator<int>{});

	ASSERT_TRUE(listAlloc.empty());
	ASSERT_THAT(listAlloc, testing::ElementsAre());

	LinkedList<int> listInitList{ -10, 20, 1 };

	ASSERT_EQ(listInitList.size(), 3);
	ASSERT_THAT(listInitList, testing::ElementsAre(-10, 20, 1));

	LinkedList<int> listValue(5, 1337);

	ASSERT_EQ(listValue.size(), 5);
	ASSERT_THAT(listValue, ::testing::ElementsAre(1337, 1337, 1337, 1337, 1337));

	LinkedList<int> listCopy{ 1, 2, 3 };
	LinkedList<int> listCopy2(listCopy);

	ASSERT_EQ(listCopy2.size(), 3);
	ASSERT_THAT(listCopy2, ::testing::ElementsAre(1, 2, 3));

	listCopy.~LinkedList();

	ASSERT_EQ(listCopy2.size(), 3);
	ASSERT_THAT(listCopy2, ::testing::ElementsAre(1, 2, 3));

	LinkedList<int> listMove{ 1, 2, 3 };
	LinkedList<int> listMove2(std::move(listMove));

	ASSERT_EQ(listMove.size(), 0);
	ASSERT_THAT(listMove, ::testing::ElementsAre());

	ASSERT_EQ(listMove2.size(), 3);
	ASSERT_THAT(listMove2, ::testing::ElementsAre(1, 2, 3));
}

TEST(XOR, Merge) {
	LinkedList<int> list1{ 1, 2, 3, 4 }; 
	LinkedList<int> list2{ 1, 2, 3, 4 };

	list2.merge(list1);

	ASSERT_TRUE(list1.empty());
	ASSERT_THAT(list1, ::testing::ElementsAre());

	ASSERT_EQ(list2.size(), 8);
	ASSERT_THAT(list2, ::testing::ElementsAre(1, 1, 2, 2, 3, 3, 4, 4));
	list2.merge(list2);
	ASSERT_EQ(list2.size(), 8);
	ASSERT_THAT(list2, ::testing::ElementsAre(1, 1, 2, 2, 3, 3, 4, 4));

	LinkedList<int> list3{ 322 }, list4{ -322 };

	list4.merge(list3);

	ASSERT_TRUE(list3.empty());
	ASSERT_THAT(list3, ::testing::ElementsAre());

	ASSERT_EQ(list4.size(), 2);
	ASSERT_THAT(list4, ::testing::ElementsAre(-322, 322));
}

TEST(XOR, Sort)
{
	LinkedList<int> list{ 1337, 42, 101, 7, -15 };

	list.sort();

	ASSERT_EQ(list.size(), 5);
	ASSERT_THAT(list, ::testing::ElementsAre(-15, 7, 42, 101, 1337));

	LinkedList<int> list2;

	list2.sort();

	ASSERT_TRUE(list2.empty());
	ASSERT_THAT(list2, ::testing::ElementsAre());
}

TEST(XOR, Unique)
{
	LinkedList<int> list{1337, 1337 };

	list.unique();

	ASSERT_EQ(list.size(), 1);
	ASSERT_THAT(list, ::testing::ElementsAre(1337));

	LinkedList<int> list2{ 1337, 1337, 322 };

	list2.unique();

	ASSERT_EQ(list2.size(), 2);
	ASSERT_THAT(list2, ::testing::ElementsAre(1337, 322));
}

TEST(XOR, Assign)
{
	LinkedList<int> list;

	list.assign(7, -322);

	ASSERT_EQ(list.size(), 7);
	ASSERT_THAT(list, ::testing::ElementsAre(-322, -322, -322, -322, -322, -322, -322));
	list.assign(9, 1337);
	ASSERT_EQ(list.size(), 9);
	ASSERT_THAT(list, ::testing::ElementsAre(1337, 1337, 1337, 1337, 1337, 1337, 1337, 1337, 1337));

	LinkedList<int> list1{ -45 };

	list1.assign(0, -13);

	ASSERT_TRUE(list1.empty());
	ASSERT_THAT(list1, ::testing::ElementsAre());

	LinkedList<int> list2{ -45, 2, -2, 1, 1 };

	list2.assign(1, -13);

	ASSERT_EQ(list2.size(), 1U);
	ASSERT_THAT(list2, ::testing::ElementsAre(-13));
	//
	LinkedList<int> list3{ -4, 3, -2, 1, 2 };

	list3.assign((int*)nullptr, (int*)nullptr);

	ASSERT_TRUE(list3.empty());
	ASSERT_THAT(list3, ::testing::ElementsAre());

	LinkedList<int> list4{ -4, 3, -2, 1, 2 };

	{
		int range[1] = { -42 };
		list4.assign(&range[0], ((int*)range) + 1);
	}

	ASSERT_EQ(list4.size(), 1U);
	ASSERT_THAT(list4, ::testing::ElementsAre(-42));
}

TEST(XOR, PushEmplace) {
	const int values[4] = { 10, 1 };
	LinkedList<int> listPush1;
	listPush1.push_back(values[0]);
	listPush1.push_front(values[1]);
	const auto &listRef = listPush1;
	ASSERT_EQ(listRef.front(), values[1]);
	ASSERT_EQ(listRef.back(), values[0]);
	LinkedList<int> listPush;

	listPush.push_back(0);
	listPush.push_front(-1);
	listPush.push_back(1);
	listPush.push_back(22);
	listPush.push_back(-1);

	ASSERT_EQ(listPush.size(), 5);
	ASSERT_THAT(listPush, ::testing::ElementsAre(-1, 0, 1, 22, -1));

	LinkedList<int> listEmplace;

	listEmplace.emplace_back(0);
	listEmplace.emplace_front(-1);
	listEmplace.emplace_back(1);
	listEmplace.emplace_back(22);
	listEmplace.emplace_back(-1);

	ASSERT_EQ(listEmplace.size(), 5);
	ASSERT_THAT(listEmplace, ::testing::ElementsAre(-1, 0, 1, 22, -1));
}

TEST(XOR, Pop) {
	LinkedList<int> list{ 322, 1337, 42 };

	list.pop_front();
	ASSERT_EQ(list.size(), 2);
	ASSERT_THAT(list, ::testing::ElementsAre(1337, 42));

	list.pop_back();
	ASSERT_EQ(list.size(), 1);
	ASSERT_THAT(list, ::testing::ElementsAre(1337));

	list.pop_back();
	ASSERT_TRUE(list.empty());
	ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(XOR, FrontBack) {
	LinkedList<int> list{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	ASSERT_EQ(list.back(), 9);
	ASSERT_EQ(list.front(), 1);
}

TEST(XOR, Reverse) {
	LinkedList<int> list{ 322, 223, 1337, 42 };

	list.reverse();
	ASSERT_EQ(list.size(), 4);
	ASSERT_THAT(list, ::testing::ElementsAre(42, 1337, 223, 322));
}

TEST(XOR, Splice) {
	LinkedList<int> list1{ 23, 45, 42, 1 };
	LinkedList<int> list2{ 322, 1337 };

	list1.splice(list1.cend(), list2);

	ASSERT_EQ(list1.size(), 6);
	ASSERT_THAT(list1, ::testing::ElementsAre(23, 45, 42, 1, 322, 1337));

	ASSERT_TRUE(list2.empty());
	ASSERT_THAT(list2, ::testing::ElementsAre());
}

TEST(XOR, SomeMovement) {
	LinkedList<int> list1(5);
	ASSERT_EQ(list1.size(), 5);
	LinkedList<int> list2(2);
	list1 = list2;
	ASSERT_EQ(list1.size(), 2);
	ASSERT_EQ(list2.size(), 2);
	list1 = std::move(list2);
	ASSERT_EQ(list2.size(), 0);
}

TEST(XOR, SomeUtility) {
	LinkedList<int> list1(5);
	list1.resize(3);
	ASSERT_EQ(list1.size(), 3);
	list1.resize(0);
	ASSERT_EQ(list1.size(), 0);

	LinkedList<int, SwapAllocator<int>> list3, list4{ 322, 42, 1337 };

	list3 = list4;

	ASSERT_EQ(list3.size(), 3);
	ASSERT_THAT(list3, ::testing::ElementsAre(322, 42, 1337));
}

TEST(XOR, CopyAlloc2) {
	LinkedList<int, SwapAllocator2<int>> list3, list4{ 322, 42, 1337 };

	list3 = list4;

	ASSERT_EQ(list3.size(), 3);
	ASSERT_THAT(list3, ::testing::ElementsAre(322, 42, 1337));
}

TEST(XOR, MoveAlloc1) {
	LinkedList<int, SwapAllocator2<int>> list3, list4{ 322, 42, 1337 };

	list3 = std::move(list4);

	ASSERT_EQ(list3.size(), 3);
	ASSERT_THAT(list3, ::testing::ElementsAre(322, 42, 1337));
}

TEST(XOR, MoveAlloc2) {
	LinkedList<int, SwapAllocator<int>> list3, list4{ 322, 42, 1337 };

	list3 = std::move(list4);

	ASSERT_EQ(list3.size(), 3);
	ASSERT_THAT(list3, ::testing::ElementsAre(322, 42, 1337));
}

TEST(XOR, Swap) {
	LinkedList<int> list1{ 322, 42, 1337 }, list2{ 11, 99 };

	list1.swap(list2);

	ASSERT_EQ(list1.size(), 2);
	ASSERT_THAT(list1, ::testing::ElementsAre(11, 99));

	ASSERT_EQ(list2.size(), 3);
	ASSERT_THAT(list2, ::testing::ElementsAre(322, 42, 1337));
}

TEST(XOR, SwapEmpty) {
	LinkedList<int> list1, list2{ 11, 99 };

	list1.swap(list2);

	ASSERT_EQ(list1.size(), 2);
	ASSERT_THAT(list1, ::testing::ElementsAre(11, 99));

	ASSERT_EQ(list2.empty(), true);
	ASSERT_THAT(list2, ::testing::ElementsAre());
}

TEST(XOR, SwapAlloc1) {
	LinkedList<int, SwapAllocator<int>> list1{ 322, 42, 1337 }, list2{ 11, 99 };

	list1.swap(list2);

	ASSERT_EQ(list1.size(), 2);
	ASSERT_THAT(list1, ::testing::ElementsAre(11, 99));

	ASSERT_EQ(list2.size(), 3);
	ASSERT_THAT(list2, ::testing::ElementsAre(322, 42, 1337));
}

TEST(XOR, SwapAlloc2) {
	LinkedList<int, SwapAllocator2<int>> list1{ 322, 42, 1337 }, list2{ 11, 99 };

	list1.swap(list2);

	ASSERT_EQ(list1.size(), 2);
	ASSERT_THAT(list1, ::testing::ElementsAre(11, 99));

	ASSERT_EQ(list2.size(), 3);
	ASSERT_THAT(list2, ::testing::ElementsAre(322, 42, 1337));
}

TEST(XOR, SpliceEnd)
{
	LinkedList<int> list1{ 1, -1, 2, -2 }, list2{ 13, 123, 45, 44 };

	list1.splice(list1.cend(), list2, ++++list2.cbegin());

	ASSERT_EQ(list1.size(), 5);
	ASSERT_THAT(list1, ::testing::ElementsAre(1, -1, 2, -2, 45));

	ASSERT_EQ(list2.size(), 3);
	ASSERT_THAT(list2, ::testing::ElementsAre(13, 123, 44));
}

TEST(XOR, SpliceMiddle)
{
	LinkedList<int> list1{ 1, -1, 2, -2 }, list2{ 13, 123, 45, 44 };

	list1.splice(++list1.cbegin(), list2, list2.cbegin());

	ASSERT_EQ(list1.size(), 5);
	ASSERT_THAT(list1, ::testing::ElementsAre(1, 13, -1, 2, -2));

	ASSERT_EQ(list2.size(), 3);
	ASSERT_THAT(list2, ::testing::ElementsAre(123, 45, 44));
}

TEST(XOR, SpliceMixItself)
{
	LinkedList<int> list{ 2, 3, 4, 5 };

	list.splice(list.cbegin(), list, ++list.cbegin(), list.cend());

	ASSERT_EQ(list.size(), 4);
	ASSERT_THAT(list, ::testing::ElementsAre(3, 4, 5, 2));
}

TEST(XOR, Iterators)
{
	LinkedList<std::string> list;

	list.emplace_back("test1");
	list.emplace_back("test2");
	list.emplace_back("test3");
	list.emplace_back("test4");

	auto iter = list.cend();
	iter--;

	ASSERT_EQ(*iter, "test4");

	auto iter2 = iter--;

	ASSERT_EQ(*iter2, "test4");
	ASSERT_EQ(*iter, "test3");
}

TEST(LIST, Splice2)
{
	LinkedList<int> list1, list2;

	list1.splice(list1.cbegin(), list2);

	ASSERT_TRUE(list1.empty());
	ASSERT_THAT(list1, ::testing::ElementsAre());

	ASSERT_TRUE(list2.empty());
	ASSERT_THAT(list2, ::testing::ElementsAre());

	list1.splice(list1.cend(), list2);


	ASSERT_TRUE(list1.empty());
	ASSERT_THAT(list1, ::testing::ElementsAre());

	ASSERT_TRUE(list2.empty());
	ASSERT_THAT(list2, ::testing::ElementsAre());
	list1.reverse();
	list1.unique();
	list2.insert(list2.cbegin(), list1.begin(), list1.begin());
	ASSERT_TRUE(list1.empty());
	ASSERT_THAT(list1, ::testing::ElementsAre());

	LinkedList<int> list3, list4{ 1 };

	list3.splice(list3.cend(), list4, list4.cbegin());

	ASSERT_EQ(list3.size(), 1);
	ASSERT_THAT(list3, ::testing::ElementsAre(1));

	ASSERT_TRUE(list4.empty());
	ASSERT_THAT(list4, ::testing::ElementsAre());

	LinkedList<int> list5{ 8 };

	list5.splice(list5.cend(), list5, list5.cbegin());

	ASSERT_EQ(list5.size(), 1);
	ASSERT_THAT(list5, ::testing::ElementsAre(8));

	LinkedList<int> list6{ 1, 2, 3, 4, 5 }, list7;

	list6.splice(list6.cbegin(), list7, list7.cbegin(), list7.cend());

	ASSERT_EQ(list6.size(), 5);
	ASSERT_THAT(list6, ::testing::ElementsAre(1, 2, 3, 4, 5));

	ASSERT_TRUE(list7.empty());
	ASSERT_THAT(list7, ::testing::ElementsAre());
}

//---------------------------------




TEST(LIST, MoveAssignment)
{
	LinkedList<int, TestAllocator1<int>> list1{ 1, 2 }, list2{ 10, 20 };

	list1 = std::move(list2);

	ASSERT_TRUE(list2.empty());
	ASSERT_THAT(list2, ::testing::ElementsAre());

	ASSERT_EQ(list1.size(), 2);
	ASSERT_THAT(list1, ::testing::ElementsAre(10, 20));
}

TEST(LIST, IterShieet) {
	LinkedList<int> list1{ 1, 2, 3, 4, 5 }, list2;
	auto test = list1.cbegin();
	//*test = 10;
}





void std_list() {

	std::list<int>* std_list = new std::list<int>(100000);
	LinkedList<int>* my_list = new LinkedList<int>(100000);
	auto iter2 = my_list->begin();
	for (auto iter = std_list->begin() ; iter != std_list->end(); iter++, iter2++) {
		int randVal = rand() % 100;
		*iter = randVal;
		*iter2 = randVal;
	}
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	std_list->sort();
	end = std::chrono::system_clock::now();
	int elapsed_mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << elapsed_mseconds << std::endl;

	start = std::chrono::system_clock::now();
	my_list->sort();
	end = std::chrono::system_clock::now();
	elapsed_mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << elapsed_mseconds << std::endl;
	//Xlist<int>* std_list = new std::list<int>();
}

void splice_copy() {
	int test = 0;
	int testl2 = 0;
	LinkedList<int>* my_list2 = new LinkedList<int>(1000000);
	LinkedList<int>* my_list = new LinkedList<int>(1000000);
	test = my_list->amount_of_copy;
	testl2 = my_list2->amount_of_copy;
	auto iter2 = my_list->begin();
	for (auto iter = my_list2->begin(); iter != my_list2->end(); iter++, iter2++) {
		int randVal = rand() % 100;
		*iter = randVal;
		*iter2 = randVal;
	}
	my_list->splice(my_list->cbegin(), *my_list2);
	int test2 = my_list->amount_of_copy;
	int test3 = my_list2->amount_of_copy;
}


int main(int argc, char *argv[]) 
{
	srand(time(NULL));
	//std_list();	
	//splice_copy();
	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
	return 0;
}

#ifndef TESTS_HPP
#define TESTS_HPP

#include <iostream>
#include <chrono>

struct Stopwatch {
    std::chrono::steady_clock::time_point begin;
    float duration_millis;

    void start() {
        begin = std::chrono::steady_clock::now();
    }

    float click() {
        auto click = std::chrono::steady_clock::now();
        duration_millis = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(click - begin).count();
        return duration_millis;
    }
};

class Test {

public:

	static void testFileLoad();
	static void testInput();
	static void testVector();
	static void testMemoryPool();

	static void testECS();
	static void testEntityManager();
	static void testEntityManagerGetEntities();
	static void testComponentManager();
	static void testSignatureConversion();
	static void testEntityComponent();
	static void testComponentManagerOperate();

	static void testSparseSet();
	static void testSparseSetAddRetrieve();
	static void testSparseSetDelete();
	static void testSparseSetAssign();
	static void testSparseSetClear();
	static void testSparseSetPerformance();
	static void testSparseSetStructHierarchy();

	static void test(int a, std::string b);
	static void testLambda();

	static void testAll();
};

#endif //TESTS_HPP

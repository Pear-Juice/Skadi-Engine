#include <iostream>
#include "Source/App.hpp"
#include "Test/Tests.hpp"

int main() {
	Test::testAll();

	App skadiApp("Boo!");
}

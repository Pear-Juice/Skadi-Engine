#include "Tests.hpp"

#include <assert.h>
#include <Source/Resources/Vector.hpp>

#include "Source/Core/ECS/ECS.hpp"
#include "Source/Core/DataStorage/SparseSet.hpp"
#include "Source/Core/Messaging/Event.hpp"
#include "Source/Core/Messaging/Lambda.hpp"

void Test::testAll() {
	std::cout << "---Test All---\n";

	testECS();
	testMessaging();

	std::cout << "---Success---\n";
}

void Test::testSparseSet() {
	testSparseSetAddRetrieve();
	testSparseSetDelete();
	testSparseSetAssign();
	testSparseSetClear();
	testSparseSetStructHierarchy();
}

void Test::testSparseSetAddRetrieve() {
	std::array<std::string, 6> sheepCase = {"Boop","Beep","Sheep","I","Am","Sheep"};
	SparseSet<std::string> set(10);
	set.add(0, "Boop");
	set.add(1,"Beep");
	set.add(2, "Sheep");
	set.add(3,"I");
	set.add(4,"Am");
	set.add(5,"Sheep");

	for (int i = 0; i < set.size(); i++) {
		assert(set.dense[i].val == sheepCase[i]);
		assert(set.get(i) == sheepCase[i]);
	}

	std::array<std::string,6> gloopCase = {"Blip","Bloop","Gloop","I","Am","Gloop"};
	std::array<std::string,6> gloopScrambleCase = {"Gloop","I","Bloop","Gloop","Am","Blip"};
	SparseSet<std::string> set2(10);
	set2.add(5,"Blip");
	set2.add(2,"Bloop");
	set2.add(3,"Gloop");
	set2.add(1,"I");
	set2.add(4,"Am");
	set2.add(0,"Gloop");

	for (int i = 0; i < set.size(); i++) {
		assert(set2.dense[i].val == gloopCase[i]);
		assert(set2.get(i) == gloopScrambleCase[i]);
	}

	struct Cow {
		std::string name;
		uint32_t weight = 0;
	};

	SparseSet<Cow> set3(3);
	//test adding structs
	assert( set3.add(0,Cow("Betsy",100)) );
	assert( set3.add(1,Cow("Shirley",150)) );
	assert( set3.add(2,Cow("Crowley",200)) );

	//test set is full
	assert( set3.add(3,Cow("Ass",250)) == false);
	assert( set3.add(4,Cow("Bongo",300)) == false);

	//test retrieval of structs
	assert( set3.get(0).name == "Betsy" && set3.get(0).weight == 100);
	assert( set3.get(1).name == "Shirley" && set3.get(1).weight == 150);
	assert( set3.get(2).name == "Crowley" && set3.get(2).weight == 200);
}

void Test::testSparseSetDelete() {
	SparseSet<std::string> set(10);
	set.add(0, "Boop");
	set.add(1,"Beep");
	set.add(2, "Sheep");
	set.add(3,"I");
	set.add(4,"Am");
	set.add(5,"Shoop");

	assert( set.del(1) );
	assert( set.del(3) );

	assert(set.get(3) == "");

	assert( set.del(5) );

	std::array<std::string, 3> correctDenseVals = {"Boop", "Am", "Sheep"};
	for (int i = 0; i < 3; i++) {
		assert(set.dense[i].val == correctDenseVals[i]);
	}

	assert( set.del(2) );
	assert( set.del(0) );

	// assert(set.del(0) == false);

	assert(set.get(4) == "Am");
	set.del(4);

	assert(set.is_empty());
}

void Test::testSparseSetAssign() {
	SparseSet<std::string> set(10);

	set.add(4, "Boink");
	set.add(2, "Bonk");
	set.add(1, "Bork");
	set.assign(2, "Boop");

	set.add(2, "Boof");
	set.add(7,"Worf");
	set.add(3, "Woof");
	set.assign(7, "Floof");

	std::array<std::string, 5> denseVals = {"Boink","Boop","Bork","Floof","Woof"};
	for (int i = 0; i < set.size(); i++) {
		assert(set.dense[i].val == denseVals[i]);
	}
}

void Test::testSparseSetClear() {
	SparseSet<int> set(10);
	for (int i = 0; i < set.capacity(); i++) {
		set.add(i, i);
	}

	set.clear();

	assert(set.is_empty());

	for (int i = 0; i < set.capacity(); i++) {
		assert(set.contains(i) == false);
	}
}

void Test::testSparseSetStructHierarchy() {

	struct Child {
		std::string name;
		float x;
		float y;
	};

	struct Parent {
		std::string name;
		Child child;
	};

	SparseSet<Parent> set(100);

	Parent parent{"Jimbo", Child("Fred", 10,20)};
	set.add(0,parent);

	Parent resParent = set.get(0);
	assert(resParent.name == "Jimbo");
	assert(resParent.child.name == "Fred");
	assert(resParent.child.x == 10);
	assert(resParent.child.y == 20);

}

void Test::testSparseSetPerformance() {
	Stopwatch stopwatch;

	int n = 100000;
	std::cout <<"N: " << n << "\n";
	SparseSet<int> set(n);

	stopwatch.start();
	for (int i = 0; i < set.capacity(); i++) {
		set.add(i,i);
	}
	std::cout << "Set add: " << stopwatch.click() << "\n";

	stopwatch.start();
	for (int i = 0; i < set.capacity(); i++) {
		int a = set.dense[i].val;
	}
	std::cout << "Set get: " << stopwatch.click() << "\n";

	stopwatch.start();
	for (int i = 0; i < set.capacity(); i++) {
		set.del(i);
	}
	std::cout << "Set del: " << stopwatch.click() << "\n";

	//compare to vector
	std::vector<int> vec;
	vec.reserve(n);

	stopwatch.start();
	for (int i = 0; i < n; i++) {
		vec.push_back(i);
	}

	std::cout << "Vec add: " << stopwatch.click() << "\n";

	stopwatch.start();
	for (int i = 0; i < n; i++) {
		int a = vec[i];
	}
	std::cout << "Vec get: " << stopwatch.click() << "\n";

	stopwatch.start();
	for (int i = 0; i < n; i++) {
		vec.pop_back();
	}
	std::cout << "Vec del: " << stopwatch.click() << "\n";
}

void Test::testECS() {
	testEntityManager();
	testEntityManagerGetEntities();
	testComponentManager();
	testSignatureConversion();
	testEntityComponent();
	testComponentManagerOperate();

	EntityManager entityManager(5000);
	ComponentManager componentManager(5000,32);

	Entity player = entityManager.allocEntity();
	Entity enemy1 = entityManager.allocEntity();
	Entity villager = entityManager.allocEntity();

	struct Transform {
		Vector3 position;
	};

	Transform playerTransform = {
		Vector3(0,0,0)
	};

	Transform enemyTransform = {
		Vector3(0,0,0)
	};

	Transform villagerTransform = {
		Vector3(1,1,1)
	};

	struct Health {
		float value;
	};

	componentManager.registerComponentType<Transform>();
	componentManager.registerComponentType<Health>();

	auto transformComp = componentManager.getComponents<Transform>();

	transformComp->add(player, playerTransform);
	transformComp->add(enemy1, enemyTransform);
	transformComp->add(villager, villagerTransform);

	auto healthComponent = componentManager.getComponents<Health>();

	healthComponent->add(player, {100});
	healthComponent->add(enemy1, {10});
	healthComponent->add(villager, {1000});

	componentManager.operate<Transform>([&componentManager](Entity entity, Transform &transform) {

		componentManager.operate<Transform>([entity, transform, &componentManager](Entity entity2, Transform&transform2) {
				if (entity != entity2 && transform.position == transform2.position) {
					auto health = componentManager.getComponents<Health>();

					health->assign(entity2, {0});
				}
		});
	});

	componentManager.operate<Transform>([healthComponent](Entity entity, Transform &transform) {
		std::cout << entity << " " << transform.position << " " << healthComponent->get(entity).value << "\n";
	});
}

void Test::testEntityManager() {
	EntityManager entityManager(500);

	for (int i = 0; i < 10; i++) {
		Entity entity = entityManager.allocEntity();

		Signature caseSig = 21;
		entityManager.setEntitySignature(entity, caseSig);

		auto sig = entityManager.getEntitySignature(entity);

		assert(sig == caseSig);
	}
}

void Test::testEntityManagerGetEntities() {
	int n = 1000000;
	EntityManager entityManager(n);
	for (int i = 0; i < n; i++) {
		Entity entity = entityManager.allocEntity();
		Signature sig = i;
		// sig = sig << (i%32);
		entityManager.setEntitySignature(entity, sig);
	}

	uint32_t testVal = 13;

	Stopwatch stopwatch;
	stopwatch.start();
	std::vector<Entity> entities = entityManager.getEntities(Signature(testVal));
	std::cout << stopwatch.click() << "\n";

	// for (Entity entity : entities) {
	// 	assert((entityManager.getEntitySignature(entity) & Signature(testVal)) == Signature(testVal));
	// }
}

void Test::testComponentManager() {
	ComponentManager componentManager(500, 32);
	struct Gun {
		uint32_t ammo;
	};

	struct Bullet {
		uint32_t damage;
	};

	componentManager.registerComponentType<Gun>();
	assert(componentManager.getComponentType<Gun>().value() == 0);

	auto gunSet = componentManager.getComponents<Gun>();

	gunSet->add(0, Gun(10));
	assert(gunSet->get(0).ammo == 10);

	componentManager.registerComponentType<Bullet>();
	assert(componentManager.getComponentType<Bullet>().value() == 1);

	//test adding and retreival
	auto bulletSet = componentManager.getComponents<Bullet>();
	bulletSet->add(0, Bullet(100));
	bulletSet->add(1, Bullet(200));
	bulletSet->add(2, Bullet(300));

	assert(bulletSet->get(0).damage == 100);
	assert(bulletSet->get(1).damage == 200);
	assert(bulletSet->get(2).damage == 300);

	//Test retrieval with second get
	SparseSet<Bullet>* bulletSet2 = componentManager.getComponents<Bullet>();
	assert(bulletSet2->get(0).damage == 100);
	assert(bulletSet2->get(1).damage == 200);
	assert(bulletSet2->get(2).damage == 300);

	componentManager.unregisterComponents<Gun>();
	componentManager.unregisterComponents<Bullet>();
}

void Test::testEntityComponent() {
	uint32_t entityCount = 500;
	uint32_t componentCount = 32;

	EntityManager entityManager(entityCount);
	ComponentManager componentManager(entityCount, componentCount);

	struct Health {
		float value = 10;
	};

	struct Position {
		Vector2 value = {15};
	};

	componentManager.registerComponentType<Health>();
	componentManager.registerComponentType<Position>();

	Entity player = entityManager.allocEntity();
	Entity enemy = entityManager.allocEntity();

	auto healthComponents = componentManager.getComponents<Health>();
	auto positionComponents = componentManager.getComponents<Position>();

	healthComponents->add(player, Health(101));
	healthComponents->add(enemy, Health(17));

	positionComponents->add(player, {Vector2(12,17)});
	positionComponents->add(enemy, {Vector2(21,72)});


	assert(healthComponents->get(player).value == 101);
	assert(positionComponents->get(player).value == Vector2(12,17));

	assert(healthComponents->get(enemy).value == 17);
	assert(positionComponents->get(enemy).value == Vector2(21,72));

	componentManager.unregisterComponents<Health>();
	componentManager.unregisterComponents<Position>();
	entityManager.freeEntity(player);
	entityManager.freeEntity(enemy);
}

void Test::testComponentManagerOperate() {
	EntityManager entityManager(30);
	ComponentManager componentManager(30,32);

	struct Transform {
		Vector2 position;
		float rotation = 0;
	};

	struct Health {
		uint32_t health = 0;
	};

	componentManager.registerComponentType<Transform>();
	componentManager.registerComponentType<Health>();
	auto transformComponents = componentManager.getComponents<Transform>();
	auto healthComponents = componentManager.getComponents<Health>();

	for (uint32_t i = 0; i < 30; ++i) {
		Transform transform{Vector2(0,i), static_cast<float>(i)};
		Health health{i};

		Entity entity = entityManager.allocEntity();
		transformComponents->add(entity, transform);
		healthComponents->add(entity, health);
	}

	componentManager.operate<Transform>([healthComponents](Entity entity, Transform &transform) {
		transform.position = transform.position + Vector2(0,10);

		if (healthComponents->get(entity).health > 15) {
			transform.position = Vector2(-10,-10);
		}
	});

	for  (uint32_t i = 0; i < transformComponents->size(); i++) {
		Vector2 pos = Vector2(0,i) + Vector2(0,10);
		if (i > 15) pos = Vector2(-10,-10);

		assert(transformComponents->dense[i].val.position == pos);
	}
}

void Test::testSignatureConversion() {
	std::vector<ComponentType> types = {0,3,5,7,9,20,31};

	Signature signature = ComponentManager::componentTypesToSignature(types);
	std::vector<ComponentType> typesRes = ComponentManager::signatureToComponentTypes(signature);

	assert(types == typesRes);
}

void Test::testLambdaFunc1(int a, std::string b) {
	assert(a == -20 && b == "Lambda1");
}

void Test::testLambdaFunc2(int a, std::string b) {
	a = 500;
	b = "Bonkbonk";
}

void Test::testLambdaFunc3(int &a, std::string &b) {
	a = 100;
	b = "Bark";
}

void Test::testMessaging() {
	testLambda();
	testEvent();
	testOnceEvent();
	// testLambdaPerformance();
}

void Test::testLambda() {
	int a = 10;
	std::string b = "Woof";

	//Capture and change A and B
	Lambda lambda([&a,&b]() {
		a = 200;
		b = "Goof";
	});

	lambda.call();
	assert(a == 200 && b == "Goof");

	Lambda lambda2(testLambdaFunc3);
	lambda2.call(a, b);

	assert(a == 100 && b == "Bark");

	struct Container {
		Lambda<std::function<void(int)>> lambda;
	};

	auto lamb = Lambda<std::function<void(int)>>([](int a){assert(a == -500);});
	auto lamb2 = Lambda<std::function<void(int)>>([](int a){assert(a == 300);});
	Container container{lamb};
	container.lambda.call(-500);

	// container.lambda = lamb2;
	// container.lambda = lamb;

}

void Test::testEvent() {
	Event<void(int)> event;

	int g = -5;
	int callCount = 0;

	event.add({[g, &callCount](int a) {
		assert(a * g == -75);
		callCount++;
	}});

	event.add({[&callCount](int a) {
		assert(a * 10 == 150);
		callCount++;
	}});

	event.add({[&callCount](int a) {
		assert(a * 100 == 1500);
		callCount++;
	}});

	event.call(15);
	assert(callCount==3);

	event.remove();
	callCount = 0;

	event.call(15);
	assert(callCount == 2);

	event.remove();
	event.remove();
	callCount = 0;

	event.call(100);
	assert(callCount == 0);

	int change = 10;
	event.add({[&callCount, &change](int mod) {
		change *= mod;
		callCount ++;
	}});

	event.call(5);
	event.call(10);
	event.call((20));

	assert(callCount == 3 && change == 10000);

	Event<void(int, std::string)> eventMixed;

	eventMixed.add({[](int x, std::string y) {
		assert(x == -20 && y == "Lambda1");
	}});
	eventMixed.add({testLambdaFunc1});

	eventMixed.call(-20, "Lambda1");

	Event<void(int &a, std::string &b)> refEvent;
	refEvent.add({testLambdaFunc3});

	int a = 0;
	std::string b = "";

	refEvent.call(a, b);

	assert(a == 100 && b == "Bark");

	//Test copying and containing of an event
	struct EventContainer {
		Event<void(int)> event;
	} container;

	a = 10;
	container.event.add([&a](int b){ a *= b; });

	container.event.call(15);

	assert(a == 150);

	EventContainer container2;
	container2.event = container.event;
	container2.event.call(100);

	assert(a == 15000);
}

void Test::testOnceEvent() {
	Event<void()> event;
	int a = 0;
	event.addOnce({[&a](){ a = 100;}});

	event.call();
	assert(a == 100);
	a = 0;

	event.call();
	assert(a == 0);
}

void testLambdaNormalFunc(int&a) {
	a*=10;
}

void Test::testLambdaPerformance() {
	int a = 0;
	auto anonymous = [&a]() {
		a *= 10;
	};

	Lambda fastLambda(testLambdaNormalFunc);

	Lambda lambda = [&a]() {
		a *= 10;
	};

	Stopwatch stopwatch;
	stopwatch.start();
	int n = 100000;

	for (int i = 0; i < n; i++) {
		testLambdaNormalFunc(a);
	}
	std::cout << "Normal: " << stopwatch.click() << "\n";
	a = 0;
	stopwatch.start();

	for (int i = 0; i < n; i++) {
		anonymous();
	}

	std::cout << "Anonymous: " << stopwatch.click() << "\n";
	a = 0;
	stopwatch.start();

	for (int i = 0; i < n; i++) {
		fastLambda.call(a);
	}

	std::cout << "Pointer Lambda: " << stopwatch.click() << "\n";
	a = 0;
	stopwatch.start();

	for  (int i = 0; i < n; i++) {
		lambda.call();
	}

	std::cout << "Lambda: " << stopwatch.click() << "\n";
	a = 0;
}


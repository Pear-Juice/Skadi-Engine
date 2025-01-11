#include "Tests.hpp"

#include <assert.h>
#include <Source/Resources/Vector.hpp>

#include "Source/ECS/ECS.hpp"

#include "Source/ECS/Messaging/Event.hpp"

void Test::test(int a, std::string b) {
	std::cout << "Call: " << a << " " << b << "\n";
}

void Test::testAll() {
	std::cout << "---Test All---\n";

	testECS();

	Lambda lambda(test);
	lambda.call(15, "Booop");

	std::cout << "---Success---\n";
}

void Test::testSparseSet() {
	SparseSet<std::string> set(100);

	// set.add(50, "A");
	// set.add(30, "B");
	// set.add(20, "C");
	// set.add(10, "D");
	//
	// set.del(20);
	//
	// set.print();


	testSparseSetAddRetrieve();
	testSparseSetDelete();
	testSparseSetAssign();
	testSparseSetClear();
	testSparseSetStructHierarchy();
	// testSparseSetPerformance();
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

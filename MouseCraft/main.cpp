#include <Windows.h>
#include <iostream>
#include "Core/OmegaEngine.h"
#include "Core/Entity.h"
#include "Core/Test/TestComponent.h"
#include "Core/Test/TestDerivedComponent.h"
#include "Core/ComponentManager.h"
#include "MainScene.h"
#include "Core/EntityManager.h"
#include "Core/Example/ExampleComponent.h"
#include "Core/Example/ExampleSystem.h"
#include "Physics/PhysicsManager.h"
#include "Physics/PhysObjectType.h"
#include "Loading/ModelLoader.h"
#include "Graphics/Model.h"
#include "Graphics/RenderSystem.h"
#include "Graphics/Renderable.h"
#include "Input/InputSystem.h"
#include "Mice.h"
#include "Graphics/ModelGen.h"
#include "Sound/SoundManager.h"
#include "Loading/ImageLoader.h"
#include "TestSubObs.h"
#include "DebugColliderComponent.h"
#include "DebugColliderSystem.h"
#include "PickupSpawner.h"
#include "Cat.h"
#include "PlayerComponent.h"
#include "HealthComponent.h"

#define GLEW_STATIC

SoundManager* noise;

extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

void Test_Rendering()
{
	//Model* m = ModelLoader::loadModel("res/models/test/CubeModel.obj");
	Model* m = ModelGen::makeCube(1, 1, 1);
	
	Model* floorModel = ModelGen::makeQuad(ModelGen::Axis::Y, 100, 100);
	
	Image* i = ImageLoader::loadImage("res/models/test/test.jpg");
	floorModel->setTexture(i);

	Image* blank = ImageLoader::loadImage("res/models/test/blank.bmp");
	m->setTexture(blank);

	OmegaEngine::Instance().initialize();
	Scene* s = new MainScene();
	OmegaEngine::Instance().ChangeScene(s);	// use fast transition

	// rendering 

	Renderable* rc = ComponentManager<Renderable>::Instance().Create<Renderable>();
	Renderable* rc2 = ComponentManager<Renderable>::Instance().Create<Renderable>();
	Renderable* floorRC = ComponentManager<Renderable>::Instance().Create<Renderable>();
	
	Camera* cam = ComponentManager<Camera>::Instance().Create<Camera>();

	rc->setColor(Color(0.5, 1.0, 0.25));
	rc->setModel(*m);

	rc2->setColor(Color(1.0, 0.25, 0.5));
	rc2->setModel(*m);

	floorRC->setColor(Color(0.6, 0.0, 0.75));
	floorRC->setModel(*floorModel);

	cam->setFOV(90.0f);
	cam->setCloseClip(0.01f);
	cam->setFarClip(100.0f);

	Entity* e1 = EntityManager::Instance().Create();
	Entity* e2 = EntityManager::Instance().Create();
	Entity* e3 = EntityManager::Instance().Create();
	Entity* floorEntity = EntityManager::Instance().Create();

	e1->transform.setLocalPosition(glm::vec3(-2.0, 0, -2.5));
	e2->transform.setLocalPosition(glm::vec3(3.5, 0, -3.0));
	e3->transform.setLocalPosition(glm::vec3(0, 10, 5));
	e3->transform.setLocalRotation(glm::vec3(-1.0f, 0, 0));

	floorEntity->transform.setLocalPosition(glm::vec3(0, 0, 0));

	e1->AddComponent(rc);
	e2->AddComponent(rc2);
	e3->AddComponent(cam);
	floorEntity->AddComponent(floorRC);

	RenderSystem* rs = new RenderSystem();
	rs->setWindow(OmegaEngine::Instance().getWindow());

	// player 1 (mice)
	auto c_p1_mice = ComponentManager<UpdatableComponent>::Instance().Create<Mice>();
	c_p1_mice->player = 0;
	c_p1_mice->speed = 50.0f;
	e1->AddComponent(c_p1_mice);

	auto c_p1_movement = ComponentManager<UpdatableComponent>::Instance().Create<PlayerComponent>();
	c_p1_movement->SetID(0);
	e1->AddComponent(c_p1_movement);

	auto c_p1_collider = ComponentManager<DebugColliderComponent>::Instance()
		.Create<DebugColliderComponent>();
	e1->AddComponent(c_p1_collider);

	auto c_p1_health = ComponentManager<HealthComponent>::Instance().Create<HealthComponent>();
	e1->AddComponent(c_p1_collider);

	// player 2 (cat)
	auto c_p2_Cat = ComponentManager<UpdatableComponent>::Instance().Create<Cat>();
	c_p2_Cat->setPlayer(1);
	e2->AddComponent(c_p2_Cat);

	auto playerc = ComponentManager<UpdatableComponent>::Instance().Create<PlayerComponent>();
	playerc->SetID(1);
	e2->AddComponent(playerc);
	
	auto healthc = ComponentManager<HealthComponent>::Instance().Create<HealthComponent>();
	e2->AddComponent(healthc);

	auto c_p2_collider = ComponentManager<DebugColliderComponent>::Instance().Create<DebugColliderComponent>();
	e2->AddComponent(c_p2_collider);

	// spawner 
	auto e_spawner = EntityManager::Instance().Create();
	auto c_spawner = ComponentManager<UpdatableComponent>::Instance().Create<PickupSpawner>();
	e_spawner->AddComponent(c_spawner);
	
	DebugColliderSystem* dcs = new DebugColliderSystem();
	InputSystem* is = new InputSystem();

	PhysicsManager* physicsSystem = new PhysicsManager();

	// component_player1_physics 
	auto c_p1_physics = physicsSystem->createObject(10, 10, 1, 1, 0, PhysObjectType::MOUSE_DOWN);

	// add to mouse entity
	e1->AddComponent(c_p1_physics);

	// component_player2_physics 

	auto c_p2_physics = physicsSystem->createObject(0, 0, 1, 1, 0, PhysObjectType::CAT_DOWN);

	// add to cat entity
	e2->AddComponent(c_p2_physics);

	// adjustments made in PlayerComponent 

	OmegaEngine::Instance().AddSystem(physicsSystem);

	// add the entities 

	OmegaEngine::Instance().AddEntity(e1);
	OmegaEngine::Instance().AddEntity(e2);
	OmegaEngine::Instance().AddEntity(e3);
	OmegaEngine::Instance().AddEntity(e_spawner);
	OmegaEngine::Instance().AddEntity(floorEntity);

	OmegaEngine::Instance().AddSystem(rs);
	OmegaEngine::Instance().AddSystem(is);
	// OmegaEngine::Instance().AddSystem(dcs);
	OmegaEngine::Instance().Loop();


}

void Test_ECS()
{
	// NOTE: Use SDL_assert b/c SDL2 is manhandling everything.

	/*
	Test:
	Entity should have instant action when not in active scene
	- add/delete/move/enable/disable

	Entity actions should propogate
	- add (must call initialize())
	- delete
	- enable/disable

	Entity should have deferred action when interacting with active scene
	- add/delete/move/enable/disable

	Special case
	- non-initialized added onto initialized

	Should be able to retrieve specific type of components

	Component initialize test
	- should be able to get reference
	*/

	EntityManager::Instance();

	// testing componentmanager types
	ComponentManager<TestComponent>& cm = ComponentManager<TestComponent>::Instance();
	ComponentManager<UpdatableComponent>& ucm = ComponentManager<UpdatableComponent>::Instance();
	ComponentManager<TestDerivedComponent>& tdm = ComponentManager<TestDerivedComponent>::Instance();
	// ComponentManager<ExampleComponent>::Instance();	// lazy initialize
	//ComponentManager<Component>& ccm = ComponentManager<Component>::Instance();
	

	// TESTS: transparent instant execution
	Entity* parent1 = new Entity();
	Entity* child1 = new Entity();
	Entity* parent2 = new Entity();
	Entity* child2 = new Entity();
	TestComponent* tc = new TestComponent(nullptr);
	TestDerivedComponent* tdc = new TestDerivedComponent();

	auto sizeShouldBe4 = EntityManager::Instance().GetEntities();
	SDL_assert(sizeShouldBe4.size() == 4 && "EntityManager failed (1)");

	// test 
	parent1->AddChild(child1);

	SDL_assert(parent1->GetChildren().size() == 1, "Instant add failed.");

	child1->SetParent(parent2);

	SDL_assert(parent1->GetChildren().size() == 0, "Instant move failed (was not removed from existing parent).");
	SDL_assert(parent2->GetChildren().size() == 1, "Instant move failed (was not moved to new parent).");

	parent2->SetEnabled(false);

	SDL_assert(parent2->GetEnabled() == false, "Instant disable failed");

	// component test 
	parent1->AddComponent(tc);
	parent2->AddComponent(tdc);

	SDL_assert(parent1->GetComponents().size() == 1 && "Component add failed (1)");
	SDL_assert(parent2->GetComponents().size() == 1 && "Component add failed (2)");

	parent1->RemoveComponent(tc);	
	parent2->RemoveComponent<TestComponent>();	// should still work despite derived

	SDL_assert(parent1->GetComponents().size() == 0 && "Component remove failed (1)");
	SDL_assert(parent2->GetComponents().size() == 0 && "Component remove failed (2)");

	// cleanup 
	delete(parent1);
	delete(child1);
	delete(parent2);
	delete(child2);

	auto sizeShouldBe0 = EntityManager::Instance().GetEntities();
	SDL_assert(sizeShouldBe0.size() == 0 && "EntityManager failed (1)");

	// TESTS: deferred execution 
	
	// init 
	OmegaEngine::Instance().initialize();

	Scene* s = new MainScene();

	OmegaEngine::Instance().ChangeScene(s);	// use fast transition

	parent1 = new Entity();
	parent2 = new Entity();
	child1 = new Entity();
	child2 = new Entity();

	parent1->AddChild(child1);
	parent2->AddChild(child2);

	auto testComponent = new TestComponent(parent2);
	auto derivedComponent = new TestDerivedComponent();

	parent1->AddComponent(testComponent);
	parent2->AddComponent(derivedComponent);

	// test
	SDL_assert(s->root.GetChildren().size() == 0, "Deferred execution initial state failed.");

	auto shouldBeNull = parent1->GetComponent<TestDerivedComponent>();
	auto shouldBeOkay = parent2->GetComponent<TestComponent>();

	SDL_assert(shouldBeNull == nullptr, "Component retrieval failed (1)");
	SDL_assert(shouldBeOkay != nullptr, "Component retrieval failed (2)");

	// deferred add 
	s->root.AddChild(parent1);
	s->root.AddChild(parent2);

	SDL_assert(s->root.GetChildren().size() == 0, "Deferred execution failed.");

	// component manager test
	// this is how you should be creating components!
	auto* t = ComponentManager<TestComponent>::Instance().Create<TestComponent>(new Entity());
	auto* t1 = ComponentManager<TestComponent>::Instance().Create<TestDerivedComponent>(new Entity());
	// not like this lol
	TestComponent test(new Entity);
	delete t;

	//auto ec1 = ComponentManager<ExampleComponent>::Instance().Create<ExampleComponent>();
	//auto ec2 = ComponentManager<ExampleComponent>::Instance().Create<ExampleComponent>();
	//auto ec3 = ComponentManager<ExampleComponent>::Instance().Create<ExampleComponent>();
	//auto ec4 = ComponentManager<ExampleComponent>::Instance().Create<ExampleComponent>();

	//parent1->AddComponent(ec1);
	//parent2->AddComponent(ec2);
	//child1->AddComponent(ec3);	// this is weird, don't move child too lol
	//child2->AddComponent(ec4);

	ExampleSystem exampleSystem;

	OmegaEngine::Instance().AddSystem(&exampleSystem);

	OmegaEngine::Instance().Loop();
}

void Test_ObserverPattern()
{
	// Test 1
	FoobarSubject subject;
	FoobarObserver observer;

	subject.Attach(observer);
	subject.Notify(4.0f, 2);

	// Test 2
	Subject<int> OnTemperatureChanged;
	FreezingObserver obs1;
	BoilingObserver obs2;

	OnTemperatureChanged.Attach(obs1);
	OnTemperatureChanged.Attach(obs2);

	OnTemperatureChanged.Notify(-20);
	OnTemperatureChanged.Notify(-10);
	OnTemperatureChanged.Notify(0);
	OnTemperatureChanged.Notify(50);
	OnTemperatureChanged.Notify(100);
	OnTemperatureChanged.Notify(200);
}

int main(int argc, char* argv[]) 
{
    //adding sound system
    noise = new SoundManager();
    //start initial music track, standard form for music selection
    //create Track Params for event
    TrackParams * initial = new TrackParams();
    //select song
    initial->track = MainBGM;
    //specify song location. Usually fine to leave with default values of 0
    initial->x = 0;
    initial->y = 0;
    initial->z = 0;
    //create a type Param from the track params and pass it into the event notifier
    TypeParam<TrackParams*> param(initial);
    EventManager::Notify(PLAY_SONG, &param);

	Test_Rendering();
    /*
	// Test_ECS();

	OmegaEngine::Instance().initialize();

	OmegaEngine::Instance().AddSystem(new InputSystem());

	// fast load 
	Scene* s = new MainScene();
	OmegaEngine::Instance().ChangeScene(s);	

	// create some entities 
	auto mouse = EntityManager::Instance().Create();
	auto c_control = ComponentManager<UpdatableComponent>::Instance()
		.Create<MouseMovement>();
	c_control->player = 0;
	mouse->AddComponent(c_control);

	auto playerTwo = EntityManager::Instance().Create();
	auto c_control2 = ComponentManager<UpdatableComponent>::Instance()
		.Create<MouseMovement>();
	c_control2->player = 1;
	playerTwo->AddComponent(c_control2);

	OmegaEngine::Instance().AddEntity(mouse);
	OmegaEngine::Instance().AddEntity(playerTwo);

	OmegaEngine::Instance().Loop();
	*/
}
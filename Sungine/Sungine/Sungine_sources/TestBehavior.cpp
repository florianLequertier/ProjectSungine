

#include "TestBehavior.h"
//forwards : 
#include "BehaviorFactory.h"
#include "Scene.h"
#include "SceneAccessor.h"
#include "Coroutine.h"
#include "Animator.h"
#include "InputHandler.h"
#include "CharacterController.h"
#include "Entity.h"

//constexpr int tatu() {
//	static int tutu;
//	BehaviorFactory::get().add(std::type_index(typeid(TestBehavior)), new TestBehavior());
//	return 1;
//} 


REGISTER_BEHAVIOUR(TestBehavior);
COMPONENT_IMPLEMENTATION_CPP(TestBehavior)

TestBehavior::TestBehavior()
{
	
}

TestBehavior::~TestBehavior()
{
}

void TestBehavior::start(Scene& scene)
{
	std::cout << "testBehaviour start" << std::endl;
	entity()->startCoroutine < void > (std::function<void()>([]() { std::cout << "coroutine called !" << std::endl; }), 1.f);
	Animator* animator = getComponent<Animator>(ComponentType::ANIMATOR);
	if (animator != nullptr) {
		animator->play();
	}
}

void TestBehavior::update(Scene& scene)
{
	//std::cout << "testBehaviour update" << std::endl;
	auto* characterController = getComponent<CharacterController>(ComponentType::CHARACTER_CONTROLLER);
	if (characterController != nullptr)
	{
		if (InputHandler::getKey(GLFW_KEY_Z, InputHandler::FOCUSING_GAME))
			characterController->move(glm::vec3(0.f, 0.f, 4.f));
		if (InputHandler::getKey(GLFW_KEY_Q, InputHandler::FOCUSING_GAME))
			characterController->move(glm::vec3(-4.f, 0.f, 0.f));
		if (InputHandler::getKey(GLFW_KEY_D, InputHandler::FOCUSING_GAME))
			characterController->move(glm::vec3(4.f, 0.f, 0.f));
		if (InputHandler::getKey(GLFW_KEY_S, InputHandler::FOCUSING_GAME))
			characterController->move(glm::vec3(0.f, 0.f, -4.f));
	}
}

void TestBehavior::onCollisionEnter(Scene& scene, const CollisionInfo& collisionInfo)
{
	std::cout << "collision begin at point : ("<<collisionInfo.point.x <<", "<< collisionInfo.point.y <<", "<< collisionInfo.point.z <<")"<< std::endl;
}

void TestBehavior::onCollisionStay(Scene & scene, const CollisionInfo & collisionInfo)
{
	std::cout << "collision stay at point : (" << collisionInfo.point.x << ", " << collisionInfo.point.y << ", " << collisionInfo.point.z << ")" << std::endl;
}

void TestBehavior::onCollisionEnd(Scene & scene, const CollisionInfo & collisionInfo)
{
	std::cout << "collision end at point : (" << collisionInfo.point.x << ", " << collisionInfo.point.y << ", " << collisionInfo.point.z << ")" << std::endl;
}

void TestBehavior::save(Json::Value & entityRoot) const
{
	Behavior::save(entityRoot);
}

void TestBehavior::load(const Json::Value & entityRoot)
{
	Behavior::load(entityRoot);
}

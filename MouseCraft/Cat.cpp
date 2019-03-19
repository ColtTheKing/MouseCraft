#include "Cat.h"
#include "Event\EventManager.h"
#include "Input\InputSystem.h"
#include <iostream>

#define ATTACK_TIME 5
#define JUMP_TIME 5
#define POUNCE_TIME 5

Cat::Cat() :
	HandleOnCollide(this, &Cat::OnCollision),
	HandleOnHit(this, &Cat::OnHit)
{
    EventManager::Subscribe(INPUT_BUTTON, this);
    playerID = 0;
    current_time = 0;
}

Cat::~Cat()
{
}

void Cat::Update(float dt) {
    if (isAttacking) {
        UpdateAttack(dt);
    }

    if (isJumping) {
        UpdateJump(dt);
    }

	PhysicsComponent* pComp = GetEntity()->GetComponent<PhysicsComponent>();

	//check to see if you are on a platform
	if (pComp != nullptr && !pComp->isJumping && pComp->isUp)
	{
		std::set<PhysObjectType::PhysObjectType> types = std::set<PhysObjectType::PhysObjectType>{
			PhysObjectType::PLATFORM
		};
			
		auto compPos = pComp->body->GetPosition();
		Vector2D* p1 = new Vector2D(compPos.x - (pComp->width / 2), compPos.y + (pComp->height / 2));
		Vector2D* p2 = new Vector2D(compPos.x + (pComp->width / 2), compPos.y - (pComp->height / 2));

		std::vector<PhysicsComponent*> found = pComp->areaCheck(types, p1, p2);

		//if you aren't on a platform then fall
		if(found.size() == 0)
			pComp->isFalling = true;
	}
	
	if (isPouncing) {
        updatePounce(dt);
    }
}

void Cat::OnInitialized()
{
	//Listens for collisions with the physics component
	PhysicsComponent* pComp = GetEntity()->GetComponent<PhysicsComponent>();

	if (pComp != nullptr)
	{
		HandleOnCollide.Observe(pComp->onCollide);
		HandleOnHit.Observe(pComp->onHit);
	}

    playerID = GetEntity()->GetComponent<PlayerComponent>()->GetID();
}

//check for button presses and then call functions
void Cat::Notify(EventName eventName, Param * param) {
    UpdatableComponent::Notify(eventName, param);
    if (eventName == INPUT_BUTTON) {
        auto data = static_cast<TypeParam<ButtonEvent>*>(param)->Param;
        if (data.player != playerID || data.isDown != true) {
            return;
        }

        switch (data.button)
        {
        case Button::PRIMARY:
        case Button::SECONDARY:
            Attack();
            break;
        case Button::AUX1:
            Jump();
            break;
        default:
            break;
        }
    }
}

void Cat::OnCollision(PhysicsComponent * pc)
{
	
}

void Cat::OnHit(PhysicsComponent* other)
{

}

//check if we are doing something, then attack
void Cat::Attack()
{
    //block if we are in an animation already
    if (isAttacking || isJumping || isPouncing) {
        return;
    }
    //actually launch attack

    //get our own physics component
    auto ourPhys = GetEntity()->GetComponent<PhysicsComponent>();

    //check which level we're on
    std::set<PhysObjectType::PhysObjectType> targets;
    if (ourPhys->isUp) {
        //generate check type
        targets = std::set<PhysObjectType::PhysObjectType>{
            //PhysObjectType::OBSTACLE_UP,
            PhysObjectType::MOUSE_UP
        };
    } else {
        //generate check type
        targets = std::set<PhysObjectType::PhysObjectType>{
           // PhysObjectType::OBSTACLE_DOWN,
            PhysObjectType::MOUSE_DOWN
        };
    }
    //determine our position for area check
    //get the section directly in front of our world position
    auto p1 = GetEntity()->transform;
    auto pos = p1.getWorldPosition();
    pos += p1.getWorldForward();
    //get the corners for our bounding box
    auto bl = pos + glm::vec3(-10, 0, -10);
    auto tr = pos + glm::vec3(10, 0, 10);

    //launch area check
    auto results = ourPhys->areaCheck(targets, new Vector2D(bl.x, bl.z), new Vector2D(tr.x, tr.z));

    //check if we hit something
    if (results.size() > 0) {
        for (size_t i = 0; i < targets.size(); i++) {
            results[i]->GetEntity()->GetComponent<HealthComponent>()->Damage(1);
        }
    }

    std::cout << "Cat has attacked." << std::endl;
    isAttacking = true;
}

// track the time since we launched the attack animation, and reset when finished
void Cat::UpdateAttack(float dt) {
    if (current_time < ATTACK_TIME) {
        //advance time
        current_time += dt;
        return;
    }
    isAttacking = false;
    current_time = 0;
}

//check if we are doing something, then check if we can jump and either jump or pounce
void Cat::Jump()
{
	PhysicsComponent* pComp = GetEntity()->GetComponent<PhysicsComponent>();

	if (pComp != nullptr)
	{
		//block if we are in an animation already
		if (isAttacking || pComp->isJumping || isPouncing)
			return;

		//position of cat
		Vector2D* curPos = new Vector2D(GetEntity()->transform.getLocalPosition().x, GetEntity()->transform.getLocalPosition().z);
		//vector in front of cat of length = JUMP_DIST
		Vector2D* jumpVec = new Vector2D(GetEntity()->transform.getLocalForward().x * JUMP_DIST, GetEntity()->transform.getLocalForward().z * JUMP_DIST);
		jumpVec = new Vector2D(*curPos + *jumpVec);

		std::set<PhysObjectType::PhysObjectType> types = std::set<PhysObjectType::PhysObjectType>{
			PhysObjectType::PLATFORM
		};

		Vector2D* hitPos = new Vector2D(0, 0);

		PhysicsComponent* jumpTarget =  pComp->rayCheck(types, curPos, jumpVec, *hitPos);

		//check if we are in a location we can jump in
		if (jumpTarget != nullptr) {
			//Jump code
			std::cout << "Cat has jumped." << std::endl;
			GetEntity()->GetComponent<PhysicsComponent>()->isJumping = true;
			isJumping = true;

			GetEntity()->GetComponent<SoundComponent>()->ChangeSound(SoundsList::Jump); //set sound to jump
			auto pos = GetEntity()->transform.getLocalPosition(); //get our current position
			GetEntity()->GetComponent<SoundComponent>()->PlaySound(pos.x, pos.y, pos.z); //play sound
			return;
		}
	}

    //pounce code
    std::cout << "Cat has pounced." << std::endl;
    //isPouncing = true;
}

// track the time since we launched the jump animation, and reset when finished
void Cat::UpdateJump(float dt)
{
    /*if (current_time < JUMP_TIME) {
        //advance time
        current_time += dt;
        return;
    }
    isJumping = false;
    current_time = 0;*/
}

// track the time since we launched the pounce animation, and reset when finished
void Cat::updatePounce(float dt)
{
    /*if (current_time < POUNCE_TIME) {
        //advance time
        current_time += dt;
        return;
    }
    isPouncing = false;
    current_time = 0;*/
}

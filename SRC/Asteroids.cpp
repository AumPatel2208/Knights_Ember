#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "AlienSpaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "BulletPowerUp.h"
#include "CircleBulletPowerUp.h"
#include "OnePowerUp.h"

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char* argv[])
	: GameSession(argc, argv) {
	mLevel = 0;
	mAsteroidCount = 0;
}

/** Destructor. */
Asteroids::~Asteroids(void) {
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start() {
	/* Made global line below*/
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	//shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);
	thisPtr = shared_ptr<Asteroids>(this);
	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	/*Add code for Start Menu*/


	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat diffuse_light[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation* explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile(
		"explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation* blue_explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile(
		"blueExplosion", 64, 1024, 64, 64, "explosionBlue_fs.png");
	Animation* asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile(
		"asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation* spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile(
		"spaceship", 128, 128, 128, 128, "spaceship_fs.png");

	Animation* alien_spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile(
		"alienSpaceship", 128, 128, 128, 128, "alien_spaceship_fs.png");

	CreateMenu();

	// Start the game
	GameSession::Start();

}

/** Stop the current game. */
void Asteroids::Stop() {
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y) {
	switch (key) {
	case ' ':
		if (isGameRunning) mSpaceship->Shoot();
		break;
	case '1':
		if (!isGameRunning) {
			// Create a spaceship and add it to the world
			mGameWorld->AddObject(CreateSpaceship());
			// Create an alienspaceship and add it to the world
			mGameWorld->AddObject(CreateAlienSpaceship());
			isAlienAlive=true;
			SetTimer(500, UPDATE_ALIEN_SHIP);
			SetTimer(2000, SHOOT_ALIEN_SHIP);
			// Create some asteroids and add them to the world
			CreateAsteroids(1);
			CreateBulletPowerUps(1);
			CreateOnePowerUps(1);
			CreateCircleBulletPowerUps(1);
			//Hiding menu
			aGameTitle->SetVisible(false);
			aStartGameOption->SetVisible(false);
			aExitGameOption->SetVisible(false);
			aInstructions->SetVisible(false);


			//Create the GUI
			CreateGUI();

			// Add a player (watcher) to the game world
			mGameWorld->AddListener(&mPlayer);

			// Add this class as a listener of the player
			mPlayer.AddListener(thisPtr);
			isGameRunning = true;
		}
		break;
	case '2':
		Stop();
	default:
		break;
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {
}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y) {
	switch (key) {
		// If up arrow key is pressed start applying forward thrust
	case GLUT_KEY_UP: if (isGameRunning)mSpaceship->Thrust(10);
		break;
		// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT: if (isGameRunning) mSpaceship->Rotate(90);
		break;
		// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT: if (isGameRunning) mSpaceship->Rotate(-90);
		break;
		// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y) {
	switch (key) {
		// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: if (isGameRunning)mSpaceship->Thrust(0);
		break;
		// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: if (isGameRunning) mSpaceship->Rotate(0);
		break;
		// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: if (isGameRunning)mSpaceship->Rotate(0);
		break;
		// Default case - do nothing
	default: break;
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object) {
	if (object->GetType() == GameObjectType("Asteroid")) {
		shared_ptr<GameObject> explosion = CreateExplosion(EXPLOSION_ASTEROID);
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0) {
			SetTimer(500, START_NEXT_LEVEL);
		}
	}
	else if (object->GetType() == GameObjectType("AlienSpaceship")) {
		shared_ptr<GameObject> explosion = CreateExplosion(EXPLOSION_SPACESHIP);
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		explosion->SetScale(0.5f);
		mGameWorld->AddObject(explosion);
		isAlienAlive=false;
	}
	else if (object->GetType() == GameObjectType("BulletPowerUp")) {
		mSpaceship->toggleSuperShot();
		bulletPowerTime = 10;
		SetTimer(10000, RESET_BULLET_POWER_UP);
		SetTimer(10, INCREASE_POWER_UP_COUNTER);
	}
	else if (object->GetType() == GameObjectType("OnePowerUp")) {
		mPlayer.addLives(1);
		std::ostringstream msg_stream;
		msg_stream << "Lives: " << mPlayer.getLives();
		// Get the lives left message as a string
		std::string lives_msg = msg_stream.str();
		mLivesLabel->SetText(lives_msg);
	}
	else if (object->GetType() == GameObjectType("CircleBulletPowerUp")) {
		mSpaceship->toggleUltraShoot();
		bulletPowerTime = 10;
		SetTimer(10000, RESET_CIRCLE_BULLET_POWER_UP);
		SetTimer(10, INCREASE_POWER_UP_COUNTER);
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value) {
	if (value == CREATE_NEW_PLAYER) {
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL) {
		mLevel++;
		const int num_asteroids = 1 + 2 * mLevel;
		CreateOnePowerUps(1);
		CreateBulletPowerUps(1);
		if(mLevel%2==0) CreateCircleBulletPowerUps(1);
		CreateAsteroids(num_asteroids);
		if(!isAlienAlive){
		mAlienSpaceship->SetRandomPosition();
		mAlienSpaceship->SetVelocity(GLVector3f(0,0,0));
		mGameWorld->AddObject(mAlienSpaceship); isAlienAlive=true;}
	}

	if (value == SHOW_GAME_OVER) {
		mGameOverLabel->SetVisible(true);
	}

	if (value == RESET_BULLET_POWER_UP) {
		mSpaceship->toggleSuperShot();
	}

	if (value == INCREASE_POWER_UP_COUNTER) {
		bulletPowerTime -= 1;
		std::ostringstream msg_stream;
		msg_stream << "Bullet Time: " << bulletPowerTime + 1;
		std::string bullet_msg = msg_stream.str();
		aBulletPowerTime->SetText(bullet_msg);
		if (bulletPowerTime >= 0) {
			SetTimer(1000, INCREASE_POWER_UP_COUNTER);
		}
	}

	if (value == RESET_CIRCLE_BULLET_POWER_UP) {
		mSpaceship->toggleUltraShoot();
	}

	if (value == UPDATE_ALIEN_SHIP) {
		if (isAlienAlive){
		mAlienSpaceship->Thrust(5, mSpaceship->GetPosition());

		}
		SetTimer(100, UPDATE_ALIEN_SHIP);
	}
	if (value == SHOOT_ALIEN_SHIP) {
		if(isAlienAlive){
		mAlienSpaceship->Shoot(mSpaceship->GetPosition());
		}
		SetTimer(2500, SHOOT_ALIEN_SHIP);
	}

}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship() {
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

shared_ptr<GameObject> Asteroids::CreateAlienSpaceship() {
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mAlienSpaceship = make_shared<AlienSpaceship>();
	mAlienSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mAlienSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet_alien.shape");
	mAlienSpaceship->SetBulletShape(bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("alienSpaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mAlienSpaceship->SetSprite(spaceship_sprite);
	mAlienSpaceship->SetAngle(180);
	mAlienSpaceship->SetScale(0.1f);
	// Return the spaceship so it can be added to the world
	return mAlienSpaceship;
}


void Asteroids::CreateAsteroids(const uint num_asteroids) {
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++) {
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateBulletPowerUps(const uint num_powerUps) {
	for (uint i = 0; i < num_powerUps; i++) {
		shared_ptr<GameObject> powerUp = make_shared<BulletPowerUp>();

		Animation* anim_ptr = AnimationManager::GetInstance().CreateAnimationFromFile(
			"bulletPowerUp", 160, 160, 160, 160, "bulletPowerUp.png");
		shared_ptr<Sprite> spaceship_sprite =
			make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		powerUp->SetSprite(spaceship_sprite);
		powerUp->SetBoundingShape(make_shared<BoundingSphere>(powerUp->GetThisPtr(), 4.00f));
		powerUp->SetScale(0.05f);
		mGameWorld->AddObject(powerUp);
	}
}

void Asteroids::CreateOnePowerUps(const uint num_powerUps) {
	for (uint i = 0; i < num_powerUps; i++) {
		shared_ptr<GameObject> powerUp = make_shared<OnePowerUp>();
		Animation* anim_ptr = AnimationManager::GetInstance().CreateAnimationFromFile(
			"onePowerUp", 160, 160, 160, 160, "1-up-powerup (1).png");
		shared_ptr<Sprite> spaceship_sprite =
			make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		powerUp->SetSprite(spaceship_sprite);
		powerUp->SetBoundingShape(make_shared<BoundingSphere>(powerUp->GetThisPtr(), 4.00f));
		powerUp->SetScale(0.05f);
		mGameWorld->AddObject(powerUp);
	}
}

void Asteroids::CreateCircleBulletPowerUps(const uint num_powerUps) {
	for (uint i = 0; i < num_powerUps; i++) {
		shared_ptr<GameObject> powerUp = make_shared<CircleBulletPowerUp>();

		Animation* anim_ptr = AnimationManager::GetInstance().CreateAnimationFromFile(
			"circleBulletPowerUp", 160, 160, 160, 160, "circleShot.png");
		shared_ptr<Sprite> spaceship_sprite =
			make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		powerUp->SetSprite(spaceship_sprite);
		powerUp->SetBoundingShape(make_shared<BoundingSphere>(powerUp->GetThisPtr(), 4.00f));
		powerUp->SetScale(0.05f);
		mGameWorld->AddObject(powerUp);
	}
}

void Asteroids::CreateMenu() {
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));

	aGameTitle = make_shared<GUILabel>("Asteroids by Aum");
	aStartGameOption = make_shared<GUILabel>("1) Start Game");
	aExitGameOption = make_shared<GUILabel>("2) Exit Game");
	aInstructions = make_shared<GUILabel>("Arrow Keys to move, Space to shoot");
	aGameTitle->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	aStartGameOption->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	aExitGameOption->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	aInstructions->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// aInstructions->SetSize(GLVector2i(0.5));

	shared_ptr<GUIComponent> a_game_menu_title_component = static_pointer_cast<GUIComponent>(aGameTitle);
	shared_ptr<GUIComponent> a_game_menu_startGame_component = static_pointer_cast<GUIComponent>(aStartGameOption);
	shared_ptr<GUIComponent> a_game_menu_exitGame_component = static_pointer_cast<GUIComponent>(aExitGameOption);
	shared_ptr<GUIComponent> a_game_menu_instructions_component = static_pointer_cast<GUIComponent>(aInstructions);

	mGameDisplay->GetContainer()->AddComponent(a_game_menu_title_component, GLVector2f(0.3f, 0.7f));
	mGameDisplay->GetContainer()->AddComponent(a_game_menu_startGame_component, GLVector2f(0.3f, 0.5f));
	mGameDisplay->GetContainer()->AddComponent(a_game_menu_exitGame_component, GLVector2f(0.3f, 0.4f));
	mGameDisplay->GetContainer()->AddComponent(a_game_menu_instructions_component, GLVector2f(0.18f, 0.0f));

}

void Asteroids::CreateGUI() {

	aExitGameOption = make_shared<GUILabel>("2) Exit Game");
	aExitGameOption->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);

	shared_ptr<GUIComponent> a_game_menu_exitGame_component = static_pointer_cast<GUIComponent>(aExitGameOption);
	mGameDisplay->GetContainer()->AddComponent(a_game_menu_exitGame_component, GLVector2f(0.7f, 1.0f));

	aBulletPowerTime = make_shared<GUILabel>("Bullet Time: 0");
	aBulletPowerTime->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);

	shared_ptr<GUIComponent> a_bulletPowerTime_component = static_pointer_cast<GUIComponent>(aExitGameOption);
	mGameDisplay->GetContainer()->AddComponent(aBulletPowerTime, GLVector2f(0.65f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component = static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));

}

void Asteroids::OnScoreChanged(int score) {
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left) {
	shared_ptr<GameObject> explosion = CreateExplosion(EXPLOSION_SPACESHIP);
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
		explosion->SetScale(0.5f);

	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0) {
		SetTimer(1000, CREATE_NEW_PLAYER);
	}
	else {
		SetTimer(500, SHOW_GAME_OVER);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion(const int& type) {
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	if (type == EXPLOSION_SPACESHIP) {
		anim_ptr = AnimationManager::GetInstance().GetAnimationByName("blueExplosion");
	}
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}




//Describers all funcitons for Player movement, digging, jumping, etc.
#include "Player.h"

//Sprite hit groups
#define DIRT	101
#define EMPTY	102
#define	LADDER  103
#define STONE	151
#define COAL	152
#define COPPER  153
#define TIN		154
#define IRON	155
#define EMERALD	156
#define GOLD	157
#define DIAMOND 158

//keycodes
#define LEFT	65
#define RIGHT	68
#define UP		87
#define DOWN	83
#define SPACE	32
#define SHIFT	16

Player::Player()
{
}

Player::~Player()
{}

float Player::Tiles(float x)
{ return 32.0f * x; }

void Player::Init()
{
	//Player Variables
	gravity = 600.0f;
	effects_vol = 25;
	jump_speed = 280.0f;
	walk_speed = 200.0f;
	pickaxe_speed = 5.0f; //player animation speed for digigng
	dig_speed = 20.0f; //needs to be variable eventually dependent on depth and pickaxe power
	starting_pos = 1600.0f;
	friction = 0;
	xOffset = 11.0f;
	yOffset = 18.0f;
	x_dir = 0;
	y_dir = 0;
	debug = false; //if true print debug variables to screen in update
	building_ladder = false;
	done_digging = false;
	stone_falling = false;
	edge1 = 42.0f;
	edge2 = 3189.0f;

	//loading player sprite
	sprite = agk::CreateSprite( agk::LoadImage("MinerR0.png") );
	agk::SetSpritePositionByOffset(sprite, starting_pos, - agk::GetSpriteHeight(sprite));
	agk::SetSpriteDepth(sprite, 4);
	agk::SetSpriteShapeBox(sprite, -10.0f, -8.0f, 10.0f, 15.0f);//set up collision box
	agk::SetSpriteOffset(sprite, 0.5f * agk::GetSpriteWidth(sprite), 0.5f * agk::GetSpriteHeight(sprite));//set offset to cSHIFT of sprite
	agk::SetSpritePhysicsOn (sprite, 2);
	agk::SetPhysicsGravity(0, gravity);
	agk::SetSpritePhysicsFriction(sprite, friction);
	agk::AddSpriteAnimationFrame(sprite, agk::LoadImage("MinerR1.png"));
	agk::AddSpriteAnimationFrame(sprite, agk::LoadImage("MinerR0.png"));
	//agk::SetSpritePhysicsCanRotate(sprite, 0); //for making him not able to flip around (not as silly)
	
	//loading dirt animation...
	dig_anim = agk::CreateSprite( agk::LoadImage ("dirt_anim.png") );
	agk::SetSpriteAnimation(dig_anim, 32, 32, 9);
	agk::SetSpriteDepth(dig_anim, 20);
	agk::SetSpriteColorAlpha(dig_anim, 200);
	agk::SetSpriteVisible(dig_anim, 0);

	//load sound effects
	jump_sound = agk::LoadSound("jump1.wav");
	dig_sound = agk::LoadSound("dig4.wav");
	break_sound = agk::LoadSound("breaksound2.wav");
	pickup_sound = agk::LoadSound("Pickup_Coin4.wav");

	//creating "light" for when underground and "ground" to cover up blocks when above goWalkd
	ground = agk::CreateSprite(agk::LoadImage("ground.png"));
	light = agk::CreateSprite(agk::LoadImage("light.png"));
	agk::SetSpriteOffset(light, agk::GetSpriteWidth(light)/2, agk::GetSpriteHeight(light)/2);
	light_scale= 2.5;
	light_burn_rate = 0.0003;
	agk::SetSpriteScale(light, light_scale, light_scale);
	//agk::SetSpriteActive(light, 0);
	agk::SetSpriteDepth(light, 1);
	agk::SetSpriteDepth(ground, 2);

	//loading sky which follows player's horizontal movements
	sky = agk::CreateSprite(agk::LoadImage("background_sky1.png"));
	agk::SetSpriteOffset(sky, agk::GetSpriteWidth(sky)/2, agk::GetSpriteHeight(sky)/2);
	agk::SetSpriteDepth(sky, 80);

	//creating parallax_mountains
	for(int i = 0; i < 4; i++)
	{
		parallax_mountains[i] = agk::CreateSprite(agk::LoadImage ( "background_mountain3.png" ));
		agk::SetSpriteDepth(parallax_mountains[i], 70);
		agk::SetSpriteScale(parallax_mountains[i], 0.7f, 0.7f);
		agk::SetSpritePosition(parallax_mountains[i], -agk::GetSpriteWidth(parallax_mountains[i]) + i * agk::GetSpriteWidth(parallax_mountains[i]), -agk::GetSpriteHeight(parallax_mountains[i]));
	}

	//creating parallax forest
	for(int i = 0; i < 4; i++)
	{
		parallax_forest[i] = agk::CreateSprite(agk::LoadImage ( "parallax_mountain_trees.png" ));
		agk::SetSpriteDepth(parallax_forest[i], 70);
		agk::SetSpriteScale(parallax_forest[i], 0.78f, 0.78f);
		agk::SetSpritePosition(parallax_forest[i], -agk::GetSpriteWidth(parallax_forest[i]) + i * agk::GetSpriteWidth(parallax_forest[i]), -agk::GetSpriteHeight(parallax_forest[i]));
	}
}


//*********PLAYER TRACKING FUNCTIONS*************//
inline float Player::GetX()
{return agk::GetSpriteXByOffset(this->sprite);}

inline float Player::GetY()
{return agk::GetSpriteYByOffset(this->sprite);}

inline float Player::GetVX()
{return agk::GetSpritePhysicsVelocityX(this->sprite);}

inline float Player::GetVY()
{return agk::GetSpritePhysicsVelocityY(sprite);}


inline void Player::SyncThings()
{
	agk::SetSpriteAngle(sprite, 0);
	stone_falling = false;

	//sync view (EDIT SPRITES TO BE SMALLER TO SAVE MEMORY)
	agk::SetViewOffset( GetX() - 1280 / 2.0f, GetY() - 720 / 2.0f); //resolution should be variable
	agk::SetSpritePosition(sky, GetX()- agk::GetSpriteWidth(sky)/2, -agk::GetSpriteHeight(sky)); //align sky sprite and player sprite every frame
	agk::SetSpritePosition(ground, GetX()- agk::GetSpriteWidth(sky)/2, 0); //align ground in the same way
	agk::SetSpritePositionByOffset(light, GetX()-4.0f, GetY()-4.0f);//manual adjustments for light offset, need to fix this(edit all these sprites)

	//While player is underground
	if(int(GetY()) > 0)
	{
		agk::SetSpriteVisible(light, 1);//turn light on
		if(light_scale >= 1) light_scale -= light_burn_rate; //start reducing the light radius
		agk::SetSpriteScaleByOffset(light, light_scale, light_scale); //sync light to player position
		agk::SetSpriteVisible(ground, 0); //hide sprite covering the ground
		//agk::Print(light_scale);
	}
	else //while player is above ground
	{
		//agk::SetSpriteScale(light, 1.5, 1.5); //temporary feature to restore latern oil when returning to surface
		agk::SetSpriteVisible(light, 0); //turn off light
		agk::SetSpriteVisible(ground, 1); //hide blocks
	}
	
	//reset values every frame
	x_dir = 0;
	y_dir = 0;

	//if any direciton key is pressed durring digging, stop digging
	if(agk::GetSpriteVisible(dig_anim))
	{
		if(agk::GetRawKeyPressed(LEFT)
			|| agk::GetRawKeyPressed(RIGHT)
			|| agk::GetRawKeyPressed(UP)
			|| agk::GetRawKeyPressed(DOWN))
			StopDigging();
	}
}


//****************INPUT FUNCTION****************//
//Update is called every frame, so limit the amount of checks per frame
//Includes checks and functionality for all player input (basically its the player's main function)
//GetRawKeyState() will return true when the button is held
//GetRawKeyPressed() returns true only when the button is first pushed
//x_dir and y_dir telegraph the player's direciton, and correspond with agk's coordinate system.
void Player::Update()
{
	SyncThings(); //synchronize all things that follow player around
	if(agk::GetSpritePhysicsVelocityY(sprite) > 500) agk::Print("dead");

	if (agk::GetRawKeyState(LEFT)) x_dir = -1.0f;
	else if(agk::GetRawKeyState(RIGHT)) x_dir = 1.0f;
	else if(agk::GetRawKeyState(UP)) y_dir = -1.0f;
	else if(agk::GetRawKeyState(DOWN)) 
	{
		y_dir = 1.0f; 
		//if at the bottom of the ladder, allow player to fall
		if( agk::GetSpriteHitGroup(EMPTY, GetX(), GetY() + yOffset) ) agk::SetPhysicsGravity(0, gravity);
	}
	else {x_dir = 0; y_dir = 0;}
	
//Directional input is organize with else statements so that if a player is moving left, it doesn't bother checking any other input

//Update player state based off of current x_dir and y_dir (switch statement limits checks per frame)
	switch(int(x_dir))
	{
		case 0: //neither left nor right is being pushed
			StopMoving();
			switch(int(y_dir))
			{
				case 0://Player not holding any direction
					StopDigging();
					break;
				default: //Up or down
					StopMoving();
					if(CanDig(0, y_dir)) 
						Dig( 0, y_dir );
					else 
					{
						StopDigging();
						if(OnLadder()) 
							Climb(y_dir);
					}
					break;
			}
			break;
		default: //left or right
			Walk(x_dir);
			if(CanDig(x_dir, 0) && GetX() > 11.0f && GetX() < 3188.0f)
				Dig(x_dir, 0);
			else 
				StopDigging();
			break;
	}
			
//SHIFT
	if(agk::GetRawKeyPressed(SHIFT)) 
		if(agk::GetSpriteHitGroup(DIRT, GetX(), GetY()+32.0f) || agk::GetSpriteHitGroup(STONE, GetX(), GetY()+32.0f)) 
		{
			if(GetY()>30)
			building_ladder = true;
		}

//SPACEBAR
	if(agk::GetRawKeyPressed(SPACE))
		//if player is on the ground(-1) or on a ladder(0) then jump
		if(int(GetVY()) == -1 || int(GetVY()) == 0) {Jump();}

//F1 for Debug Mode
	if (agk::GetRawKeyPressed (112))
	{
		if (debug == false) {debug = true;}
		else {debug = false;}
	}

	Parallax(x_dir);
	Debug(); //displays debug variables if debug mode is on
}

//Displays debug variables
void Player::Debug()
{
	float mouse_x, mouse_y;
	if(debug)
	{
		agk::Print(int(agk::ScreenFPS( ))); //print frames per second
		agk::Print(agk::GetRawLastKey()); //which key is being pressed
		agk::Print( int(GetX()) );//print current x position
		agk::Print( int(GetY()) );//print current y position
		agk::Print( int(GetVX()) );//print current x velocity
		agk::Print( int(GetVY()) );//print current y velocity
		agk::Print( int(y_dir) );
		agk::Print( int(y_dir) );

		mouse_x = agk::GetViewOffsetX() + agk::GetRawMouseX();
		mouse_y = agk::GetViewOffsetY() + agk::GetRawMouseY();

		if(CheckBlock(mouse_x, mouse_y))
			agk::Print("RESOURCE!");
		agk::SetPhysicsDebugOn();
	}
	else
		agk::SetPhysicsDebugOff();
}

//background moving function
void Player::Parallax(float x_dir)
{
	int h_scroll = 1; //1 to enable horizontal parallax scrolling, 0 to disable
	float mountain_speed = 2.0f; //rate at which to move mountains
	float forest_speed = 1.0f; //rate at which to move th forest
	float delta_x1, delta_y1, delta_x2, delta_y2; //change in x and y positions of background

	//if player is above ground
	if(int(GetY()) <=-15)
	{
		//if player is at edge of map, turn off horizontal scrolling
		if(int(GetX()) <= 42 || int(GetX()) >= 3189) h_scroll = 0;

		//mountain parallax
		for(int i = 0; i < 4; i++)
		{
			//calculate the change in x and y positions
			delta_x1 = agk::GetSpriteX(parallax_mountains[i]) + x_dir * mountain_speed * h_scroll; //current x + movement direction * rate of movement
			delta_y1 = GetY() - agk::GetSpriteHeight(parallax_mountains[i]) + agk::GetSpriteHeight(sprite)/2;

			agk::SetSpritePosition(parallax_mountains[i], delta_x1, delta_y1);
		}

		//forest parallax
		for(int i = 0; i < 4; i++)
		{
			delta_x2 = agk::GetSpriteX(parallax_forest[i]) + x_dir * forest_speed * h_scroll;
			delta_y2 = GetY() - agk::GetSpriteHeight(parallax_forest[i]) + agk::GetSpriteHeight(sprite)/2;

			agk::SetSpritePosition(parallax_forest[i], delta_x2, delta_y2);
		}
	}
}

//**************MOVING FUNCTIONS****************//
inline void Player::Walk(float dir)
{
	//flip sprite so it is facing right direction
	if(int(dir) == -1) {agk::SetSpriteFlip (sprite, 180, 0);}
	else		       {agk::SetSpriteFlip (sprite, 0, 0);}
	
	agk::SetSpritePhysicsVelocity (sprite, dir * walk_speed, GetVY()); //move by Walkspeed * direction
	if(!OnLadder()) agk::SetPhysicsGravity(0, gravity); //if player moves off a ladder, restore gravity
}

inline void Player::Jump()
{
	agk::SetSpritePhysicsVelocity(sprite, GetVX(), -jump_speed); //Allow jump, retaining current horizontal speed 
	agk::PlaySound(jump_sound, 10, 0);
	agk::SetPhysicsGravity(0, gravity); //When player jumps on a ladder, gravity must be restored
	StopDigging(); //jump cancels out digging
}

inline void Player::Climb(float dir)
{
	//move up or down
	agk::SetSpritePosition(sprite, agk::GetSpriteX(sprite), agk::GetSpriteY(sprite) + (2.0f * dir));
	//agk::SetSpriteAngle(sprite, 0);
	agk::SetPhysicsGravity(0, 0); //remove effects of gravity while climbing
	StopDigging(); //cannot dig while climbing
}

//****************DIGGING FUNCTIONS**************//
//checks if player is able to dig block at player-relative x and y tile position
inline bool Player::CanDig(float x, float y)
{
	//if player is in the mine (underground) && player is on ground or ladder %% not at edge of map
	if(GetY() > 32.0f && int(GetVY()) == 0)
	{
		//Establish hit detection ranges
		x = GetX() + (x*xOffset); 
		y = GetY() + (y*yOffset);

		//grab ID from block to dig
		unsigned int blockID = agk::GetSpriteHitGroup(DIRT, x, y);

		if(agk::GetSpriteHitGroup(DIRT, x, y) //if dirt (also check resources)
			&& agk::GetSpriteY(blockID) > 30.0f) //if not top dirt
			return true; //allow digging
		else
			return false; //else reject dig action
	}
	else return false;
}

inline void Player::Dig(float x, float y)
{
	x = GetX() + (x*xOffset);
	y = GetY() + (y*yOffset);
	//grab ID of block being dug (change to function for checking hitgroup of all resource types)
	unsigned int blockID = agk::GetSpriteHitGroup(DIRT, x, y);
	float block_x =  agk::GetSpriteX(blockID);
	float block_y =  agk::GetSpriteY(blockID);

	//move animation to current block
	agk::SetSpritePosition(dig_anim, block_x, block_y);

	//if dig animaiton not visible yet (not digging)
	if(!agk::GetSpriteVisible(dig_anim))
	{
		if(!agk::GetSoundsPlaying(dig_sound)) agk::PlaySound(dig_sound, 50, 1, 1);
		//animate dirt being dug
		agk::PlaySprite(dig_anim, dig_speed - GetY()/32.0f, 0);
		agk::SetSpriteVisible(dig_anim, 1);
		agk::PlaySprite(sprite, pickaxe_speed, 1);
	}
	else //Player is digging
	{
/*********************BLOCK BREAK*******************************/
		if(!agk::GetSpritePlaying(dig_anim))
		{
			//agk::PlaySound(break_sound, 100, 0, 0);  //This sound sucks, need a new one or none at all
			if(CheckBlock(x, y)) agk::PlaySound(pickup_sound, 30);
			BlockBreakParticleEffect(block_x, block_y);
			done_digging = true;
			this->blockX = block_x;
			this->blockY = block_y;

			//if player digs under a stone, trigger stone falling effect
			if(agk::GetSpriteHitGroup(STONE, block_x + 16.0f, block_y - 30.0f))
			{
				//agk::SetSpritePhysicsVelocity(agk::GetSpriteHitGroup(STONE, block_x + 16.0f, block_y - 30.0f), 0.0f, 40.0f);
				agk::PlaySprite(agk::GetSpriteHitGroup(STONE, block_x + 16.0f, block_y - 30.0f), 20, 0);
			    //stone_falling = true;
			}

			StopDigging();
		}
	}
}

bool Player::CheckBlock(int group, float x, float y)
{
	if(agk::GetSpriteHitGroup(group, x, y))
		{return true;}
	else {return false;}
}
bool Player::CheckBlock(float x, float y)
{
	//if(agk::GetSpriteHitGroup(DIRT, x, y)) return true;
	//else if(agk::GetSpriteHitGroup(STONE, x, y)) return true;
	if(agk::GetSpriteHitGroup(COAL, x, y)) return true;
	else if(agk::GetSpriteHitGroup(COPPER, x, y)) return true;
	else if(agk::GetSpriteHitGroup(TIN, x, y)) return true;
	else if(agk::GetSpriteHitGroup(IRON, x, y)) return true;
	else if(agk::GetSpriteHitGroup(EMERALD, x, y)) return true;
	else if(agk::GetSpriteHitGroup(GOLD, x, y)) return true;
	else return false;
}

void Player::BlockBreakParticleEffect(float x, float y)
{
	unsigned int bp = agk::CreateParticles(x, y);
	agk::AddParticlesForce(bp, 0.0f, 1.00f, 0.0f, 0.0f);
	agk::SetParticlesAngle(bp, 80);
	agk::AddParticlesColorKeyFrame(bp, 10.0f, 176, 90, 49, 200);
	agk::SetParticlesDirection(bp, 0.0f, 50.0f);
	agk::SetParticlesSize(bp, 8.5f);
	agk::SetParticlesStartZone(bp, 2.0f, 5.0f, 32.0f, 10.0f);
	agk::SetParticlesLife(bp, 0.8f);
	agk::SetParticlesMax(bp, 8);
	agk::SetParticlesDepth(bp, 3);
	agk::SetParticlesFrequency(bp, 110.0f);
}

//TRUE if player is on ladder, FALSE if he is not
inline bool Player::OnLadder()
{
	if(agk::GetSpriteHitGroup(LADDER, GetX(),GetY() + (yOffset * y_dir))
		&& agk::GetSpriteHitGroup(LADDER, GetX(),GetY())) //Limit the range of climbing
		{return true;}
	else {return false;}
}

inline void Player::StopMoving()
{
	agk::SetSpritePhysicsVelocity (sprite, 0, GetVY());
	if(!agk::GetSpritePlaying(sprite))
	{
		agk::StopSprite(sprite);
		agk::SetSpriteFrame(sprite, 2);
	}
}

void Player::StopDigging()
{
	//stop block dig animation
	agk::SetSpriteVisible(dig_anim, 0);
	agk::StopSprite(dig_anim);
	agk::SetSpriteFrame(dig_anim, 1);
	
	//stop player sprite animation
	agk::StopSprite(sprite);
	agk::SetSpriteFrame(sprite, 2);

	agk::StopSound(dig_sound);
}
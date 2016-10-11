    // Created by Elaine Lin @ UCLA CS32
    // March 2016
    // Header file for all actors

#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"

class Actor : public GraphObject
{
public:
	// c'tor & d'tor
	Actor(StudentWorld* worldPtr, int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0);
	virtual		  ~Actor() { ; }

	//accessors
	StudentWorld* getWorld() { return m_world; }
	bool		  getAlive() { return m_alive; }
	virtual bool  isAgent() { return false; }

	//mutators
	virtual void  annoyed(int amt, Actor* whoAnnoysMe) { ; }
	virtual void  foundGold() { ; }
	virtual void  decCount() { ; } //decrease total count of such objects in studentworld
	virtual void  succeedInAnnoying(bool canBeBribed) { ; }
	virtual void  doSomething() = 0;
	void		  setAlive(bool state) { m_alive = state; return; }

	//Utilities
	bool		  nextStep(int dir, int& next_x, int& next_y, char character);
	//changes next_x and next_y to coordinates of where the actor shall travel to after one tick from its current position without any limitation

	Direction	  directionConverter(int input);
	int			  directionConverter(Direction dir);
	int			  distSquared(int x1, int x2, int y1, int y2);

private:
	StudentWorld* m_world;
	bool		  m_alive;
};

class FrackMan : public Actor
{
public:
	FrackMan(StudentWorld* worldPtr);
	~FrackMan() { ; }

	//Accessors
	int          getHealth() { return m_hitPts; }
	int          getWater() { return m_squirtUnits; }
	int          getGold() { return m_goldNuggets; }
	int			 getSonar() { return m_sonarCharge; }
	bool         isAgent() { return true; }

	//Mutators
	void         doSomething();
	void		 changeGold(int amt) { m_goldNuggets += amt; }
	void         changeSonar(int amt) { m_sonarCharge += amt; }
	void		 changeSquirt(int amt) { m_squirtUnits += amt; }
	void		 decHitPts(int amt);
	void		 annoyed(int decHits, Actor* whoAnnoyedMe);
	bool		 dirt_overlap(int playerX, int playerY);
private:
	int			 m_hitPts;
	int			 m_squirtUnits;
	int			 m_sonarCharge;
	int			 m_goldNuggets;
};


class Dirt : public Actor
{
public:
	Dirt(StudentWorld* worldPtr, int startX, int startY)
		:Actor(worldPtr, IID_DIRT, startX, startY, right, 0.25, 3) {
		;
	}
	~Dirt() { ; }
	void doSomething() { ; }
};

class Boulder : public Actor
{
public:
	Boulder(StudentWorld* worldPtr, int startX, int startY)
		:Actor(worldPtr, IID_BOULDER, startX, startY, down) {
		m_state = 1;
	} //stable
	~Boulder() { ; }

	// mutators
	void         doSomething();
	void         succeedInAnnoying(bool canBeBribed = false) { getWorld()->increaseScore(500); }
	void         decCount() { getWorld()->decNumBoulder(); }
private:
	int			 m_state;
};


class Squirt : public Actor //shot by FrackMan
{
public:
	Squirt(StudentWorld* worldPtr, int startX, int startY, Direction dir)
		: Actor(worldPtr, IID_WATER_SPURT, startX, startY, dir, 1.0, 1) {
		m_travelDist = 4;
	}//initial travel distance is four squares
	~Squirt() { ; }

	// mutators
	void         succeedInAnnoying(bool canBeBribed = false); //false == regular protester
	void         doSomething();

private:
	int			 m_travelDist;
};

class RegularProtester : public Actor //regular protesters
{
public:
	RegularProtester(StudentWorld* worldPtr, int startX, int startY, int imageID = IID_PROTESTER, int hitPts = 5, int scoreWorthSquirted = 100, bool acceptBribe = true);
	virtual      ~RegularProtester() { ; }

	//Accessors
	int          getScoreWorthSquirted() { return m_scoreWorthSquirted; }; //return how much score should increase when squirted (depends on type of protester)
	int          playerInDirectSight(int& next_x, int& next_y, int& dir); //player in direct line of sight, vertical = 1, horizontal = 2, neither (blocked) = 0
	int          pickRandomDirection(); //pick a random and viable direction to move protester one step (provide coordinates. Do not move protester. Return viable direction.
	int          canMovePerpendicularly(int& toDirection1, int& toDirection2); //return 0 if absolutely cannot, return 1 if can move in one perpendicular direction, return 2 if both perpendicular directions are available. toDirection1 and toDirection2 changed to direction coding in terms of integer if it is a viable option, or set to 0 if not a viable direction.
	bool         faceAndNextToPlayer(); //return true if protester is facing player and right next to the player
	bool         withinFourUnits();
	bool         isAgent() { return true; }
	void		 shortestReturnRoute(int& x, int& y, int start_x, int start_y);

	// Mutators
	void         checkIfTurningPerpendicularly(int currentDir, int changeToDir);
	void         doSomething();
	void         generateNumSquaresToMove() { m_numSquaresToMoveInCurrentDirection = 8 + rand() % 53; } //generate random number from 8 - 60
	void         annoyed(int decHits, Actor* whoAnnoyedMe);
	void         foundGold();
	void         decCount() { getWorld()->decNumProtester(); }

private:
	int			 m_numSquaresToMoveInCurrentDirection;
	int			 m_hitPts;
	int			 m_ticksToWaitBetweenMoves; //if num resting ticks exceeds this threshold, protester should move
	int			 m_restingTicksElapsed; // num of resting ticks elapsed
	int			 m_movingTicksNotShouted; //++ if protester does not shout during each moving ticks
	int          m_movingTicksNotTurnedPerp; //++ if protester does not turn in a perpendicular direction during each moving ticks
	int          m_scoreWorthSquirted;
	bool         m_acceptBribe; //regular accepts bribe and leaves, hardcore stares at bribe and continues
	bool		 m_leaveOilField;
};

class HardcoreProtester : public RegularProtester
{
public:
	~HardcoreProtester() { ; }
	HardcoreProtester(StudentWorld* worldPtr, int startX, int startY)
		: RegularProtester(worldPtr, startX, startY, IID_HARD_CORE_PROTESTER, 20, 250, false) {
		;
	}
};

class Goodie : public Actor //anything that can be picked up
{
public:
	Goodie(StudentWorld* worldPtr, int startX, int startY, int imageID, int scoreWorth, int unitsEquivalent = 1, bool permanent = false, bool pickupByFrackMan = true, bool shouldIDisplay = true, int gotGoodieSound = SOUND_GOT_GOODIE);
	virtual		 ~Goodie() { ; }

	// Accessors
	bool         getPermanent() { return m_permanent; }
	bool         getPlayerPickup() { return m_playerPickUp; }
	void         gotGoodie(int score);
	int          getLife() { return m_lifetime; }

	// Mutators

	void         decLife() { m_lifetime--; }
	void         doSomething();
	virtual void addGoodieFromPlayer(int amt) = 0;
private:
	bool		 m_permanent;    // temporary if false, permanent if true
	bool		 m_playerPickUp; // true if gold is for FrackMan, false if for protesters
	int			 m_lifetime;     // number of ticks it can survive
	int          m_scoreWorth;  //score to increment by after pickup
	int          m_unitsEquivalent; //number to incrementt by after pickup
	int			 m_gotGoodieSound;

};

class Oil : public Goodie
{
public:
	Oil(StudentWorld* worldPtr, int startX, int startY)
		: Goodie(worldPtr, startX, startY, IID_BARREL, 1000, -1, true, true, false,SOUND_FOUND_OIL)
	{
		;
	}
	virtual		 ~Oil() { ; }

	//mutators
	void         addGoodieFromPlayer(int amt) { ; }
	void         decCount() { getWorld()->decNumOil(); }
};

class GoldNugget : public Goodie
{
public:
	GoldNugget(StudentWorld* worldPtr, int startX, int startY, bool shouldIDisplay)
		: Goodie(worldPtr, startX, startY, IID_GOLD, 10, 1, !shouldIDisplay, !shouldIDisplay, shouldIDisplay) {
		;
	}
	~GoldNugget() { ; }

	// mutators
	void         addGoodieFromPlayer(int amt) { getWorld()->getPlayer()->changeGold(amt); }
	void         decCount() { getWorld()->decNumGoldNugget(); }
};

class SonarKit : public Goodie
{
public:
	SonarKit(StudentWorld* worldPtr, int startX = 0, int startY = 60)
		: Goodie(worldPtr, startX, startY, IID_SONAR, 75) {
		;
	}
	~SonarKit() { ; }

	//mutators
	void         addGoodieFromPlayer(int amt) { getWorld()->getPlayer()->changeSonar(amt); }
	void         decCount() { getWorld()->decNumSonar(); }
};

class WaterRefill : public Goodie
{
public:
	WaterRefill(StudentWorld* worldPtr, int startX, int startY)
		:Goodie(worldPtr, startX, startY, IID_WATER_POOL, 100, 5) {
		;
	}
	~WaterRefill() { ; }

	//mutator
	void addGoodieFromPlayer(int amt) { getWorld()->getPlayer()->changeSquirt(amt); }
	//decCount() not defined is OKAY because it is inherited from class Actor with default {;} implementation
};

class Coord
{
public:
	Coord(int xx, int yy) : m_x(xx), m_y(yy) { ; }
	int x() const { return m_x; }
	int y() const { return m_y; }
private:
	int m_x;
	int m_y;
};
#endif // ACTOR_H_
//12:33pm

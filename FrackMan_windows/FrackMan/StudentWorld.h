    // Created by Elaine Lin @ UCLA CS32
    // March, 2016
    // Header file for each level of game (aka StudentWorld)
    // Is a subclass of GameWorld, which controls the world across all levels

#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <list>
class Actor;
class FrackMan;
class Dirt;
#include <string>

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir);
	virtual ~StudentWorld();
	FrackMan* getPlayer() { return m_player; }
	Dirt* getDirt(int col, int row) { return m_dirt[col][row]; } //row and col should range between 0 to max-1
	void eraseDirt(int col, int row);
	virtual int init();
	virtual int move();
	virtual void cleanUp();
	std::list<Actor*> getActor() { return m_lActor; }
	int getNumBoulder() { return m_numBoulder; }
	int getNumGoldNugget() { return m_numGoldNugget; }
	int getNumOil() { return m_numOil; }
	void decNumBoulder() { m_numBoulder--; }
	void decNumGoldNugget() { m_numGoldNugget--; }
	void decNumOil() { m_numOil--; }
	void decNumSonar() { m_numSonar--; }
	void decNumProtester() { m_numProtestersAllowed++; } //kill one protester allows one more to be added to the world
	void incNumProtester() { m_numProtestersAllowed--; }
	void generateCoord(int &x, int &y);
	bool generateCoordNoDirt(int &x, int &y, int limit); //return true if coordinates valid, return false if no valid coord found after recursion of a limit number of times
	void pushBackActorToList(Actor* toAdd);
	
private:
	FrackMan* m_player;
	Dirt* m_dirt[VIEW_HEIGHT][VIEW_WIDTH];
	std::list<Actor*> m_lActor;
	int m_numBoulder;
	int m_numGoldNugget;
	int m_numOil;
	int m_numSonar;
	int m_ticksToWaitInBetweenAdd;
	int m_numProtestersAllowed;
	int m_ticksElapsed; //to time protester addition
};

#endif // STUDENTWORLD_H_
//12:33pm

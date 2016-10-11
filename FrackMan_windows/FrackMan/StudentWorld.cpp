    // Created by Elaine Lin @ UCLA CS32
    // March, 2016
    // Implementation of StudentWorld
    // Creates the world for each level

#include "StudentWorld.h"
#include <string>
#include "Actor.h"
#include <cstdlib>
#include <time.h>
#include <algorithm>
#include "GameController.h"
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

bool StudentWorld::generateCoordNoDirt(int &x, int&y, int limit)
{
	if (limit == 0)
		return false;
	x = rand() % 61;
	y = rand() % 61;
	int playerX = getPlayer()->getX();
	int playerY = getPlayer()->getY();
	bool generateAgain = false;
	for (int i = 0; i < 4 && generateAgain == false; i++)
	{
		for (int j = 0; j < 4; j++)
			if (getDirt(x + i, y + j) != nullptr)
			{
				generateAgain = true;
				break;
			}
	}
	if (generateAgain == false)
	{
		if ((playerX - 3 <= x) && (playerX + 3 >= x) && (playerY - 3 <= y) && (playerY + 3 >= y)) //overlap player
			generateAgain = true;
		else
		{
			list<Actor*>::iterator it;
			it = m_lActor.begin();
			while (it != m_lActor.end() && generateAgain == false)
			{
				int actorX = (*it)->getX();
				int actorY = (*it)->getY();
				if ((actorX - 3 <= x) && (actorX + 3 >= x) && (actorY - 3 <= y) && (actorY + 3 >= y))
					generateAgain = true;
				it++;
			}
		}
	}
	if (generateAgain)
	{
		limit--;
		return generateCoordNoDirt(x, y, limit);
	}
	return true;
}

void StudentWorld::generateCoord(int &x, int &y) //generate x y randomly from 0-60 & 20-56
{
	x = rand() % 61;
	while ((x <= 33 && x >= 27))
		x = rand() % 61;
	y = rand() % 37 + 20;
	if (!m_lActor.empty())
	{
		list<Actor*>::iterator it = m_lActor.begin();
		while (it != m_lActor.end())
		{
			int dist = (*it)->distSquared(x, (*it)->getX(), y, (*it)->getY());
			if (dist <= 36)
			{ //needs to be > 6 distance away from all objects
				generateCoord(x, y);
				return;
			}
			it++;
		}
	}
}

StudentWorld::StudentWorld(std::string assetDir)
	: GameWorld(assetDir)
{
	m_player = nullptr;
	for (int i = 0; i <= VIEW_HEIGHT - 1; i++)
		for (int j = 0; j <= VIEW_WIDTH - 1;j++)
			m_dirt[i][j] = nullptr;

	m_numGoldNugget = m_numBoulder = m_numOil = m_numSonar = 0;
}

void StudentWorld::eraseDirt(int col, int row)
{
	if (m_dirt[col][row] != nullptr)
	{
		delete m_dirt[col][row];
		m_dirt[col][row] = nullptr;
	}
}

int StudentWorld::init()
{
	//initializing FrackMan
	m_player = new FrackMan(this);

	//initializing dirt
	for (int j = 0; j < 60; j++)
	{
		for (int i = 0; i < VIEW_WIDTH; i++)
		{
			if ((i<30 || i>33) || (j<4 && i>=30 && i<= 33))
				m_dirt[i][j] = new Dirt(this, i, j);
		}
	}

	//determine number of boulder, goldnugget, oil
	int level = getLevel();
	m_numBoulder = min(level / 2 + 2, 6);
	m_numGoldNugget = max(5 - level / 2, 2);
	m_numOil = min(2 + level, 20);
	int secondNum = 2 + level*1.5;
	m_numProtestersAllowed = min(15, secondNum);

	//initialize rest of actors
	srand(time(NULL));
	int numBoulder = m_numBoulder;
	int numOil = m_numOil;
	int numGoldNugget = m_numGoldNugget;
	//order of each Actor type on the list: boulder, oil, nugget, sonar charge/water/protesters
	while (numGoldNugget != 0)
	{
		int x, y;
		while (numOil != 0)
		{
			while (numBoulder != 0)
			{
				generateCoord(x, y);
				m_lActor.push_back(new Boulder(this, x, y));
				for (int i = x; i < x + 4; i++)
					for (int j = y; j < y + 4; j++)
						eraseDirt(i, j);
				numBoulder--;
			}
			generateCoord(x, y);
			m_lActor.push_back(new Oil(this, x, y));
			numOil--;
		}
		generateCoord(x, y);
		m_lActor.push_back(new GoldNugget(this, x, y, false));
		numGoldNugget--;
	}

	m_lActor.push_back(new RegularProtester(this, 60, 60));
	m_numProtestersAllowed--;
	m_ticksToWaitInBetweenAdd = max(25, 200 - level);
	m_ticksElapsed = 0;

	return GWSTATUS_CONTINUE_GAME;

}

int StudentWorld::move()
{
	//update status text
	if (m_player != nullptr)
	{
		int health = 10 * (getPlayer()->getHealth());
		string stat = "Scr: " + to_string(getScore()) + " Lvl: " + to_string(getLevel()) + " Lives: " + to_string(getLives()) + " Hlth: " + to_string(health) + "% Wtr: " + to_string(getPlayer()->getWater()) + " Gld: " + to_string(getPlayer()->getGold()) + " Sonar: " + to_string(getPlayer()->getSonar()) + " Oil Left: " + to_string(m_numOil);
		setGameStatText(stat);
	}

	//FrackMan does something
	if ((m_player != nullptr) && m_player->getAlive() && m_numOil != 0)
		m_player->doSomething();
	//StudentWorld should detect that FrackMan has died and address appropriately

	//rest of actors do something
	list<Actor*>::iterator it;
	it = m_lActor.begin();
	while (!m_lActor.empty() && it != m_lActor.end() && (*it) != nullptr && getPlayer() != nullptr && getPlayer()->getAlive() && m_numOil != 0)
	{
		(*it)->doSomething();
		it++;
	}

	//clean up dead actors after all have done something from list of actors
	it = m_lActor.begin();
	while (it != m_lActor.end())
	{
		if (!(*it)->getAlive())
		{
			(*it)->decCount();
			delete *it;
			it = m_lActor.erase(it);
		}
		else
			it++;
	}
	//check if player died
	if (!getPlayer()->getAlive())
	{
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}
	if (m_numOil <= 0)
	{
		GameController::getInstance().playSound(SOUND_FINISHED_LEVEL);
		return GWSTATUS_FINISHED_LEVEL;
	}

	//Add new protesters
	if ((m_ticksElapsed < m_ticksToWaitInBetweenAdd)  || (m_numProtestersAllowed <= 0))
		m_ticksElapsed++; //no adding
	else
	{
		int level = getLevel();
		int x = 60, y = 60;
		int probHardcore = min(90, level * 10 + 30);
		int chance = rand() % probHardcore + 1; //random number from 1 to probHardCore
		if (chance == 1) //add hardcore
			m_lActor.push_back(new HardcoreProtester(this, x, y));
		else
			m_lActor.push_back(new RegularProtester(this, x, y));
		m_numProtestersAllowed--;
		m_ticksElapsed = 0;
	}
	//Add new characters (sonar or water) by probability

	int G = getLevel() * 25 + 300; //one out of G chance of any new addition
	int chance = rand() % G + 1; //random number from 1 to G
	if (G == chance)
	{
		int chance2 = rand() % 5; //random number from 0 - 4
 		if (chance2 == 0 && m_numSonar == 0)
		{
			m_lActor.push_back(new SonarKit(this));
			m_numSonar++;
		}
		else
		{
			int x, y;
			int limit = 1000;
			if (generateCoordNoDirt(x, y, limit))
				m_lActor.push_back(new WaterRefill(this, x, y)); //needs to be generated at places w/o dirt
		}

	}

	return GWSTATUS_CONTINUE_GAME;

}

void StudentWorld::cleanUp()
{
	if (m_player != nullptr)
		delete m_player;
	for (int i = 0; i <= VIEW_WIDTH - 1; i++)
	{
		for (int j = 0; j <= VIEW_HEIGHT - 1; j++)
			if (m_dirt[i][j] != nullptr)
				delete m_dirt[i][j];
	}
	list<Actor*>::iterator it;
	for (it = m_lActor.begin(); it != m_lActor.end(); it++)
		if (*it != nullptr)
			delete *it;
	m_lActor.erase(m_lActor.begin(), m_lActor.end());
}


StudentWorld::~StudentWorld()
{
	if (m_player != nullptr)
		delete m_player;
	for (int i = 0; i <= VIEW_WIDTH - 1; i++)
	{
		for (int j = 0; j <= VIEW_HEIGHT - 1; j++)
			if (m_dirt[i][j] != nullptr)
				delete m_dirt[i][j];
	}
	list<Actor*>::iterator it;
	for (it = m_lActor.begin(); it != m_lActor.end(); it++)
		if (*it != nullptr)
			delete *it;
	m_lActor.erase(m_lActor.begin(), m_lActor.end());
}

void StudentWorld::pushBackActorToList(Actor* toAdd)
{
	m_lActor.push_back(toAdd);
}

//12:33pm
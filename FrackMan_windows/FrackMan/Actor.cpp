    // Created by Elaine Lin @ UCLA CS32
    // March 2016
    // Implementation of all Actors

#include "Actor.h"
#include "StudentWorld.h"
#include "GameController.h"
#include <algorithm>
#include <list>
#include <queue>
#include <vector>
using namespace std;

///////////////////////////
///Actor implementation////
///////////////////////////


Actor::Actor(StudentWorld* worldPtr, int imageID, int startX, int startY, Direction dir, double size, unsigned int depth)
	:GraphObject(imageID, startX, startY, dir, size, depth)
{
	m_world = worldPtr;
	m_alive = true;
	setVisible(true);
}

bool Actor::nextStep(int dir, int& next_x, int& next_y, char character = 'o')
{
	int increment = 1;
	if (character == 's') //squirt
		increment = 3;
	switch (dir)
	{
	case KEY_PRESS_UP:
		next_y = next_y + increment;
		break;
	case KEY_PRESS_DOWN:
		next_y = next_y - increment;
		break;
	case KEY_PRESS_LEFT:
		next_x = next_x - increment;
		break;
	case KEY_PRESS_RIGHT:
		next_x = next_x + increment;
		break;
	default:
		return false;
	}

	if ((next_x < 0) || (next_y < 0) || (next_x >(VIEW_WIDTH - 4)) || (next_y >(VIEW_HEIGHT - 4)))
		return false; // cannot go to next step

					  //FrackMan, boulder, squirt, protesters cannot run into a boulder
	else if (character == 'f' || character == 'b' || character == 's' || character == 'p')
	{
		list<Actor*>::iterator it;
		list<Actor*> a = getWorld()->getActor();
		it = a.begin();
		int numBoulder = getWorld()->getNumBoulder();
		for (; numBoulder > 0; numBoulder--)
		{
			if ((*it) != this && (next_x > (*it)->getX() - 4 && next_x < (*it)->getX() + 4) && (next_y >(*it)->getY() - 4 && next_y < (*it)->getY() + 4))
				return false;
			it++;
		}
	}

	//Boulder can only move if next step is not bottom of oil field and if path is clear of dirt
	if (character == 'b')
	{
		if (next_y == 1)
			return false;
		else
		{
			if (getWorld()->getDirt(next_x, next_y) != nullptr || getWorld()->getDirt(next_x + 1, next_y) != nullptr || getWorld()->getDirt(next_x + 2, next_y) != nullptr || getWorld()->getDirt(next_x + 3, next_y) != nullptr)
				return false;
		}
	}
	if (character == 's' || character == 'p') //checking if squirt initialized position or protester's next step is blocked by dirt
	{
		for (int i = next_x; i < next_x + 4; i++)
			for (int j = next_y; j < next_y + 4; j++)
				if (getWorld()->getDirt(i, j) != nullptr)
					return false;
	}
	return true;
}

GraphObject::Direction Actor::directionConverter(int input)
{
	switch (input)
	{
	case KEY_PRESS_UP:
		return up;
	case KEY_PRESS_DOWN:
		return down;
	case KEY_PRESS_RIGHT:
		return right;
	case KEY_PRESS_LEFT:
		return left;
	default:
		return none;
	}
}

int Actor::directionConverter(Direction dir)
{
	switch (dir)
	{
	case up:
		return KEY_PRESS_UP;
	case down:
		return KEY_PRESS_DOWN;
	case right:
		return KEY_PRESS_RIGHT;
	case left:
		return KEY_PRESS_LEFT;
	default:
		return 0;
	}
}

bool operator<(const Actor &a, const Actor &b)
{
	return (a.getX() < b.getX());
}

int Actor::distSquared(int x1, int x2, int y1, int y2)
{
	return ((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}


////////////////////////////////////
//////FrackMan Implementation///////
////////////////////////////////////

FrackMan::FrackMan(StudentWorld* worldPtr)
	:Actor(worldPtr, IID_PLAYER, 30, 60)
{
	m_hitPts = 10;
	m_squirtUnits = 5;
	m_sonarCharge = 1;
	m_goldNuggets = 0;
}

bool FrackMan::dirt_overlap(int playerX, int playerY)
{
	bool dirtOverlap = false;

	for (int i = playerX; ((i < playerX + 4) && (i < VIEW_WIDTH)); i++)
	{
		for (int j = playerY; ((j < playerY + 4) && (j < VIEW_HEIGHT - 1)); j++)

			if ((getWorld() != nullptr) && (getWorld()->getDirt(i, j) != nullptr))
			{
				getWorld()->eraseDirt(i, j);
				dirtOverlap = true;
			}
	}
	return dirtOverlap;
}

void FrackMan::doSomething()
{
	if (getAlive() == false)
		return;
	else
	{
		int ch = 0;
		if (dirt_overlap(getX(), getY()) == true)
		{
			GameController::getInstance().playSound(SOUND_DIG);
		}
		else if ((getWorld() != nullptr) && (getWorld()->getKey(ch)) == true) //getKey returns true if user presses any valid key including directions, tab, spaces, etc.
		{
			if (ch == KEY_PRESS_UP || ch == KEY_PRESS_DOWN || ch == KEY_PRESS_LEFT || ch == KEY_PRESS_RIGHT)
			{
				int next_x = getX();
				int next_y = getY();
				if (directionConverter(ch) != getDirection())
					setDirection(directionConverter(ch));
				else if (nextStep(ch, next_x, next_y, 'f')) //&& road is clear of other actors
					moveTo(next_x, next_y);
			}
			else if (ch == KEY_PRESS_SPACE)
			{
				if (m_squirtUnits != 0)
				{
					m_squirtUnits--;
					GameController::getInstance().playSound(SOUND_PLAYER_SQUIRT);
					int next_x = getX(), next_y = getY();
					int direction = directionConverter(getDirection());
					if (nextStep(direction, next_x, next_y, 's'))
					{
						Actor* newSquirt = new Squirt(getWorld(), next_x, next_y, getDirection());
						getWorld()->pushBackActorToList(newSquirt); //squirt will be accessed from back
					}
				}
			}
			else if (ch == KEY_PRESS_ESCAPE)
			{
				setAlive(false);
				return;
			}
			else if ((ch == 'Z' || ch == 'z') && m_sonarCharge > 0) // use sonar charge to illuminate area
			{
				m_sonarCharge--;
				list<Actor*> lactor = getWorld()->getActor();
				list<Actor*>::iterator it = lactor.begin();
				int i = getWorld()->getNumBoulder();
				while (i != 0)
				{
					it++;
					i--;
				}
				//(it) points to oil as of now
				for (int j = (getWorld()->getNumOil() + (getWorld()->getNumGoldNugget())); j != 0; j--)
				{
					int actorX = (*it)->getX();
					int actorY = (*it)->getY();
					int playerX = getX();
					int playerY = getY();
					if (distSquared(actorX, playerX, actorY, playerY) <= 144)
						(*it)->setVisible(true);
					it++;
				}
			}
			else if ((ch == KEY_PRESS_TAB) && m_goldNuggets > 0)
			{
				m_goldNuggets--;
				int x = getX(), y = getY();
				Actor* p = new GoldNugget(getWorld(), x, y, true);
				getWorld()->pushBackActorToList(p);
			}
		}
	}
	return;
}

void FrackMan::annoyed(int decHits, Actor* whoAnnoyedMe)
{
	m_hitPts -= decHits;
	if (m_hitPts <= 0)
	{
		GameController::getInstance().playSound(SOUND_PLAYER_GIVE_UP);
		setAlive(false);
	}
}
void FrackMan::decHitPts(int amt)
{
	m_hitPts += amt;
	if (m_hitPts <= 0)
		setAlive(false);
}

//////////////////////////////////
//////Boulder Implementation//////
//////////////////////////////////

void Boulder::doSomething()
{
	if (getAlive() == false)
		return;
	else
	{
		StudentWorld* temp = getWorld();
		if (m_state == 1)
		{
			if (temp->getDirt(getX(), getY() - 1) || temp->getDirt(getX() + 1, getY() - 1) || temp->getDirt(getX() + 2, getY() - 1) || temp->getDirt(getX() + 3, getY() - 1))

				return;      //Stable rock does nothing
			else
				m_state = -30; //waiting state
		}
		else if (m_state < 0)
			m_state++;
		//each tick the m_state should increment by 1 if (m_state < 0)
		else if (m_state == 0) //transition into falling state
		{
			GameController::getInstance().playSound(SOUND_FALLING_ROCK);
			m_state = 2; //falling state
		}
		else if (m_state == 2) //falling state
		{
			int next_x = getX();
			int next_y = getY();
			if (nextStep(KEY_PRESS_DOWN, next_x, next_y, 'b'))
				moveTo(next_x, next_y);
			//!bottomOfOilField && pathClear)
			//move downward one square
			else
				setAlive(false); //set dead
			if (getAlive()) //boulder will not fall to its death
			{
				int playerX = getWorld()->getPlayer()->getX();
				int playerY = getWorld()->getPlayer()->getY();
				int distFrackMan = distSquared(playerX, next_x, playerY, next_y);
				if (distFrackMan <= 9)
				{
					getWorld()->getPlayer()->annoyed(100, this);
					getWorld()->getPlayer()->setAlive(false); //player is dead if hit by boulder
				}
				list<Actor*>::iterator it;
				list<Actor*> lActor = getWorld()->getActor();
				it = lActor.begin();
				for (; it != lActor.end(); it++)
				{
					if ((*it)->isAgent())
					{
						int protesterX = (*it)->getX();
						int protesterY = (*it)->getY();
						int distProtester = distSquared(protesterX, next_x, protesterY, next_y);
						if (distProtester <= 9)
							(*it)->annoyed(100, this);
					}
				}
			}
		}
	}
}

//////////////////////////////////////
////////Goodie Implementation/////////
//////////////////////////////////////

Goodie::Goodie(StudentWorld* worldPtr, int startX, int startY, int imageID, int scoreWorth, int unitsEquivalent, bool permanent, bool pickupByFrackMan, bool shouldIDisplay, int gotGoodieSound)
	:Actor(worldPtr, imageID, startX, startY, right, 1.0, 2)
{
	m_permanent = permanent;
	m_playerPickUp = pickupByFrackMan;
	int level = getWorld()->getLevel();
	m_lifetime = max(100, 300 - 10 * level);
	m_scoreWorth = scoreWorth;
	m_unitsEquivalent = unitsEquivalent;
	m_gotGoodieSound = gotGoodieSound;
	setVisible(shouldIDisplay);
}


void Goodie::gotGoodie(int score)
{
	setAlive(false);
	GameController::getInstance().playSound(m_gotGoodieSound);
	getWorld()->increaseScore(score);
}

void Goodie::doSomething()
{
	if (!getAlive())
		return;
	else
	{
		int dist = distSquared(getWorld()->getPlayer()->getX(), getX(), getWorld()->getPlayer()->getY(), getY());
		if (!isVisible() && dist <= 16)
		{
			setVisible(true);
			return;
		}
		else if (getPlayerPickup() == true && dist <= 9)
		{
			gotGoodie(m_scoreWorth);
			addGoodieFromPlayer(m_unitsEquivalent);
		}
		else if (getPlayerPickup() == false)    //only gold nuggets will have a chance to enter this if statement
		{
			list<Actor*> lActor = getWorld()->getActor();
			list<Actor*>::iterator it = lActor.begin();
			for (; it != lActor.end() && getAlive(); it++)
			{
				if ((*it)->isAgent())
				{
					int protesterX = (*it)->getX();
					int protesterY = (*it)->getY();
					dist = distSquared(protesterX, getX(), protesterY, getY());
					if (dist <= 9)
					{
						setAlive(false);
						GameController::getInstance().playSound(SOUND_PROTESTER_FOUND_GOLD);
						(*it)->foundGold();
					}
				}
			}
		}
		if (!getPermanent())
		{
			decLife();
			if (getLife() == 0)
				setAlive(false);
		}
	}
}


////////////////////////////////////
////////Squirt Implementation///////
////////////////////////////////////

void Squirt::doSomething()
{
	int next_x = getX();
	int next_y = getY();
	list<Actor*> lActor = getWorld()->getActor();
	list<Actor*>::iterator it = lActor.begin();
	for (; it != lActor.end(); it++)
		if ((*it)->isAgent())
		{
			int protesterX = (*it)->getX();
			int protesterY = (*it)->getY();
			int distProtester = distSquared(protesterX, next_x, protesterY, next_y);
			if (distProtester <= 9)
			{
				(*it)->annoyed(2, this);
				setAlive(false);
			}
		}
	//within radius of 3 of one or more protestors
	//cause 2 points of annoyance to these protesters
	if (m_travelDist == 0)
		setAlive(false);
	if (!getAlive()) //dead
		return;
	else
	{
		if (nextStep(directionConverter(getDirection()), next_x, next_y))
		{
			list<Actor*>::iterator it;
			list<Actor*> a = getWorld()->getActor();
			it = a.begin();

			int i = getWorld()->getNumBoulder();
			while (i != 0) //set to last boulder
			{
				int actorX = (*it)->getX();
				int actorY = (*it)->getY();
				if ((actorX - 3 <= next_x) && (actorX + 3 >= next_x) && (actorY - 3 <= next_y) && (actorY + 3 >= next_y))
				{
					setAlive(false); //hit boulder
					getWorld()->getPlayer()->changeSquirt(-1); //lost one squirt
					return;
				}
				i--;
				it++;
			}
			if (getAlive()) // Will not hit boulder
			{	//check if position is overlapped with dirt
				int x_limit = 0, y_limit = 0, x_start = next_x, y_start = next_y; //setting parameters for loops used for checking if dirt is present
				switch (getDirection())
				{
				case up:
					x_limit = 3;
					y_start = next_y + 3;
					break;
				case right:
					x_start = next_x + 3;
					y_limit = 3;
					break;
				case left:
					y_limit = 3;
					break;
				case down:
					x_limit = 3;
					break;
				case none:
					break;
				}
				for (int i = x_start; i <= x_start + x_limit; i++)
					for (int j = y_start; j <= y_start + y_limit; j++)
						if (getWorld()->getDirt(i, j) != nullptr)
						{
							setAlive(false); //hit dirt
							getWorld()->getPlayer()->changeSquirt(-1); //lost one squirt
							return;
						}
			}
			//does not hit dirt either
			moveTo(next_x, next_y);
			m_travelDist--;
			return;
		}
		else
		{
			setAlive(false);
			getWorld()->getPlayer()->changeSquirt(-1);
			return;
		}
	}
}

void Squirt::succeedInAnnoying(bool canBeBribed)
{
	if (canBeBribed) //regular protester
		getWorld()->increaseScore(100);
	else
		getWorld()->increaseScore(250); //increase by 250 if it were a hardcore protester
}


//////////////////////////////////////////////
///////RegularProtester Implementation////////
//////////////////////////////////////////////

RegularProtester::RegularProtester(StudentWorld* worldPtr, int startX, int startY, int imageID, int hitPts, int scoreWorthSquirted, bool acceptBribe)
	: Actor(worldPtr, imageID, startX, startY, left), m_hitPts(hitPts), m_leaveOilField(false), m_movingTicksNotShouted(0), m_movingTicksNotTurnedPerp(0), m_restingTicksElapsed(0), m_scoreWorthSquirted(scoreWorthSquirted), m_acceptBribe(acceptBribe)
{
	generateNumSquaresToMove();
	int level = getWorld()->getLevel();
	m_ticksToWaitBetweenMoves = max(0, 3 - level / 4);
}


bool RegularProtester::faceAndNextToPlayer()
{
	int playerX = getWorld()->getPlayer()->getX();
	int playerY = getWorld()->getPlayer()->getY();
	switch (getDirection())
	{
	case right:
		if (getX() >= playerX - 4 && getX() <= playerX - 1 && getY() >= playerY && getY() <= playerY + 3)
			return true;
	case left:
		if (getX() >= playerX + 1 && getX() <= playerX + 4 && getY() >= playerY  && getY() <= playerY + 3)
			return true;
	case up:
		if (getY() >= playerY - 4 && getY() <= playerY - 1 && getX() >= playerX && getX() <= playerX + 3)
			return true;
	case down:
		if (getY() >= playerY + 1 && getY() <= playerY + 4 && getX() >= playerX && getX() <= playerX + 3)
			return true;
	}
	return false;
}

bool RegularProtester::withinFourUnits()
{
	int playerX = getWorld()->getPlayer()->getX();
	int playerY = getWorld()->getPlayer()->getY();
	if ((playerX >= getX() - 4) &&
		(playerX <= getX() + 4) &&
		(playerY >= getY() - 4) &&
		(playerY <= getY() + 4) &&
		(!((playerX == getX() + 4) && ((playerY == getY() + 4) || (playerY == getY() - 4)))) &&
		(!((playerX == getX() - 4) && ((playerY == getY() + 4) || (playerY == getY() - 4)))))
		return true;
	return false;
}

int RegularProtester::playerInDirectSight(int& next_x, int& next_y, int& dir)
{
	int playerX = getWorld()->getPlayer()->getX(); //Player = frackman
	int playerY = getWorld()->getPlayer()->getY();
	bool shouldFollow = true; //initialized as true. set to false if found out not in vertical/horizontal line. not affected even if obstructed by dirt/boulder

							  //Using loops to check if any dirt/boulder is in the way. Setting double looping parameters (initialized values & limits)
	int dlower_x = 0, dupper_x = 0, dlower_y = 0, dupper_y = 0; // d = dirt
	int blower_x = 0, bupper_x = 0, blower_y = 0, bupper_y = 0; // b = boulder

	if (((playerX >= getX() - 3 && playerX <= getX() + 3) && ((playerY >= getY() + 4) || (playerY <= getY() - 4))) || (playerX == getX())) // in a  vertical line of sight
	{
		dlower_x = getX();
		dupper_x = getX() + 3;
		blower_x = getX() - 3;
		bupper_x = getX() + 3;
		next_x = getX();
		if (playerY >= getY()) //FrackMan above protester
		{
			dlower_y = getY() + 4;
			dupper_y = playerY - 1;
			next_y = getY() + 1;
			dir = KEY_PRESS_UP;
		}
		else                //FrackMan below protester
		{
			dlower_y = playerY + 4;
			dupper_y = getY() - 1;
			next_y = getY() - 1;
			dir = KEY_PRESS_DOWN;
		}
		blower_y = dlower_y;
		bupper_y = dupper_y;
	}
	else if (playerY >= getY() - 3 && playerY <= getY() + 3 && playerX != getX()) // in a horizontal line of sight)
	{
		dlower_y = getY();
		dupper_y = getY() + 3;
		blower_y = getY() - 3;
		bupper_y = getY() + 3;
		next_y = getY();
		if (playerX > getX()) // FrackMan on the right side of protester
		{
			dlower_x = getX() + 4;
			dupper_x = playerX - 1;
			next_x = getX() + 1;
			dir = KEY_PRESS_RIGHT;
		}
		else //Frackman on the left side of protester
		{
			dlower_x = playerX + 4;
			dupper_x = getX() - 1;
			next_x = getX() - 1;
			dir = KEY_PRESS_LEFT;
		}
		blower_x = dlower_x;
		bupper_x = dupper_x;
	}
	else
		shouldFollow = false;
	if (shouldFollow) //shouldFollow return true if frackman within line of sight, vertical or horizontal, obstructed or not
	{
		//check if path clear of dirt
		for (int i = dlower_x; i <= dupper_x; i++)
			for (int j = dlower_y; j <= dupper_y; j++)
				if (getWorld()->getDirt(i, j) != nullptr)
					return 0; //obstructed by dirt
							  //check if path clear of boulder
		list<Actor*> temp = getWorld()->getActor();
		list<Actor*>::iterator it;
		it = temp.begin();
		for (int i = getWorld()->getNumBoulder();i != 0; i--)
		{
			int boulderX = (*it)->getX();
			int boulderY = (*it)->getY();
			if (boulderX >= blower_x && boulderX <= bupper_x && boulderY >= blower_y && boulderY <= bupper_y)
				return 0; //obstructed by boulder
			it++;
		}
	}
	if (shouldFollow)   //shouldFollow = true && hasn't returned means passed all tests.
		if (dir == KEY_PRESS_UP || dir == KEY_PRESS_DOWN)
			return 1;   // vertical
		else
			return 2;   // horizontal
	else
		return 0; //not even in direct line of path
}

void RegularProtester::checkIfTurningPerpendicularly(int currentDir, int changeToDir)
{
	bool isPerpendicular = false;
	switch (currentDir)
	{
	case KEY_PRESS_LEFT:
	case KEY_PRESS_RIGHT:
		if (changeToDir == KEY_PRESS_UP || changeToDir == KEY_PRESS_DOWN)
			isPerpendicular = true;
		break;
	case KEY_PRESS_UP:
	case KEY_PRESS_DOWN:
		if (changeToDir == KEY_PRESS_LEFT || changeToDir == KEY_PRESS_RIGHT)
			isPerpendicular = true;
		break;
	}
	if (isPerpendicular)
		m_movingTicksNotTurnedPerp = 0;
}

int RegularProtester::pickRandomDirection()
{
	int randomDir;
	int next_x = getX();
	int next_y = getY();
	bool canMove = false;
	while (canMove == false)
	{
		randomDir = rand() % 4 + 1000; //generate random numbers from 1000-1003, which is a code for directions
		if (nextStep(randomDir, next_x, next_y, 'p'))
			canMove = true;
	}
	checkIfTurningPerpendicularly(directionConverter(getDirection()), randomDir);
	generateNumSquaresToMove();
	return randomDir;
}

int RegularProtester::canMovePerpendicularly(int& toDirection1, int& toDirection2)
{
	int currentDir = directionConverter(getDirection());
	int result = 0;
	toDirection1 = 0, toDirection2 = 0;
	int x = getX();
	int y = getY();
	switch (currentDir)
	{
	case KEY_PRESS_RIGHT:
	case KEY_PRESS_LEFT:
	{
		if (nextStep(KEY_PRESS_UP, x, y, 'p'))
		{
			result++;
			toDirection1 = KEY_PRESS_UP;
		}
		if (nextStep(KEY_PRESS_DOWN, x, y, 'p'))
		{
			result++;
			toDirection2 = KEY_PRESS_DOWN;
		}
		break;
	}
	case KEY_PRESS_UP:
	case KEY_PRESS_DOWN:
	{
		if (nextStep(KEY_PRESS_RIGHT, x, y, 'p'))
		{
			result++;
			toDirection1 = KEY_PRESS_RIGHT;
		}
		if (nextStep(KEY_PRESS_LEFT, x, y, 'p'))
		{
			result++;
			toDirection2 = KEY_PRESS_LEFT;
		}
		break;
	}
	}
	return result;
}
void RegularProtester::doSomething()
{
	if (!getAlive())
		return;
	else
	{
		if (m_restingTicksElapsed <= m_ticksToWaitBetweenMoves)	//resting state
		{
			m_restingTicksElapsed++;
			return;
		}
		else //moving ticks
		{
			m_restingTicksElapsed = 0;
			if (m_leaveOilField == true)
			{
				if (getX() == 60 && getY() == 60)
					return setAlive(false);

				else
				{
					int next_x = getX();
					int next_y = getY();
					Direction dir;
					shortestReturnRoute(next_x, next_y, 60, 60);
					if (next_x == getX()) //move up or down
						if (next_y == getY() - 1) // moved down
							setDirection(down);
						else
							setDirection(up);
					else
						if (next_x == getX() - 1) //moved left
							setDirection(left);
						else
							setDirection(right);

					moveTo(next_x, next_y);
					return;
				}
			}
			else    //Not annoyed enough to leave
			{
				int next_x = getX();
				int next_y = getY();
				int chosenDir = directionConverter(getDirection());
				int playerX = getWorld()->getPlayer()->getX();
				int playerY = getWorld()->getPlayer()->getY();
				if (withinFourUnits() && faceAndNextToPlayer())
				{
					if (m_movingTicksNotShouted >= 15)
					{
						GameController::getInstance().playSound(SOUND_PROTESTER_YELL);
						getWorld()->getPlayer()->annoyed(2, this);
						m_movingTicksNotShouted = 0;
					}
					else
						m_movingTicksNotShouted++;
					m_movingTicksNotTurnedPerp++;
					return;
				}
				else if (m_acceptBribe == false && !withinFourUnits())	//only for hardcore frackman
				{
					int level = getWorld()->getLevel();
					int M = 16 + level * 2;
					int numMovesToFrackMan = 0;
					int h_x = getX(), h_y = getY();
					shortestReturnRoute(h_x, h_y, playerX, playerY);
					int goToX = h_x, goToY = h_y;
					while (h_x != playerX || h_y != playerY)
					{
						numMovesToFrackMan++;
						shortestReturnRoute(h_x, h_y, playerX, playerY);
					}
					if (numMovesToFrackMan <= M)
					{
						if (goToX == getX()) //move up or down
							if (goToY == getY() - 1) // moved down
								setDirection(down);
							else
								setDirection(up);
						else
							if (goToX == getX() - 1) //moved left
								setDirection(left);
							else
								setDirection(right);
						moveTo(goToX, goToY);
						return;
					}
				}
				else if (distSquared(next_x, playerX, next_y, playerY) != 0)
				{
					m_movingTicksNotShouted++;
					m_movingTicksNotTurnedPerp++;
					int dir = directionConverter(getDirection());
					if (playerInDirectSight(next_x, next_y, dir) != 0) //player in either vertical or horizontal, unobstructed direct line of path
					{
						checkIfTurningPerpendicularly(directionConverter(getDirection()), dir);
						setDirection(directionConverter(dir));
						moveTo(next_x, next_y);
						m_numSquaresToMoveInCurrentDirection = 0;
						return;
					}
					else //cannot directly see FrackMan
					{
						int option1_dir = 0, option2_dir = 0;
						m_numSquaresToMoveInCurrentDirection--;
						if (m_numSquaresToMoveInCurrentDirection <= 0)
							chosenDir = pickRandomDirection();
						else if (m_movingTicksNotTurnedPerp >= 201 && canMovePerpendicularly(option1_dir, option2_dir) != 0)//if at intersection && can move at least one perp' direction && not turned perp' within past 200 steps
						{
							if ((option1_dir + option2_dir) < 2000 && (option1_dir + option2_dir) != 0) //only one direction is viable
							{
								chosenDir = option2_dir; //guess option2 is the viable one
								if (option1_dir != 0) //if turns out option 1 is the viable one
									chosenDir = option1_dir;
							}
							else    //can move in both perpendicular directions
							{
								if (rand() % 2 == 0)
									chosenDir = option1_dir;
								else
									chosenDir = option2_dir;
							}
							generateNumSquaresToMove();
							m_movingTicksNotTurnedPerp = 0;
						}
					}
				}
				next_x = getX();
				next_y = getY();
				if (nextStep(chosenDir, next_x, next_y, 'p'))
				{
					checkIfTurningPerpendicularly(directionConverter(getDirection()), chosenDir);
					setDirection(directionConverter(chosenDir));
					moveTo(next_x, next_y);
				}
				else
					m_numSquaresToMoveInCurrentDirection = 0;
				return;
				// Protester and FrackMan positions overlap
			}
		}
	}
}

void RegularProtester::annoyed(int decHits, Actor* whoAnnoyedMe)
{
	if (m_leaveOilField)
		return;
	else
	{
		m_hitPts -= decHits;
	}
	if (m_hitPts > 0)
	{
		GameController::getInstance().playSound(SOUND_PROTESTER_ANNOYED);
		int level = getWorld()->getLevel();
		int N = max(50, 100 - level * 10);
		m_restingTicksElapsed = m_ticksToWaitBetweenMoves - N;
	}
	else
	{
		m_leaveOilField = true;
		GameController::getInstance().playSound(SOUND_PROTESTER_GIVE_UP);
		m_restingTicksElapsed = 0;
		whoAnnoyedMe->succeedInAnnoying(m_acceptBribe);
	}
}

void RegularProtester::foundGold()
{
	GameController::getInstance().playSound(SOUND_PROTESTER_FOUND_GOLD);
	if (m_acceptBribe)
	{
		getWorld()->increaseScore(25);
		m_leaveOilField = true;
	}
	else
	{
		getWorld()->increaseScore(50);
		int level = getWorld()->getLevel();
		int ticks_to_stare = max(50, 100 - level * 10);
		m_restingTicksElapsed = m_ticksToWaitBetweenMoves - ticks_to_stare;
	}
}

void RegularProtester::shortestReturnRoute(int& next_x, int& next_y, int start_x, int start_y)
{
	queue<Coord> coordQueue;
	Coord start(start_x + 1, start_y + 1);
	Coord end(next_x + 1, next_y + 1);	//target node
	coordQueue.push(start);

	//Construct a maze that marks position of boulder and dirt
	int maze[66][66];
	for (int j = 0; j < 64; j++)
	{
		for (int i = 0; i < 64; i++)
			if (getWorld()->getDirt(i, j) != nullptr)
				maze[i + 1][j + 1] = 'X';
			else
				maze[i + 1][j + 1] = '.';
	}
	for (int i = 0; i <= 65; i++)
	{
		maze[i][0] = 'X';
		maze[i][65] = 'X';
	}
	for (int i = 0; i <= 65; i++)
	{
		maze[0][i] = 'X';
		maze[65][i] = 'X';
	}
	list<Actor*>::iterator it;
	list<Actor*> lActor = getWorld()->getActor();
	int numBoulder = getWorld()->getNumBoulder();
	it = lActor.begin();
	while (numBoulder != 0)
	{
		numBoulder--;
		int b_x = (*it)->getX();
		int b_y = (*it)->getY();
		for (int i = b_x + 1; i < b_x + 5; i++)
		{
			for (int j = b_y + 1; j < b_y + 5; j++)
				maze[i][j] = 'X';
		}
		it++;
	}

	while (!coordQueue.empty())
	{
		Coord current = coordQueue.front();
		int y = current.y();
		int x = current.x();
		coordQueue.pop();
		if ((current.x() - 1 == end.x() && current.y() == end.y()) || (current.x() == end.x() && current.y() + 1 == end.y()) || (current.x() + 1 == end.x() && current.y() == end.y()) || (current.x() == end.x() && current.y() - 1 == end.y()))
		{
			next_x = current.x() - 1;
			next_y = current.y() - 1;
			return;
		}
		else
		{
			if (maze[current.x() - 1][current.y()] != 'X' && maze[current.x() - 1][current.y() + 1] != 'X' && maze[current.x() - 1][current.y() + 2] != 'X' && maze[current.x() - 1][current.y() + 3] != 'X') //left
			{
				Coord p(x - 1, y);
				coordQueue.push(p);
				maze[current.x() - 1][current.y()] = 'X';
			}
			if (maze[current.x()][current.y() + 1] != 'X' && maze[current.x() + 1][current.y() + 1] != 'X'  && maze[current.x() + 2][current.y() + 1] != 'X' && maze[current.x() + 3][current.y() + 1] != 'X') //North
			{
				Coord p(x, y + 1);
				coordQueue.push(p);
				maze[current.x()][current.y() + 1] = 'X';
			}
			if (maze[current.x() + 1][current.y()] != 'X' && maze[current.x() + 1][current.y() + 1] != 'X' && maze[current.x() + 1][current.y() + 2] != 'X' && maze[current.x() + 1][current.y() + 3] != 'X') //South
			{
				Coord p(x + 1, y);
				coordQueue.push(p);
				maze[current.x() + 1][current.y()] = 'X';
			}
			if (maze[current.x()][current.y() - 1] != 'X' && maze[current.x() + 1][current.y() - 1] != 'X' && maze[current.x() + 2][current.y() - 1] != 'X' && maze[current.x() + 3][current.y() - 1] != 'X') //West
			{
				Coord p(x, y - 1);
				coordQueue.push(p);
				maze[current.x()][current.y() - 1] = 'X';
			}
		}
	}
}


//12:33pm


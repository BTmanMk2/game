#include "stonemanager.h"
#include <QGraphicsScene>
#include <QKeyEvent>
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include "Game.h"

int randint2(int a, int b) {
	return qrand() % (b - a + 1) + a;
}

void StoneManager::keyPressEvent(QKeyEvent *event)
{
	std::cout << "key pressed" << std::endl;
}

void StoneManager::removeStone(unsigned int stoneID)
{
	if (stoneID < stonePointers.size()) {
		stonePointers[stoneID] = NULL;
	}
	if (stoneID < stoneLayer.size()) {
		stoneLayer[stoneID] = -1;
	}
	return;
}

void StoneManager::popFront(unsigned int stoneID)
{
	stonePointers[stoneID]->setZValue(LAYER_STONE_FRONT);
	stoneLayer[stoneID] = LAYER_STONE_FRONT;
	for (int i = 0; i < stoneLayer.size(); i++) {
		if (stoneLayer[i] < LAYER_STONE_BACK && stoneLayer[i]>0) {
			stoneLayer[i]++;
			stonePointers[i]->setZValue(stoneLayer[i]);
		}
	}
	return;
}

void StoneManager::levelUp()
{
	level++;
	for (int i = 0; i < stonePointers.size(); i++) {
		stonePointers[i]->speedUp();
	}
}

void StoneManager::stop()
{
	timer->stop();
	for (int i = 0; i < stonePointers.size(); i++) {
		if (stonePointers[i] != NULL) {
			stonePointers[i]->stop();
		}
	}
}

void StoneManager::start()
{
	timer->start(1000);
}

void StoneManager::spawn() {
	//scene()->setSceneRect(0, 200, 512, 400);
	int stoneIndex = randint2(0, stoneTypes - 1);
	QImage* randStoneImg = stoneImages[stoneIndex];
	int imgWidth = randStoneImg->width();
	int imgHeight = randStoneImg->height();

	//random position on shoot line
	QPointF shootPos;
	shootPos.setX(randint2(originPos.x() + 100, originPos.x() + 300/*-image.width*/));
	shootPos.setY(linePos);

	unsigned int sid = genStoneID();
	Stone *target = new Stone(randStoneImg, birthPos, shootPos, sid, playerID, this);
	target->setStoneType(stoneIndex);
	stonePointers[sid] = target;

	int layer = genLayer();		//alloc layer num
	stoneLayer[sid] = layer;
	scene()->addItem(target);
	target->setZValue(layer);

	std::cout << "birth zvalue: 0x" << std::hex << (int)target->zValue() << std::endl;

	std::cout << target->x() << "  " << target->y() << std::endl;
}

unsigned int StoneManager::genStoneID()
{
	int i = 0;
	for (; i < stonePointers.size(); i++) {
		if (stonePointers[i] == NULL) {
			return i;
		}
	}
	stonePointers.push_back(NULL);
	stoneLayer.push_back(-1);
	return i;
}

int StoneManager::genLayer()
{
	int min = LAYER_BG;
	for (int i = 0; i < stoneLayer.size(); i++) {
		if (stoneLayer[i] < LAYER_HOLE) {	//ignore under hole layers
			continue;
		}
		if (stoneLayer[i] < min) {
			min = stoneLayer[i];
		}
	}
	return min - 1;		//new lowest layer
}

void StoneManager::addScore(int score)
{
	parentManager->hitOne(playerID, score);
	scoreGap += score;
	if (scoreGap >= levelThreshold) {
		scoreGap = 0;
		currentSpeed += speedStep;
		for (int i = 0; i < stonePointers.size(); i++) {
			if (stonePointers[i] != NULL) {
				stonePointers[i]->speedUp();
			}
		}
	}
}

void StoneManager::missOne()
{
	parentManager->missOne(playerID, 5);
}

int StoneManager::getWaterHeight()
{
	return parentManager->getCurrentY(playerID);
}

int StoneManager::getCurrentScore()
{
	return parentManager->getCurrentScore(playerID);
}

double StoneManager::getCurrentSpeed()
{
	return currentSpeed;
}

StoneManager::StoneManager(QPointF position, GameProtocol playerID, GameManager* parentManager) {
	this->parentManager = parentManager;
	level = 0;
	originPos = position;

	//load stone images
	for (int i = 0; i < stoneTypes; i++) {
		std::stringstream ss;
		if (i < 9) {
			ss << '0';
		}
		ss << (i + 1);
		std::string stoneName = STONE_PREFIX + ss.str() + STONE_SUFFIX;
		QImage* tempStoneImg = new QImage();
		tempStoneImg->load(stoneName.c_str());
		stoneImages.push_back(tempStoneImg);
	}
	

	//set key points
	birthPos.setX(position.x() + 256/*width/2*/);
	birthPos.setY(position.y() + 170/*offset*/);
	linePos = shootLineHeight;	//shoot line
	shootLineY = shootLineHeight;

	this->playerID = playerID;

	timer = new QTimer(this);
	QObject::connect(timer, SIGNAL(timeout()), this, SLOT(spawn()));
	timer->start(1000);
}

StoneManager::~StoneManager()
{
	for (int i = 0; i < stonePointers.size(); i++) {
		if (stonePointers[i] != NULL) {
			delete stonePointers[i];
		}
	}
	timer->stop();
}
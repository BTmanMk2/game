#include "BgManager.h"

BgManager::BgManager(const char * bgfile, const char * holefile, QGraphicsScene* scene,
	int x, int y, GameProtocol bglayer, GameProtocol holelayer,
	QGraphicsItem * parent)
	:bg(QPixmap(bgfile)), hole(QPixmap(holefile)), QGraphicsItem(parent),
	rx(x), ry(y)
{
	bg.setPos(x, y);
	bg.setZValue(bglayer);
	bg.setShapeMode(QGraphicsPixmapItem::MaskShape);
	hole.setPos(x + 174, y + 26);
	hole.setZValue(holelayer);

	scene->addItem(&bg);
	scene->addItem(&hole);
}

void BgManager::holePos(int x, int y)
{
	hole.setPos(x + rx, y + ry);
}

QRectF BgManager::boundingRect() const
{
	return QRectF(0, 0, 512, 768);
}

void BgManager::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
}
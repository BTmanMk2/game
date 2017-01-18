#include "MySVG.h"

SVGData::SVGData(const char * filename, QVector<SVGData::SVGTriangle>& tr, QVector<SVGFill>& fill, double t)
	:tris(tr), fills(fill), maxX(0), maxY(0), minX(0), minY(0), turb(t)
{
	// read the content of the file 
	QFile svgfile(filename);
	if (!svgfile.open(QIODevice::ReadOnly))
	{
		error = INIT_FAILURE;
		return;
	}
	if (!raw.setContent(&svgfile))
	{
		svgfile.close();
		error = INIT_FAILURE;
		return;
	}
	svgfile.close();

	// do the process progress
	if (!processData())
	{
		error = INIT_FAILURE;
	}

}

void SVGData::test()
{
	
}

int SVGData::processData()
{
	// get the root of the document
	QDomElement svg = raw.documentElement();
	QDomElement child = svg.firstChildElement();
	if (!child.isNull() && child.tagName() == "g") child = child.firstChildElement();
	while (!child.isNull())
	{
		qDebug("yes, we are here1!:%s", child.tagName().toStdString().c_str());
		if (child.tagName() == "polygon") // regard as triangle
		{
			triscounter++;
			qDebug("yes, we are here2!");
			getTriangle(child);
		}
		else if (child.tagName() == "linearGradient") // regard as fill
		{
			getFillLineGradient(child);
		}

		// goto the next
		child = child.nextSiblingElement();
	}

	// return to the (0, 0)
	unsigned int amount = tris.size();
	for (unsigned int i = 0; i < amount; i++)
	{
		unsigned int v_amount = tris[i].triangle.size();
		for (unsigned int j = 0; j < v_amount; j++)
		{
			tris[i].triangle[j].setX(tris[i].triangle[j].x() - minX);
			tris[i].triangle[j].setY(tris[i].triangle[j].y() - minY);
		}
	}
	maxX -= minX; maxY -= minY;
	minX = 0; minY = 0;
	qDebug("childcontent:%d", tris.size());
	qDebug("tris found:%d  fills found: %d", triscounter, findcounter);
	return 0;
}

int SVGData::getTriangle(QDomElement& e)
{
	SVGTriangle result;
	
	// get the fill pattern
	QDomAttr qda = e.attributeNode("fill");
	std::string fill = qda.value().toStdString();
	qDebug("Tri fill id:%s", fill.c_str());
	if (fill.substr(0, 3) == "url")
	{
		std::string data = fill.substr(5);
		data.pop_back();
		if (refs.contains(data))
		{
			findcounter++;
			result.fillIndex = refs[data];
		}
	}

	// get the opacity
	qda = e.attributeNode("opacity");
	if (!qda.isNull())
	{
		ss.clear();
		ss.str(qda.value().toStdString());
		ss >> result.opacity;
		qDebug("Opacity: %f", result.opacity);
	}

	// get the vertex info
	ss.clear();
	ss.str(e.attributeNode("points").value().toStdString());
	double xreader, yreader;
	char ch_reader;
	if (firstIn && ss.good())
	{
		ss >> xreader >> ch_reader >> yreader;
		maxX = xreader; minX = xreader;
		maxY = yreader; minY = yreader;
		firstIn = false;
	}
	while (ss.good())
	{
		ss >> xreader >> ch_reader >> yreader;
		if (xreader > maxX) maxX = xreader;
		else if (xreader < minX) minX = xreader;
		if (yreader > maxY) maxY = yreader;
		else if (yreader < minY) minY = yreader;
		result.triangle.push_back(QPointF(xreader, yreader));
		result.turbTar.push_back(QPair<qreal, bool>(0, true));
		result.turbArc.push_back(QPair<qreal, bool>(0, true));
	}

	// write back
	tris.push_back(result);

	return 0;
}

int SVGData::getFillLineGradient(QDomElement& e)
{
	QGradientStops qss;
	// get the id
	QDomAttr qda = e.attributeNode("id");
	std::string id = qda.value().toStdString();
	qDebug("%s", id.c_str());

	// get the gradient matrix
	QTransform qts;
	double m11, m12, m13 = 0, m21, m22, m23 = 0, m31, m32, m33 = 1;
	qda = e.attributeNode("gradientTransform");
	std::string tmp = qda.value().toStdString();
	if (tmp.substr(0, 6) == "matrix")
	{
		tmp = tmp.substr(7);
		tmp.pop_back();
		ss.clear();
		ss.str(tmp);
		ss >> m11 >> m12 >> m21 >> m22 >> m31 >> m32;
		// qDebug("%f, %f, %f\n%f, %f, %f\n%f, %f, %f", m11, m12, m13, m21, m22, m23, m31, m32, m33);
		qts.setMatrix(m11, m12, m13, m21, m22, m23, m31, m32, m33);
	}

	// get the start points and the stop points
	double x1, y1, x2, y2;
	qda = e.attributeNode("x1");
	ss.clear();
	ss.str(qda.value().toStdString());
	ss >> x1;
	qda = e.attributeNode("y1");
	ss.clear();
	ss.str(qda.value().toStdString());
	ss >> y1;
	qda = e.attributeNode("x2");
	ss.clear();
	ss.str(qda.value().toStdString());
	ss >> x2;
	qda = e.attributeNode("y2");
	ss.clear();
	ss.str(qda.value().toStdString());
	ss >> y2;
	// qDebug("%f %f %f %f", x1, y1, x2, y2);
	QLinearGradient result(x1, y1, x2, y2);

	// get the stops
	QDomElement child = e.firstChildElement();
	double offset;
	unsigned int color[3];
	while (!child.isNull())
	{
		qda = child.attributeNode("offset");
		ss.clear();
		ss.str(qda.value().toStdString());
		ss >> offset;
		// qDebug("%s", qda.value().toStdString().c_str());
		qda = child.attributeNode("style");
		tmp = qda.value().toStdString();
		tmp = tmp.substr(12);
		qDebug("%s", tmp.c_str());
		ss.clear();
		ss.str("");
		ss << "0x" << tmp.substr(0, 2) << ' '
			<< "0x" << tmp.substr(2, 2) << ' '
			<< "0x" << tmp.substr(4, 2);
		// qDebug("%s", ss.str().c_str());
		ss >> std:: hex >> color[0] >> color[1] >> color[2] >> std::dec;
		// qDebug("%x %x %x", color[0], color[2], color[2]);
		qss.push_back(QGradientStop(offset, QColor(color[0], color[1], color[2])));
		child = child.nextSiblingElement();
	}

	// store and build up the map
	result.setStops(qss);
	result.setSpread(QGradient::PadSpread);
	QBrush brush(result);
	brush.setTransform(qts);
	fills.push_back(brush);
	refs[id] = fills.size() - 1;
	return 0;
}

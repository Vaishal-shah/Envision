/***********************************************************************************************************************
**
** Copyright (c) 2011, ETH Zurich
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
** following conditions are met:
**
**    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following
**      disclaimer.
**    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
**      following disclaimer in the documentation and/or other materials provided with the distribution.
**    * Neither the name of the ETH Zurich nor the names of its contributors may be used to endorse or promote products
**      derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
***********************************************************************************************************************/

/***********************************************************************************************************************
 * ControlFlowItem.cpp
 *
 *  Created on: Feb 24, 2011
 *      Author: Dimitar Asenov
 **********************************************************************************************************************/

#include "items/ControlFlowItem.h"
#include "items/VMethodCF.h"

namespace ControlFlowVisualization {

ITEM_COMMON_DEFINITIONS(ControlFlowItem, "item")

ControlFlowItem::ControlFlowItem(Item* parent,  const StyleType* style) :
	Item(parent, style),
	preferredContinueExit_(EXIT_RIGHT),
	preferredBreakExit_(EXIT_LEFT)
{
}

bool ControlFlowItem::showAsControlFlow() const
{
	auto item = parent();
	while (item)
	{
		VMethodCF* met = dynamic_cast<VMethodCF*> (item);
		if (met) return true;

		item = item->parent();
	}

	return false;
}

QPainterPath ControlFlowItem::connector(const QList< QPoint >& points, bool arrowEnding)
{
	QPainterPath path;

	// Draw Linear path
	if (!points.isEmpty() > 0) path.moveTo(points.at(0));
	for(int i = 1; i<points.size(); ++i) path.lineTo(points.at(i));

	// Draw arrow if specified
	if (arrowEnding && points.size() >= 2)
	{
		QPoint last = points.last();
		QPoint prev = points.at( points.length()-2 );

		if (last.y() == prev.y())
		{
			// Horizontal arrow
			if (last.x() < prev.x())
			{
				// Left arrow
				path.lineTo(last.x() + style()->pinLength()/2, last.y() - style()->pinLength()/4);
				path.lineTo(last);
				path.lineTo(last.x() + style()->pinLength()/2, last.y() + style()->pinLength()/4);
				path.lineTo(last);
			}
			else
			{
				// Right arrow
				path.lineTo(last.x() - style()->pinLength()/2, last.y() - style()->pinLength()/4);
				path.lineTo(last);
				path.lineTo(last.x() - style()->pinLength()/2, last.y() + style()->pinLength()/4);
				path.lineTo(last);
			}
		}
		else
		{
			// Vertical arrow
			if (last.y() < prev.y())
			{
				// Up arrow
				path.lineTo(last.x() - style()->pinLength()/4, last.y() + style()->pinLength()/2);
				path.lineTo(last);
				path.lineTo(last.x() + style()->pinLength()/4, last.y() + style()->pinLength()/2);
				path.lineTo(last);
			}
			else
			{
				// Down arrow
				path.lineTo(last.x() - style()->pinLength()/4, last.y() - style()->pinLength()/2);
				path.lineTo(last);
				path.lineTo(last.x() + style()->pinLength()/4, last.y() - style()->pinLength()/2);
				path.lineTo(last);
			}

		}
	}

	return path;
}

void ControlFlowItem::addConnector(QList< QPoint >& points, bool arrowEnding)
{
	connectors_.append(points);
	arrowEndings_.append(arrowEnding);
	setUpdateNeeded(StandardUpdate);
}

void ControlFlowItem::addConnector(int xBegin, int yBegin, int xEnd, int yEnd, bool arrowEnding)
{
	addConnector(QPoint(xBegin, yBegin),QPoint(xEnd, yEnd), arrowEnding);
}

void ControlFlowItem::addConnector(const QPoint& begin, const QPoint& end, bool arrowEnding)
{
	QList< QPoint > points;
	points.append(begin);
	points.append(end);
	addConnector(points, arrowEnding);
}

void ControlFlowItem::addToLastConnector(int x, int y)
{
	connectors_.last().append(QPoint(x,y));
}

void ControlFlowItem::addToLastConnector(const QPoint& point)
{
	connectors_.last().append(point);
}

void ControlFlowItem::clearConnectors()
{
	connectors_.clear();
	arrowEndings_.clear();
	setUpdateNeeded(StandardUpdate);
}

void ControlFlowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Item::paint(painter, option, widget);

	int xOffset = 0;
	int yOffset = 0;
	if (hasShape())
	{
		xOffset = getShape()->contentLeft();
		yOffset = getShape()->contentTop();
	}

	if (showAsControlFlow())
	{
		painter->setPen( style()->pin());
		painter->translate( xOffset + style()->pin().width()/2.0, yOffset + style()->pin().width()/2.0);

		for(int i = 0; i < connectors_.size(); ++i)
		{
			painter->drawPath( connector(connectors_.at(i), arrowEndings_.at(i)) );
		}
	}
}

}

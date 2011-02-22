/***********************************************************************************************************************
 * VNewExpression.cpp
 *
 *  Created on: Feb 14, 2011
 *      Author: Dimitar Asenov
 **********************************************************************************************************************/

#include "expressions/VNewExpression.h"

#include "VisualizationBase/headers/items/Symbol.h"

using namespace Visualization;
using namespace OOModel;

namespace OOVisualization {

ITEM_COMMON_DEFINITIONS(VNewExpression)

VNewExpression::VNewExpression(Item* parent, NodeType* node, const StyleType* style) :
	ItemWithNode<LayoutProvider<>, NewExpression>(parent, node, style),
	newSymbol_( new Symbol(NULL, &style->newSymbol()) ),
	type_(NULL),
	amountSymbol_( NULL ),
	amount_(NULL)
{
	layout()->append(newSymbol_);
}

VNewExpression::~VNewExpression()
{
	// These were automatically deleted by LayoutProvider's destructor
	newSymbol_ = NULL;
	amountSymbol_ = NULL;
	type_ = NULL;
	amount_ = NULL;
}

void VNewExpression::determineChildren()
{
	layout()->synchronizeMid(type_, node()->type(), 1);
	layout()->synchronizeMid(amountSymbol_, node()->amount() != NULL, &style()->amountSymbol(), 2);
	layout()->synchronizeLast(amount_, node()->amount());

	// TODO: find a better way and place to determine the style of children. Is doing this causing too many updates?
	// TODO: consider the performance of this. Possibly introduce a style updated boolean for all items so that they know
	//			what's the reason they are being updated.
	// The style needs to be updated every time since if our own style changes, so will that of the children.
	layout()->setStyle( &style()->layout());
	newSymbol_->setStyle( &style()->newSymbol());
	if (amountSymbol_) amountSymbol_->setStyle( &style()->amountSymbol());
}

}

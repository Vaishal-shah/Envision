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
 * VIntegerLiteral.cpp
 *
 *  Created on: Feb 11, 2011
 *      Author: Dimitar Asenov
 **********************************************************************************************************************/

#include "literals/VIntegerLiteral.h"

#include "ModelBase/src/Model.h"

using namespace OOModel;
using namespace Visualization;

namespace OOVisualization {

ITEM_COMMON_DEFINITIONS(VIntegerLiteral, "item")

VIntegerLiteral::VIntegerLiteral(Item* parent, NodeType *literal, const StyleType *style) :
	ItemWithNode<Item, IntegerLiteral>(parent, literal, style),
	vis_(nullptr)
{
}

VIntegerLiteral::~VIntegerLiteral()
{
	SAFE_DELETE_ITEM(vis_);
}

void VIntegerLiteral::determineChildren()
{
	synchronizeItem(vis_, node()->valueNode(), style());
	vis_->setEditable(false);
	vis_->setStyle( style() );
}

void VIntegerLiteral::updateGeometry(int availableWidth, int availableHeight)
{
	Item::updateGeometry(vis_, availableWidth, availableHeight);
}

}
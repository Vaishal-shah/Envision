/***********************************************************************************************************************
 **
 ** Copyright (c) 2011, 2012 ETH Zurich
 ** All rights reserved.
 **
 ** Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 ** following conditions are met:
 **
 **    * Redistributions of source code must retain the above copyright notice, this list of conditions and the
 **      following disclaimer.
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
 **********************************************************************************************************************/

/*
 * VControlFlowMethodSwitch.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: Dimitar Asenov
 */

#include "items/VControlFlowMethodSwitch.h"
#include "items/VMethodCF.h"

#include "OOVisualization/headers/top_level/VMethod.h"


namespace ControlFlowVisualization {

ITEM_COMMON_DEFINITIONS(VControlFlowMethodSwitch, "item")

VControlFlowMethodSwitch::VControlFlowMethodSwitch(Item* parent, NodeType* node, const StyleType* style)
	:ItemWithNode<Visualization::Item, OOModel::Method>(parent, node, style), showAsControlFlow_(false),
	 metCF_(nullptr), met_(nullptr)
{
}

bool VControlFlowMethodSwitch::sizeDependsOnParent() const
{
	if (metCF_) return metCF_->sizeDependsOnParent();
	else  return met_->sizeDependsOnParent();
}

void VControlFlowMethodSwitch::determineChildren()
{
	if (isRenderingChanged())
	{
		SAFE_DELETE(met_);
		SAFE_DELETE(metCF_);
	}

	if (isShownAsControlFlow()) synchronizeItem<VMethodCF>(metCF_, node(), nullptr);
	else synchronizeItem<OOVisualization::VMethod>(met_, node(), nullptr);
}

void VControlFlowMethodSwitch::updateGeometry(int availableWidth, int availableHeight)
{
	if (metCF_) Item::updateGeometry(metCF_, availableWidth, availableHeight);
	else Item::updateGeometry(met_, availableWidth, availableHeight);
}

bool VControlFlowMethodSwitch::isRenderingChanged() const
{
	return (met_ && isShownAsControlFlow()) || (metCF_ && !isShownAsControlFlow());
}

} /* namespace ControlFlowVisualization */
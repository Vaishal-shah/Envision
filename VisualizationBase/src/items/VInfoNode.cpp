/***********************************************************************************************************************
 **
 ** Copyright (c) 2015 ETH Zurich
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
#include "VInfoNode.h"

#include "declarative/DeclarativeItemDef.h"
#include "nodes/InfoNode.h"
#include "Comments/src/items/VCommentBrowser.h"
#include "utils/InfoJavascriptFunctions.h"
#include "QtWebKitWidgets/QGraphicsWebView"
#include "QtWebKitWidgets/QWebFrame"

namespace Visualization {

ITEM_COMMON_DEFINITIONS(VInfoNode, "item")

VInfoNode::VInfoNode(Item* parent, NodeType* node, const StyleType* style) :
		Super(parent, node, style)
{
	node->fullUpdate();
	browser_ = new Comments::VCommentBrowser(this, QString());
	browser_->setHeightResizesWithContent(true);
}

void VInfoNode::initializeForms()
{
	addForm(item(&I::browser_));
}

void VInfoNode::determineChildren()
{
	Super::determineChildren();
	if (node()->hasContentChanged())
	{
		browser_->setContent(node()->infoHtml());
		browser_->browser()->page()->mainFrame()->addToJavaScriptWindowObject("operations",
								InfoJavascriptFunctions::instance());
	}
}

}

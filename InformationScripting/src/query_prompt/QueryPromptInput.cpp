/***********************************************************************************************************************
**
** Copyright (c) 2011, 2015 ETH Zurich
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
#include "QueryPromptInput.h"

#include "../nodes/QueryNodeContainer.h"
#include "../nodes/EmptyQueryNode.h"

#include "VisualizationBase/src/declarative/DeclarativeItemDef.h"
#include "VisualizationBase/src/cursor/TextCursor.h"
#include "VisualizationBase/src/VisualizationManager.h"

namespace InformationScripting {

ITEM_COMMON_DEFINITIONS(QueryPromptInput, "item")

QueryPromptInput::QueryPromptInput(Item* parent, const StyleType* style)
	: Super{parent, style}, query_ {new QueryNodeContainer()}
{
	query_->setQuery(new EmptyQueryNode());
}

void QueryPromptInput::initializeForms()
{
	addForm(item(&I::queryVis_, [](I* v) {return v->query_;}));
}

void QueryPromptInput::setSelection(Interaction::PromptMode::InputSelection)
{
	queryVis_->moveCursor();
}

}

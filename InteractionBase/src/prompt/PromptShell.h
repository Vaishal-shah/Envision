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

#pragma once

#include "../interactionbase_api.h"
#include "PromptShellStyle.h"
#include "Prompt.h"

#include "VisualizationBase/src/declarative/DeclarativeItem.h"

namespace Visualization {
	class Static;
	class StaticStyle;
}

namespace Interaction {

class INTERACTIONBASE_API PromptShell  : public Super<Visualization::DeclarativeItem<PromptShell>>
{
	ITEM_COMMON(PromptShell)

	public:
		PromptShell(const QString& initialCommandText,
								 Prompt::PromptOptions options = Prompt::None,
								 const StyleType* style = itemStyles().get());

		static void initializeForms();

		void setErrors(QList<Item*> errors);

	protected:
		virtual void determineChildren() override;
		virtual void changeGeometry(int availableWidth = 0, int availableHeight = 0) override;

	private:
		Item* inputItem_{};
		Visualization::Static* modeIcon_{};
		Visualization::StaticStyle* modeIconStyle_{};
		QList<Item*> errors_;
		Prompt::PromptOptions promptOptions_{};
		bool initialUpdate_{true};

		void setShellPosition();
		void centerViewOnShell() const;
};

}

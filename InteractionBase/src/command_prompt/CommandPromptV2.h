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

#include "interactionbase_api.h"
#include "VisualizationBase/src/cursor/Cursor.h"

namespace Interaction {

class CommandPromptShell;

class INTERACTIONBASE_API CommandPromptV2
{
	public:

		enum PromptOption {
			None = 0,
			CenterViewOnPrompt = 0x00000001 /**< If set, the view will be scrolled so that the prompt is in the center. */
		};
		Q_DECLARE_FLAGS(PromptOptions, PromptOption)

		static void show(Visualization::Item* commandReceiver, QString initialCommandText = {},
							  PromptOptions options = None);
		static void hide();

	private:
		friend class CommandPromptShell;

		static CommandPromptShell* shell_;

		static Visualization::Item* commandReceiver_;
		static std::unique_ptr<Visualization::Cursor> commandReceiverCursor_;

		static Visualization::Item* commandReceiver();
		static QPoint commandReceiverCursorPosition();
};
Q_DECLARE_OPERATORS_FOR_FLAGS(CommandPromptV2::PromptOptions)

inline Visualization::Item* CommandPromptV2::commandReceiver() { return commandReceiver_; }
inline QPoint CommandPromptV2::commandReceiverCursorPosition()
	{ return commandReceiverCursor_ ? commandReceiverCursor_->position() : QPoint(0, 0); }

}

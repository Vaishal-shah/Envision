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
 * AutoCompleteEntry.h
 *
 *  Created on: Jul 23, 2012
 *      Author: Dimitar Asenov
 */

#ifndef InteractionBase_AUTOCOMPLETEENTRY_H_
#define InteractionBase_AUTOCOMPLETEENTRY_H_

#include "../interactionbase_api.h"

namespace Visualization {
	class Item;
}

namespace Interaction {

class INTERACTIONBASE_API AutoCompleteEntry
{
	public:
		typedef std::function<void (AutoCompleteEntry* entry)> ExecuteFunction;

		AutoCompleteEntry(const QString& text = QString(), const QString& description = QString(),
				Visualization::Item* visualization = nullptr, ExecuteFunction execFunction = nullptr);
		virtual ~AutoCompleteEntry();

		virtual void execute();

		const QString& text();
		const QString& description();
		Visualization::Item* visualization();

		void setText(const QString& suggestion);
		void setDescription(const QString& description);
		void setVisualization(Visualization::Item* item);
		void setExecutionFunction(ExecuteFunction execFunction);

	private:
		QString text_;
		QString description_;
		Visualization::Item* vis_;
		ExecuteFunction execFunction_;

};

inline const QString& AutoCompleteEntry::text() { return text_; }
inline const QString& AutoCompleteEntry::description() { return description_; }
inline Visualization::Item* AutoCompleteEntry::visualization() { return vis_; }
inline void AutoCompleteEntry::setText(const QString& text) { text_ = text; }
inline void AutoCompleteEntry::setDescription(const QString& description) { description_ = description; }

} /* namespace Interaction */
#endif /* InteractionBase_AUTOCOMPLETEENTRY_H_ */
/***********************************************************************************************************************
 **
 ** Copyright (c) 2011, 2014 ETH Zurich
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

#include "CommentsPlugin.h"
#include "handlers/HComment.h"
#include "handlers/HCommentDiagram.h"
#include "handlers/HCommentDiagramShape.h"
#include "handlers/HCommentDiagramConnector.h"
#include "handlers/HCommentImage.h"
#include "handlers/HCommentBrowser.h"
#include "handlers/HCommentFreeNode.h"
#include "handlers/HCommentText.h"
#include "items/VComment.h"
#include "items/VCommentDiagram.h"
#include "items/VCommentDiagramShape.h"
#include "items/VCommentDiagramConnector.h"
#include "items/VCommentImage.h"
#include "items/VCommentBrowser.h"
#include "items/VCommentFreeNode.h"
#include "items/VCommentText.h"

#include "InteractionBase/src/handlers/GenericHandler.h"
#include "SelfTest/src/TestManager.h"

namespace Comments {

bool CommentsPlugin::initialize(Core::EnvisionManager&)
{
	Core::TypeRegistry::initializeNewTypes();

	VComment::setDefaultClassHandler(HComment::instance());
	VCommentDiagram::setDefaultClassHandler(HCommentDiagram::instance());
	VCommentDiagramShape::setDefaultClassHandler(HCommentDiagramShape::instance());
	VCommentDiagramConnector::setDefaultClassHandler(HCommentDiagramConnector::instance());
	VCommentImage::setDefaultClassHandler(HCommentImage::instance());
	VCommentBrowser::setDefaultClassHandler(HCommentBrowser::instance());
	VCommentText::setDefaultClassHandler(HCommentText::instance());
	VCommentFreeNode::setDefaultClassHandler(HCommentFreeNode::instance());

	Interaction::GenericHandler::setCommentCreationFunction([]()
		{return new Comments::CommentNode{"Enter comment here"};});

	return true;
}

void CommentsPlugin::unload()
{
}

void CommentsPlugin::selfTest(QString testid)
{
	if (testid.isEmpty()) SelfTest::TestManager<CommentsPlugin>::runAllTests().printResultStatistics();
	else SelfTest::TestManager<CommentsPlugin>::runTest(testid).printResultStatistics();
}

}

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

#include "VersionControlQuery.h"

#include "../query_framework/QueryRegistry.h"

#include "ModelBase/src/nodes/Node.h"
#include "ModelBase/src/model/TreeManager.h"

#include "FilePersistence/src/simple/SimpleTextFileStore.h"
#include "FilePersistence/src/version_control/GitRepository.h"
#include "FilePersistence/src/version_control/History.h"

#include "ModelBase/src/nodes/Node.h"
#include "OOModel/src/elements/StatementItem.h"
#include "OOModel/src/statements/ExpressionStatement.h"
#include "OOModel/src/expressions/EmptyExpression.h"

using namespace FilePersistence;

namespace InformationScripting {

const QStringList VersionControlQuery::COUNT_ARGUMENT_NAMES{"c", "count"};
const QStringList VersionControlQuery::NODE_TYPE_ARGUMENT_NAMES{"t", "type"};
const QStringList VersionControlQuery::NODES_ARGUMENTS_NAMES{"nodes"};
const QStringList VersionControlQuery::IN_ARGUMENT_NAMES{"in"};
const QStringList VersionControlQuery::INCLUDE_INTERMEDIATE_VERSIONS_ARGUMENT_NAMES{"intermediate"};
const QStringList VersionControlQuery::TYPED_CHANGES_ARGUMENT_NAMES{"tc", "typed_changes"};

Optional<TupleSet> VersionControlQuery::executeLinear(TupleSet input)
{
	Model::TreeManager* treeManager = target()->manager();

	// get GitRepository
	QString path{"projects/" + treeManager->name()};
	if (!GitRepository::repositoryExists(path)) return {"No repository found"};

	GitRepository repository{path};
	TupleSet result;

	auto commitIdRange = repository.revisions();
	// TODO: This only works for linear histories
	auto adaptedRangeOptional = commitsToConsider(commitIdRange);
	if (!adaptedRangeOptional)
		return adaptedRangeOptional.errors()[0];
	commitIdRange = adaptedRangeOptional.value();

	// if argument is not set, make sure that commitIdRange only contains two commitIds
	if (!arguments_.isArgumentSet(INCLUDE_INTERMEDIATE_VERSIONS_ARGUMENT_NAMES[0]) && commitIdRange.size() > 2)
	{
		commitIdRange = QStringList{commitIdRange.first(), commitIdRange.last()};
	}

	QList<Model::Node*> nodesToLookAt;

	bool considerLocalCommitsOnly = arguments_.isArgumentSet(ArgumentParser::LOCAL_SCOPE_ARGUMENT_NAMES[0]) &&
			arguments_.scope(this) == ArgumentParser::Scope::Local;
	if (considerLocalCommitsOnly)
		nodesToLookAt << target();
	else if (arguments_.scope(this) == ArgumentParser::Scope::Input)
		for (const auto& astTuple : input.tuples("ast"))
			nodesToLookAt << static_cast<Model::Node*>(astTuple["ast"]);

	bool outputNodesOnly = arguments_.isArgumentSet(NODES_ARGUMENTS_NAMES[0]);

	auto typeMatcher = Model::SymbolMatcher::guessMatcher(arguments_.argument(NODE_TYPE_ARGUMENT_NAMES[1]));

	for (int i = commitIdRange.size() - 1; i > 0; --i)
	{
		QString oldCommitId = commitIdRange[i];
		QString newCommitId = commitIdRange[i - 1];

		Diff diff = repository.diff(oldCommitId, newCommitId);
		auto changes = diff.changes();

		if (!outputNodesOnly)
			addCommitMetaInformation(result, repository.getCommitInformation(newCommitId));

		for (auto change : changes.values())
		{
			// We ignore structure only changes since a node with a structure only change,
			// has a child which was deleted or added or modified, and this child will appear in the final representation.
			if (change->isFake() || change->onlyStructureChange()) continue;

			auto id = change->nodeId();
			if (auto node = const_cast<Model::Node*>(treeManager->nodeIdMap().node(id)))
			{
				Model::Node* changedNode = nullptr;
				if (auto statement = node->firstAncestorOfType(typeMatcher))
					changedNode = statement;
				else // The node is hopefully higher up in the node hierarchy thus we take it as is.
					changedNode = node;

				// Ignore empty nodes
				if (auto changedExpr = DCast<OOModel::ExpressionStatement>(changedNode))
					if (DCast<OOModel::EmptyExpression>(changedExpr->expression()))
						continue;

				auto ancestorIt = std::find_if(nodesToLookAt.begin(), nodesToLookAt.end(), [changedNode](Model::Node* n) {
						return n == changedNode || n->isAncestorOf(changedNode);
				});
				if (nodesToLookAt.isEmpty() || ancestorIt != nodesToLookAt.end())
				{
					// If we have input or local node(s) only return the information of the input nodes and not the children:
					if (!nodesToLookAt.isEmpty())
						changedNode = *ancestorIt;

					if (outputNodesOnly)
						result.add({{"ast", changedNode}});

					else
						result.add({"change", {{"id", newCommitId}, {"ast", changedNode}}});

					if (arguments_.isArgumentSet(TYPED_CHANGES_ARGUMENT_NAMES[0]))
					{
						QString color;
						QString change_type;
						switch (change->type())
						{
							case ChangeType::Deletion:
								color = "red";
								change_type = "Delete";
								break;
							case ChangeType::Insertion:
								color = "blue";
								change_type = "Insertion";
								break;
							case ChangeType::Move:
								color = "yellow";
								change_type = "Move";
								break;
							case ChangeType::Stationary:
								color = "black";
								change_type = "Stationary";
								break;
							case ChangeType::Unclassified:
								color = "white";
								change_type = "Unclassified";
								break;
							default:
								// TODO throw exception?
								color = "";
								change_type = "unhandled_type";
						}
						result.add({"color", {{"ast", changedNode}, {"color", color}}});
						result.add({"change_typed", {{"ast", changedNode}, {"type", change_type}}});
					}
				}
			}
		}
	}

	return result;
}

void VersionControlQuery::registerDefaultQueries()
{
	QueryRegistry::registerQuery<VersionControlQuery>("changes",
		std::vector<ArgumentRule>{{ArgumentRule::AtMostOneOf, {{COUNT_ARGUMENT_NAMES[1], ArgumentValue::IsSet},
												{IN_ARGUMENT_NAMES[0], ArgumentValue::NotEmpty}}},
										  {ArgumentRule::AtMostOneOf, {{NODES_ARGUMENTS_NAMES[0], ArgumentValue::IsSet},
												{TYPED_CHANGES_ARGUMENT_NAMES[0], ArgumentValue::IsSet}}}});
}

VersionControlQuery::VersionControlQuery(Model::Node* target, QStringList args, std::vector<ArgumentRule> argumentRules)
	: LinearQuery{target}, arguments_{{
		{COUNT_ARGUMENT_NAMES, "The amount of revisions to look at", COUNT_ARGUMENT_NAMES[1], "10"},
		{NODE_TYPE_ARGUMENT_NAMES, "The minimum type of the nodes returned", NODE_TYPE_ARGUMENT_NAMES[1], "StatementItem"},
		QCommandLineOption{NODES_ARGUMENTS_NAMES},
		QCommandLineOption{INCLUDE_INTERMEDIATE_VERSIONS_ARGUMENT_NAMES, "By default, only the first and last commit "
			" are used to compute changes. Setting this flag will change the behavior to also use intermediate commits."},
		QCommandLineOption{TYPED_CHANGES_ARGUMENT_NAMES, 	"Add the type (Delete, Insertion, Move, Stationary, "
																			"Unclassified) of the changes to the result and define a "
																			"color for the nodes according to the type"}
}, {PositionalArgument{IN_ARGUMENT_NAMES[0], "Specific commits to look at, either a single one or a range with .."}},
args, true}
{
	for (const auto& rule : argumentRules)
		rule.check(arguments_);
}

void VersionControlQuery::addCommitMetaInformation(TupleSet& ts, const CommitMetaData& metadata)
{
	ts.add({"commit", {{"id", metadata.sha1_},
			  {"message", metadata.message_},
			  {"date", metadata.dateTime_.toString("dd.MM.yyyy hh:mm")},
			  {"commiter", metadata.committer_.name_ + " " + metadata.committer_.eMail_},
			  {"author", metadata.author_.name_ + " " + metadata.author_.eMail_}}});
}

Optional<QList<QString>> VersionControlQuery::commitsToConsider(const QStringList& commitIdRange) const
{
	QString userProvidedCommitRange;
	for (int i = 0; i< arguments_.numPositionalArguments(); ++i)
	{
		userProvidedCommitRange = arguments_.positionalArgument(i).trimmed();
		if (!userProvidedCommitRange.isEmpty())
			break;
	}

	if (!userProvidedCommitRange.isEmpty())
	{
		auto commitRange = userProvidedCommitRange.split("..");

		if (!arguments_.isArgumentSet(INCLUDE_INTERMEDIATE_VERSIONS_ARGUMENT_NAMES[0]))
			return commitRange;

		QString startCommit = commitRange[0];
		auto startIt = std::find_if(commitIdRange.begin(), commitIdRange.end(), [startCommit](QString sha1) {
				return sha1.startsWith(startCommit);
		});
		if (startIt == commitIdRange.end())
			return {QString{"%1 is not a commit id that exists in this repository."}.arg(startCommit)};

		QStringList::const_iterator endIt;
		if (commitRange.size() > 1)
		{
			QString endCommit = commitRange[1];
			endIt = std::find_if(commitIdRange.begin(), commitIdRange.end(), [endCommit](QString sha1) {
					return sha1.startsWith(endCommit);
			});
			if (endIt == commitIdRange.end())
				return {QString{"%1 is not a commit id that exists in this repository."}.arg(endCommit)};
		}
		else
		{
			endIt = startIt + 1;
		}
		QStringList result;
		// We always need one more than the actual end since we compare on two commits:
		for (auto it = startIt; it != endIt + 1; ++it)
			result.append(*it);
		return result;
	}
	else
	{
		QString countArgument = arguments_.argument(COUNT_ARGUMENT_NAMES[0]);
		bool converts = false;
		const int CHANGE_COUNT = countArgument.toInt(&converts);
		if (converts)
			return commitIdRange.mid(0, std::min(commitIdRange.size(), CHANGE_COUNT + 1));
		else if (countArgument != "all")
			return {"Invalid count argument"};
		else
			return commitIdRange;
	}
}

}

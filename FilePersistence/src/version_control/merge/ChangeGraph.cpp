/***********************************************************************************************************************
**
** Copyright (c) 2016 ETH Zurich
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
** following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this list of conditions and the following
** disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
** following disclaimer in the documentation and/or other materials provided with the distribution.
** * Neither the name of the ETH Zurich nor the names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
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

#include "ChangeGraph.h"

#include "../Diff.h"
#include "../../simple/GenericTree.h"
#include "../../simple/GenericNode.h"

namespace FilePersistence {

ChangeGraph::~ChangeGraph()
{
	for (auto c : changes_) delete c;
	changes_.clear();
}

void ChangeGraph::init(Diff& diffA, Diff& diffB, GenericTree* baseTree)
{
	QList<MergeChange*> allChanges;
	for (auto it = diffA.changes().begin(); it != diffA.changes().end(); ++it)
		allChanges << MergeChange::changesFromDiffChange(*(it.value()), MergeChange::BranchA);

	for (auto it = diffB.changes().begin(); it != diffB.changes().end(); ++it)
		allChanges <<  MergeChange::changesFromDiffChange(*(it.value()), MergeChange::BranchB);

	insert(allChanges, baseTree);
}

bool ChangeGraph::hasConflicts() const
{
	Q_ASSERT(false);
}

inline void ChangeGraph::insert(QList<MergeChange*> changes, GenericTree* baseTree)
{
	for (auto & change : changes) insertSingleChange(change);

	recomputeAllDependencies(baseTree);
}

void ChangeGraph::insertSingleChange(MergeChange* change)
{
	Q_ASSERT(change);

	if (auto existingChange = findIdenticalChange(change))
	{
		existingChange->addBranches(change->branches());
		delete change;
		return;
	}

	addDirectConflicts(change);

	changesForNode_.insert(change->nodeId(), change);
	if (!change->oldParentId().isNull())
		changesForChildren_.insert(change->oldParentId(), change);
	if (!change->newParentId().isNull() && change->newParentId() != change->oldParentId())
		changesForChildren_.insert(change->newParentId(), change);
	changes_.append(change);
}

MergeChange* ChangeGraph::findIdenticalChange(const MergeChange* change) const
{
	auto it = changesForNode_.find(change->nodeId());
	while (it != changesForNode_.end() && it.key() == change->nodeId())
	{
		if (*it.value() == *change) return it.value();
		++it;
	}

	return nullptr;
}


void ChangeGraph::addDirectConflicts(MergeChange* change)
{
	// Identical changes have been merged, so remaining changes to the same node either
	//  - do not conflict if one change is for the Value/Type and the other is for the Label/NonStationary
	//  - conflict otherwise
	auto it = changesForNode_.find(change->nodeId());
	while (it != changesForNode_.end() && it.key() == change->nodeId())
	{
		if (change->isValueOrTypeChange() == it.value()->isValueOrTypeChange())
		{
			directConflicts_.insert(change, it.value());
			directConflicts_.insert(it.value(), change);
		}
		++it;
	}

	// A change might conflict with another change of the same parent if the labels clash
	// Do not consider deletions and value/type changes
	if (!change->isValueOrTypeChange() && change->type() != ChangeType::Deletion)
	{
		Q_ASSERT(!change->newParentId().isNull());

		it = changesForChildren_.find(change->newParentId());
		while (it != changesForChildren_.end() && it.key() == change->newParentId())
		{
			// If two changes happen to move a node to the same parent, ignore those changes, they were
			// already covered by the loop above.
			if (change != it.value() && it.value()->type() != ChangeType::Deletion
				 && change->newLabel() == it.value()->newLabel())
			{
				directConflicts_.insert(change, it.value());
				directConflicts_.insert(it.value(), change);
			}
			++it;
		}
	}

	// If this change is a deletion, see if some other change moves/inserts a child node, which is a conflict.
	// Changes of existing labels will anyway be in conflict with the deletion of the corresponding nodes. Thus
	// we do not explicitly look for them here.
	if (change->type() == ChangeType::Deletion)
	{
		it = changesForChildren_.find(change->nodeId());
		while (it != changesForChildren_.end() && it.key() == change->nodeId())
		{
			if ( (it.value()->type() == ChangeType::Insertion || it.value()->type() == ChangeType::Move)
				  && it.value()->newParentId() == change->nodeId())
			{
				directConflicts_.insert(change, it.value());
				directConflicts_.insert(it.value(), change);
			}
			++it;
		}
	}

	// If this change inserts or moves a node into a deleted node this is a conflict.
	if (change->type() == ChangeType::Move || change->type() == ChangeType::Insertion)
	{
		Q_ASSERT(!change->newParentId().isNull());
		it = changesForNode_.find(change->newParentId());
		while (it != changesForNode_.end() && it.key() == change->newParentId())
		{
			if ( it.value()->type() == ChangeType::Deletion)
			{
				directConflicts_.insert(change, it.value());
				directConflicts_.insert(it.value(), change);
			}
			++it;
		}
	}
}

void ChangeGraph::recomputeAllDependencies(GenericTree* baseTree)
{
	dependencies_.clear();

	for (auto change : changes_)
		recomputeDependenciesForChange(change, baseTree);
}


void ChangeGraph::recomputeDependenciesForChange(MergeChange* change, GenericTree* baseTree)
{
	addParentPresentDependency(change, baseTree);
	addChildrenRemovedDependency(change, baseTree);
	addLabelDependency(change, baseTree);
	addMoveDependency(change, baseTree);
}


void ChangeGraph::addParentPresentDependency(MergeChange* change, GenericTree* baseTree)
{
	// Some changes require that the parent node must exist
	// If it is not already in base, find the change that introduces it and depend on it
	if (change->type() == ChangeType::Insertion || change->type() == ChangeType::Move)
	{
		Q_ASSERT(!change->newParentId().isNull());
		bool existsInBase = baseTree->find(change->newParentId(), true);
		if (!existsInBase)
		{
			// There must be an insertion change introducing this node. Add a dependency on it.
			bool parentInsertionChangeFound = false;
			auto it = changesForNode_.find(change->newParentId());
			while (it != changesForNode_.end() && it.key() == change->newParentId())
			{
				if (it.value()->type() == ChangeType::Insertion)
				{
					dependencies_.insert(change, it.value());
					parentInsertionChangeFound = true;
				}
				++it;
			}
			Q_ASSERT(parentInsertionChangeFound);
		}
	}
}

void ChangeGraph::addChildrenRemovedDependency(MergeChange* change, GenericTree* baseTree)
{
	// A delete change requires that the node has no children left.
	// Such a change must depend on all changes that delete or move children out.
	if (change->type() == ChangeType::Deletion)
	{
		auto baseNode = baseTree->find(change->nodeId(), true);
		Q_ASSERT(baseNode);
		for (auto child : baseNode->children())
		{
			bool changeFound = false;
			auto it = changesForNode_.find(child->id());
			while (it != changesForNode_.end() && it.key() == child->id())
			{
				if (it.value()->type() == ChangeType::Deletion
					 || (it.value()->type() == ChangeType::Move && it.value()->oldParentId() == change->nodeId()))
				{
					dependencies_.insert(change, it.value());
					changeFound = true;
				}
				++it;
			}
			Q_ASSERT(changeFound);
		}
	}
}

void ChangeGraph::addLabelDependency(MergeChange* change, GenericTree* baseTree)
{
	// Labels must be unique. All changes that introduce new labels must depend on other changes
	// that change the existing old label.
	if (change->type() == ChangeType::Insertion || change->type() == ChangeType::Move
		 || change->updateFlags().testFlag(ChangeDescription::Label))
	{
		Q_ASSERT(!change->newParentId().isNull());
		auto baseNode = baseTree->find(change->newParentId(), true);
		Q_ASSERT(baseNode);

		bool clashingLabelFoundInBase = baseNode->child(change->newLabel());

		bool changeFound = false;
		auto it = changesForChildren_.find(change->newParentId());
		while (it != changesForChildren_.end() && it.key() == change->newParentId())
		{
			if (it.value()->oldLabel() == change->newLabel() &&
					(	it.value()->updateFlags().testFlag(ChangeDescription::Label)
						|| it.value()->type() == ChangeType::Deletion
						|| (it.value()->type() == ChangeType::Move && it.value()->oldParentId() == change->nodeId())
					)
				 )
			{
				dependencies_.insert(change, it.value());
				changeFound = true;
			}
			++it;
		}
		Q_ASSERT(changeFound == clashingLabelFoundInBase);
	}
}


void ChangeGraph::addMoveDependency(MergeChange* change, GenericTree* baseTree)
{
	// In order to detect cycles each move change depends on the first move change that moves an ancestor of it.
	if (change->type() != ChangeType::Move)
		return;

	// This change might move some tree into a newly inserted tree. Traverse this newly inserted tree first.
	auto ancestorId = change->newParentId();
	bool ancestorIsInserted = true;

	while (ancestorIsInserted)
	{
		ancestorIsInserted = false;
		auto it = changesForNode_.find(ancestorId);
		while (it != changesForNode_.end() && it.key() == ancestorId)
		{
			if (it.value()->type() == ChangeType::Insertion)
			{
				ancestorId = it.value()->newParentId();
				ancestorIsInserted = true;
				break;
			}
			else
				++it;
		}
	}

	// At this point we have an ancestor ID which belongs to a node in the baseTree.
	// We should traverse the ancestor chain until we reach the root node or until we reach a moved node
	Q_ASSERT(!ancestorId.isNull());
	while (!ancestorId.isNull())
	{
		bool dependencyFound = false;
		auto it = changesForNode_.find(ancestorId);
		while (it != changesForNode_.end() && it.key() == ancestorId)
		{
			if (it.value()->type() == ChangeType::Move)
			{
				dependencies_.insert(change, it.value());
				dependencyFound = true;
				// Keep looking for additional dependencies, perhaps both branches move the same node to different places.
			}

			++it;
		}

		// If we found the first move, then we're done.
		if (dependencyFound) break;
		else
		{
			auto ancestorNode = baseTree->find(ancestorId, true);
			Q_ASSERT(ancestorNode);
			ancestorId = ancestorNode->parentId();
		}
	}
}

}
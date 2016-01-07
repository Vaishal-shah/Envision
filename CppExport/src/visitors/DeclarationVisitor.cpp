/***********************************************************************************************************************
 **
 ** Copyright (c) 2011, 2015 ETH Zurich
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

#include "DeclarationVisitor.h"
#include "ExpressionVisitor.h"
#include "StatementVisitor.h"
#include "ElementVisitor.h"

#include "OOModel/src/declarations/Project.h"
#include "OOModel/src/declarations/Module.h"
#include "OOModel/src/declarations/Class.h"
#include "OOModel/src/declarations/Declaration.h"
#include "OOModel/src/declarations/Method.h"
#include "OOModel/src/declarations/NameImport.h"
#include "OOModel/src/declarations/VariableDeclaration.h"
#include "OOModel/src/declarations/ExplicitTemplateInstantiation.h"
#include "OOModel/src/declarations/TypeAlias.h"
#include "OOModel/src/expressions/MetaCallExpression.h"
#include "OOModel/src/declarations/MetaDefinition.h"
#include "OOModel/src/expressions/ArrayInitializer.h"
#include "OOModel/src/expressions/types/AutoTypeExpression.h"

#include "Export/src/tree/SourceDir.h"
#include "Export/src/tree/SourceFile.h"
#include "Export/src/tree/CompositeFragment.h"
#include "Export/src/tree/TextFragment.h"

#include "Comments/src/nodes/CommentNode.h"

using namespace Export;
using namespace OOModel;

namespace CppExport {

SourceFragment* DeclarationVisitor::visit(Declaration* declaration)
{
	if (auto castDeclaration = DCast<Method>(declaration)) return visit(castDeclaration);
	if (auto castDeclaration = DCast<MetaDefinition>(declaration)) return visit(castDeclaration);
	if (auto castDeclaration = DCast<Class>(declaration)) return visit(castDeclaration);
	if (auto castDeclaration = DCast<VariableDeclaration>(declaration)) return visit(castDeclaration);
	if (auto castDeclaration = DCast<TypeAlias>(declaration)) return visit(castDeclaration);
	if (auto castDeclaration = DCast<NameImport>(declaration)) return visit(castDeclaration);

	notAllowed(declaration);

	auto fragment = new CompositeFragment(declaration);
	*fragment << "Invalid Declaration";
	return fragment;
}

SourceDir* DeclarationVisitor::visitProject(Project* project, SourceDir* parent)
{
	auto projectDir = parent ? &parent->subDir(project->name()) : new SourceDir(nullptr, "src");

	for (auto node : *project->projects()) visitProject(node, projectDir);
	for (auto node : *project->modules()) visitModule(node, projectDir);
	for (auto node : *project->classes()) visitTopLevelClass(node, projectDir);

	notAllowed(project->methods());
	notAllowed(project->fields());

	return projectDir;
}

SourceDir* DeclarationVisitor::visitModule(Module* module, SourceDir* parent)
{
	Q_ASSERT(parent);
	auto moduleDir = &parent->subDir(module->name());

	for (auto node : *module->modules()) visitModule(node, moduleDir);
	for (auto node : *module->classes()) visitTopLevelClass(node, moduleDir);

	notAllowed(module->methods());
	notAllowed(module->fields());

	return moduleDir;
}

SourceFile* DeclarationVisitor::visitTopLevelClass(Class* classs, SourceDir* parent)
{
	Q_ASSERT(parent);
	auto classFile = &parent->file(classs->name() + ".cpp");

	auto fragment = classFile->append(new CompositeFragment(classs, "sections"));

	auto imports = fragment->append(new CompositeFragment(classs, "vertical"));
	for (auto node : *classs->subDeclarations())
	{
		if (auto ni = DCast<NameImport>(node)) *imports << visit(ni);
		else notAllowed(node);
	}

	*fragment << visit(classs);

	return classFile;
}

SourceFragment* DeclarationVisitor::visitTopLevelClass(Class* classs)
{
	if (!headerVisitor()) return visit(classs);

	auto fragment = new CompositeFragment(classs, "spacedSections");
	*fragment << visit(classs);

	auto filter = [](Method* method) { return !method->typeArguments()->isEmpty() ||
															(method->modifiers()->isSet(OOModel::Modifier::Inline) &&
															 !method->modifiers()->isSet(OOModel::Modifier::Default) &&
															 !method->modifiers()->isSet(OOModel::Modifier::Deleted)); };
	*fragment << list(classs->methods(), DeclarationVisitor(SOURCE_VISITOR, data()), "spacedSections", filter);
	return fragment;
}

SourceFragment* DeclarationVisitor::visit(Class* classs)
{
	auto fragment = new CompositeFragment(classs);

	if (sourceVisitor())
	{
		//TODO
		auto sections = fragment->append( new CompositeFragment(classs, "sections"));
		*sections << list(classs->enumerators(), ElementVisitor(data()), "enumerators");
		*sections << list(classs->classes(), this, "sections");

		*sections << list(classs->methods(), this, "spacedSections", [](Method* method)
		{
			return method->typeArguments()->isEmpty() &&
						 !method->modifiers()->isSet(OOModel::Modifier::Inline) &&
						 !method->modifiers()->isSet(OOModel::Modifier::Abstract) &&
						 !method->modifiers()->isSet(OOModel::Modifier::Deleted) &&
						 !method->modifiers()->isSet(OOModel::Modifier::Default);
		});
		*sections << list(classs->fields(), this, "vertical");
	}
	else
	{
		bool friendClass = false;
		if (auto parentClass = classs->firstAncestorOfType<Class>())
			friendClass = parentClass->friends()->isAncestorOf(classs);

		if (!friendClass)
		{
			*fragment << declarationComments(classs);

			if (!classs->typeArguments()->isEmpty())
				*fragment << list(classs->typeArguments(), ElementVisitor(data()), "templateArgsList");

			*fragment << printAnnotationsAndModifiers(classs);
		}
		else
			*fragment << "friend ";

		if (Class::ConstructKind::Class == classs->constructKind()) *fragment << "class ";
		else if (Class::ConstructKind::Struct == classs->constructKind()) *fragment << "struct ";
		else if (Class::ConstructKind::Enum == classs->constructKind()) *fragment << "enum ";
		else notAllowed(classs);

		if (auto namespaceModule = classs->firstAncestorOfType<Module>())
			*fragment << namespaceModule->name().toUpper() + "_API ";

		*fragment << classs->nameNode();

		if (!friendClass)
		{
			if (!classs->baseClasses()->isEmpty())
				// TODO: inheritance modifiers like private, virtual... (not only public)
				*fragment << " : public " << list(classs->baseClasses(), ExpressionVisitor(data()), "comma");

			auto sections = fragment->append( new CompositeFragment(classs, "bodySections"));
			*sections << list(classs->metaCalls(), ExpressionVisitor(data()), "sections");

			if (classs->enumerators()->size() > 0)
				error(classs->enumerators(), "Enum unhandled"); // TODO

			auto publicSection = new CompositeFragment(classs, "accessorSections");
			auto publicFilter = [](Declaration* declaration) { return declaration->modifiers()->isSet(Modifier::Public); };
			bool hasPublicSection = addMemberDeclarations(classs, publicSection, publicFilter);

			auto protectedSection = new CompositeFragment(classs, "accessorSections");
			auto protectedFilter = [](Declaration* declaration) { return declaration->modifiers()->isSet(Modifier::Protected); };
			bool hasProtectedSection = addMemberDeclarations(classs, protectedSection, protectedFilter);

			auto privateSection = new CompositeFragment(classs, "accessorSections");
			auto privateFilter = [](Declaration* declaration) { return !declaration->modifiers()->isSet(Modifier::Public) &&
																						  !declaration->modifiers()->isSet(Modifier::Protected); };
			bool hasPrivateSection = addMemberDeclarations(classs, privateSection, privateFilter);

			if (hasPublicSection)
			{
				if (hasProtectedSection || hasPrivateSection || classs->constructKind() != Class::ConstructKind::Struct)
					*sections << "public:";
				sections->append(publicSection);
			}
			if (hasProtectedSection)
			{
				if (hasPublicSection) *sections << "\n"; // add newline between two accessor sections

				*sections << "protected:";
				sections->append(protectedSection);
			}
			if (hasPrivateSection)
			{
				if (hasPublicSection || hasProtectedSection) *sections << "\n"; // add newline between two accessor sections

				*sections << "private:";
				sections->append(privateSection);
			}
		}

		*fragment << ";";
	}

	return fragment;
}

template<typename Predicate>
bool DeclarationVisitor::addMemberDeclarations(Class* classs, CompositeFragment* section, Predicate filter)
{
	auto subDeclarations = list(classs->subDeclarations(), this, "sections", filter);
	auto fields = list(classs->fields(), this, "vertical", filter);
	auto friends = list(classs->friends(), this, "sections", filter);
	auto classes = list(classs->classes(), this, "sections", filter);
	auto methods = list(classs->methods(), this, "sections", filter);

	*section << subDeclarations << fields << friends << classes << methods;
	return !subDeclarations->fragments().empty() ||
			 !fields->fragments().empty() ||
			 !friends->fragments().empty() ||
			 !classes->fragments().empty() ||
			 !methods->fragments().empty();
}

bool DeclarationVisitor::shouldExportMethod(Method* method)
{
	if (headerVisitor())
	{
		auto parentDeclaration = method->firstAncestorOfType<Declaration>();
		Q_ASSERT(parentDeclaration);

		for (auto expression : *(parentDeclaration->metaCalls()))
			if (auto metaCall = DCast<OOModel::MetaCallExpression>(expression))
				for (auto generatedMethod : Model::Node::childrenOfType<Method>(metaCall->generatedTree()))
					if (methodSignaturesMatch(method, generatedMethod))
						return false;
	}
	return true;
}

bool DeclarationVisitor::methodSignaturesMatch(Method* method, Method* other)
{
	// TODO: this method can be made more specific and relocated to generic method overload resolution
	if (method->arguments()->size() != other->arguments()->size() || method->symbolName() != other->symbolName())
		return false;

	for (auto i = 0; i < method->arguments()->size(); i++)
		if (method->arguments()->at(i)->name() != other->arguments()->at(i)->name())
			return false;

	return true;
}

SourceFragment* DeclarationVisitor::visit(MetaDefinition* metaDefinition)
{
	auto fragment = new CompositeFragment(metaDefinition, "emptyLineAtEnd");
	auto macro = new CompositeFragment(metaDefinition, "macro");
	*macro << "#define " << metaDefinition->nameNode();
	*macro << list(metaDefinition->arguments(), ElementVisitor(data()), "argsList");
	auto body = new CompositeFragment(metaDefinition->context(), "macroBody");
	if (auto context = DCast<Module>(metaDefinition->context()))
		*body << list(context->classes(), DeclarationVisitor(MACRO_VISITOR, data()), "spacedSections");
	else if (auto context = DCast<Class>(metaDefinition->context()))
	{
		*body << list(context->metaCalls(), ExpressionVisitor(data()), "sections");
		*body << list(context->methods(), DeclarationVisitor(MACRO_VISITOR, data()), "spacedSections");
	}
	*macro << body;
	*fragment << macro;
	return fragment;
}

SourceFragment* DeclarationVisitor::visit(Method* method)
{
	if (!shouldExportMethod(method)) return nullptr;

	auto fragment = new CompositeFragment(method);

	if (!sourceVisitor())
		*fragment << declarationComments(method);

	if (!headerVisitor())
		if (method->modifiers()->isSet(Modifier::Inline))
			if (auto parentClass = method->firstAncestorOfType<Class>())
				if (!parentClass->typeArguments()->isEmpty())
					*fragment << list(parentClass->typeArguments(), ElementVisitor(data()), "templateArgsList");

	if (!method->typeArguments()->isEmpty())
		*fragment << list(method->typeArguments(), ElementVisitor(data()), "templateArgsList");

	if (!sourceVisitor())
		if (auto parentClass = method->firstAncestorOfType<Class>())
			if (parentClass->friends()->isAncestorOf(method))
				*fragment << "friend ";

	if (!sourceVisitor())
		*fragment << printAnnotationsAndModifiers(method);

	if (!headerVisitor())
		if (method->modifiers()->isSet(Modifier::Inline))
			*fragment << new TextFragment(method->modifiers(), "inline") << " ";

	if (method->results()->size() > 1)
		error(method->results(), "Cannot have more than one return value in C++");

	if (method->methodKind() == Method::MethodKind::Conversion)
	{
		if (sourceVisitor())
			if (auto parentClass = method->firstAncestorOfType<Class>())
				*fragment << parentClass->name() << "::";
		*fragment << "operator ";
	}

	if (method->methodKind() != Method::MethodKind::Constructor &&
		 method->methodKind() != Method::MethodKind::Destructor)
	{
		if (!method->results()->isEmpty())
			*fragment << expression(method->results()->at(0)->typeExpression()) << " ";
		else
			*fragment << "void ";
	}

	if (headerVisitor() && method->firstAncestorOfType<Declaration>() == method->firstAncestorOfType<Module>())
		*fragment << "CORE_API ";

	if (sourceVisitor() && method->methodKind() != Method::MethodKind::Conversion)
		if (auto parentClass = method->firstAncestorOfType<Class>())
			*fragment << parentClass->name() << "::";

	if (method->methodKind() == Method::MethodKind::Destructor && !method->name().startsWith("~")) *fragment << "~";
	if (method->methodKind() == Method::MethodKind::OperatorOverload) *fragment << "operator";
	*fragment << method->nameNode();

	*fragment << list(method->arguments(), ElementVisitor(data()), "argsList");
	if (method->modifiers()->isSet(Modifier::Const))
		*fragment << " " << new TextFragment(method->modifiers(), "const");

	if (!headerVisitor())
		if (!method->memberInitializers()->isEmpty())
			*fragment << " : " << list(method->memberInitializers(), ElementVisitor(data()), "comma");

	if (!method->throws()->isEmpty())
	{
		*fragment << " throw (";
		*fragment << list(method->throws(), ExpressionVisitor(data()), "comma");
		*fragment << ")";
	}

	if (!sourceVisitor())
	{
		if (method->modifiers()->isSet(Modifier::Override))
			*fragment << new TextFragment(method->modifiers(), " override");
		if (method->modifiers()->isSet(Modifier::Default))
			*fragment << new TextFragment(method->modifiers(), " = default");
		if (method->modifiers()->isSet(Modifier::Deleted))
			*fragment << new TextFragment(method->modifiers(), " = delete");
		if (method->modifiers()->isSet(Modifier::Abstract))
			*fragment << new TextFragment(method->modifiers(), " = 0");
		*fragment << ";";
	}

	if (!headerVisitor())
		*fragment << list(method->items(), StatementVisitor(data()), "body");

	notAllowed(method->subDeclarations());
	notAllowed(method->memberInitializers());

	return fragment;
}

SourceFragment* DeclarationVisitor::declarationComments(Declaration* declaration)
{
	if (auto commentNode = DCast<Comments::CommentNode>(declaration->comment()))
	{
		auto commentFragment = new CompositeFragment(commentNode->lines(), "declarationComment");
		for (auto line : *(commentNode->lines()))
			*commentFragment << line;
		return commentFragment;
	}

	return nullptr;
}

SourceFragment* DeclarationVisitor::visit(VariableDeclaration* variableDeclaration)
{
	auto fragment = new CompositeFragment(variableDeclaration);
	if (!sourceVisitor() && !variableDeclaration->modifiers()->isSet(Modifier::ConstExpr))
	{
		*fragment << declarationComments(variableDeclaration);
		*fragment << printAnnotationsAndModifiers(variableDeclaration);
		*fragment << expression(variableDeclaration->typeExpression()) << " " << variableDeclaration->nameNode();
		if (variableDeclaration->initialValue() && !variableDeclaration->modifiers()->isSet(Modifier::Static))
		{
			//TODO: Use {} instead of =
			if (!DCast<ArrayInitializer>(variableDeclaration->initialValue())) *fragment << " = ";
			*fragment << expression(variableDeclaration->initialValue());
		}
		if (!DCast<Expression>(variableDeclaration->parent())) *fragment << ";";
	}
	else if (sourceVisitor() && (!DCast<Field>(variableDeclaration) ||
										  variableDeclaration->modifiers()->isSet(Modifier::Static) ||
										  variableDeclaration->modifiers()->isSet(Modifier::ConstExpr)))
	{
		if (!DCast<Field>(variableDeclaration) || variableDeclaration->modifiers()->isSet(Modifier::ConstExpr))
			*fragment << printAnnotationsAndModifiers(variableDeclaration);
		*fragment << expression(variableDeclaration->typeExpression()) << " ";

		if (DCast<Field>(variableDeclaration))
			if (auto parentClass = variableDeclaration->firstAncestorOfType<Class>())
				*fragment << parentClass->name() << "::";

		*fragment << variableDeclaration->nameNode();
		if (variableDeclaration->initialValue())
		{
			if (!DCast<ArrayInitializer>(variableDeclaration->initialValue()) ||
				 DCast<AutoTypeExpression>(variableDeclaration->typeExpression())) *fragment << " = ";
			*fragment << expression(variableDeclaration->initialValue());
		}

		if (!DCast<Expression>(variableDeclaration->parent())) *fragment << ";";
	}
	return fragment;
}

SourceFragment* DeclarationVisitor::printAnnotationsAndModifiers(Declaration* declaration)
{
	auto fragment = new CompositeFragment(declaration, "vertical");
	if (!declaration->annotations()->isEmpty()) // avoid an extra new line if there are no annotations
		*fragment << list(declaration->annotations(), StatementVisitor(data()), "vertical");
	auto header = fragment->append(new CompositeFragment(declaration, "space"));

	if (declaration->modifiers()->isSet(Modifier::ConstExpr))
		*header << new TextFragment(declaration->modifiers(), "constexpr");
	if (declaration->modifiers()->isSet(Modifier::Static))
		*header << new TextFragment(declaration->modifiers(), "static");
	if (declaration->modifiers()->isSet(Modifier::Final))
		*header << new TextFragment(declaration->modifiers(), "final");
	if (declaration->modifiers()->isSet(Modifier::Virtual))
		*header << new TextFragment(declaration->modifiers(), "virtual");
	if (declaration->modifiers()->isSet(Modifier::Explicit))
		*header << new TextFragment(declaration->modifiers(), "explicit");

	return fragment;
}

SourceFragment* DeclarationVisitor::visit(NameImport* nameImport)
{
	auto fragment = new CompositeFragment(nameImport);
	notAllowed(nameImport->annotations());

	Q_ASSERT(!nameImport->importAll());
	*fragment << "using " << expression(nameImport->importedName()) << ";";
	return fragment;
}

SourceFragment* DeclarationVisitor::visit(ExplicitTemplateInstantiation* eti)
{
	notAllowed(eti);
	return new TextFragment(eti);
}

SourceFragment* DeclarationVisitor::visit(TypeAlias* typeAlias)
{
	auto fragment = new CompositeFragment(typeAlias);

	if (!typeAlias->typeArguments()->isEmpty())
		*fragment << list(typeAlias->typeArguments(), ElementVisitor(data()), "templateArgsList");

	*fragment << "using " << typeAlias->nameNode() << " = " << expression(typeAlias->typeExpression()) << ";";
	return fragment;
}
}

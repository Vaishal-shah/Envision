/***********************************************************************************************************************
 **
 ** Copyright (c) 2011, 2013 ETH Zurich
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

#pragma once

#include "cppimport_api.h"
#include "CppImportLogger.h"

namespace CppImport {

class ExpressionVisitor;

class CppImportUtilities
{
	public:
		CppImportUtilities(CppImportLogger* logger, ExpressionVisitor* visitor);
		OOModel::Expression* convertClangType(clang::QualType qualType);
		OOModel::BinaryOperation::OperatorTypes convertClangOpcode(clang::BinaryOperatorKind kind);
		OOModel::AssignmentExpression::AssignmentTypes convertClangAssignOpcode(clang::BinaryOperatorKind kind);
		OOModel::UnaryOperation::OperatorTypes convertUnaryOpcode(clang::UnaryOperatorKind kind);
		OOModel::Modifier::ModifierFlag convertAccessSpecifier(clang::AccessSpecifier as);
		OOModel::Modifier::ModifierFlag convertStorageSpecifier(clang::StorageClass storage);

		OOModel::Expression* translateNestedNameSpecifier(clang::NestedNameSpecifier* nestedName);

		OOModel::Expression* convertTemplateArgument(const clang::TemplateArgument& templateArg);

		OOModel::BinaryOperation::OperatorTypes translateBinaryOverloadOp(clang::OverloadedOperatorKind kind);
		OOModel::AssignmentExpression::AssignmentTypes translateAssignOverloadOp(clang::OverloadedOperatorKind kind);
		OOModel::UnaryOperation::OperatorTypes translateUnaryOverloadOp
		(clang::OverloadedOperatorKind kind, unsigned numArgs);

		enum class OverloadKind : int {Unsupported, Unary, Binary, Assign, MethodCall, ReferenceExpr, Comma};
		OverloadKind getOverloadKind(clang::OverloadedOperatorKind kind, unsigned numArgs);

		OOModel::Expression* createErrorExpression(QString reason);
	private:
		OOModel::Expression* convertBuiltInClangType(const clang::Type* type);
		CppImportLogger* log_;
		ExpressionVisitor* exprVisitor_{};
};
}

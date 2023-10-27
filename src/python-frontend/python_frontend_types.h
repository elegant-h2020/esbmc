#pragma once

enum class StatementType
{
  VARIABLE_ASSIGN,
  COMPOUND_ASSIGN,
  FUNC_DEFINITION,
  IF_STATEMENT,
  WHILE_STATEMENT,
  EXPR,
  RETURN,
  ASSERT,
  UNKNOWN,
};

enum class ExpressionType
{
  UNARY_OPERATION,
  BINARY_OPERATION,
  LOGICAL_OPERATION,
  LITERAL,
  VARIABLE_REF,
  FUNC_CALL,
  UNKNOWN,
};
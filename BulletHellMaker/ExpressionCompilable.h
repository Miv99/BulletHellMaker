#pragma once
#include "exprtk.hpp"
#include "SymbolTable.h"

/*
An object that uses expressions that can be compiled. In the context of this game, this 
is an object used in gameplay that has expressions or has unique objects (objects that have 
a 1:1 relationship with this object) that have expressions.

Since everything is compiled as floats, any values that are needed as ints will be rounded.
*/
class ExpressionCompilable {
public:
	/*
	Compiles every expression used in this object as well as every expression used in any unique objects.
	It's possible to just store the value of the expression as soon as it's compiled symbolTable was returned
	from a ValueSymbolTable or ExprSymbolTable because every symbol in symbolTable will be a constant.

	symbolTables - a list of symbol_tables; its content combined defines all symbols that will be needed to compile the above expressions.
		The symbol_tables should be ordered in descending priority such that in the case of the same symbol being defined multiple times, 
		the definition in the farthest-back symbol_table will be used.
	*/
	virtual void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) = 0;

	ValueSymbolTable getSymbolTable() {
		return symbolTable;
	}
	void setSymbolTable(ValueSymbolTable symbolTable) {
		this->symbolTable = symbolTable;
	}

protected:
	// The ValueSymbolTable that defines or redelegates all 
	// symbols that are used in this object and its unique objects
	ValueSymbolTable symbolTable;
};
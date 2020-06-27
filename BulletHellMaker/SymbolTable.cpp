#include "SymbolTable.h"

std::string ExprSymbolTable::format() const {
    std::string res;
    for (auto it = map.begin(); it != map.end(); it++) {
        res += formatString(it->first) + formatString(it->second.expressionStr);
    }
    return res;
}

void ExprSymbolTable::load(std::string formattedString) {
    auto items = split(formattedString, DELIMITER);
    map.clear();
    for (int i = 0; i < items.size(); i += 2) {
        map[items[i]] = { items[i + 1] };
    }
}

void ExprSymbolTable::setSymbol(std::string symbol, std::string expressionStr) {
    map[symbol] = { expressionStr };
}

void ExprSymbolTable::removeSymbol(std::string symbol) {
    map.erase(symbol);
}

exprtk::symbol_table<float> ExprSymbolTable::toLowerLevelSymbolTable(exprtk::symbol_table<float> higherLevelSymbolTable) {
    exprtk::parser<float> parser;
    exprtk::symbol_table<float> table;
    for (auto it = map.begin(); it != map.end(); it++) {
        exprtk::expression<float> expression = exprtk::expression<float>();
        expression.register_symbol_table(higherLevelSymbolTable);
        parser.compile(it->second.expressionStr, expression);
        float value = expression.value();
        table.add_variable(it->first, value, true);
    }
    return table;
}

std::string ValueSymbolTable::format() const {
    std::string res;
    for (auto it = map.begin(); it != map.end(); it++) {
        res += formatString(it->first) + tos(it->second.value) + formatBool(it->second.redelegated);
    }
    return res;
}

void ValueSymbolTable::load(std::string formattedString) {
    auto items = split(formattedString, DELIMITER);
    map.clear();
    for (int i = 0; i < items.size(); i += 3) {
        map[items[i]] = { std::stof(items[i + 1]), unformatBool(items[i + 2]) };
    }
}

void ValueSymbolTable::setSymbol(std::string symbol, float value, bool redelegated) {
    map[symbol] = { value, redelegated };
}

void ValueSymbolTable::removeSymbol(std::string symbol) {
    map.erase(symbol);
}

exprtk::symbol_table<float> ValueSymbolTable::toExprtkSymbolTable() {
    exprtk::symbol_table<float> table;
    for (auto it = map.begin(); it != map.end(); it++) {
        // Add as constants so that the symbol_table is still valid even after the floats go out of scope

        if (!it->second.redelegated) {
            table.add_variable(it->first, it->second.value, true);
        }
    }
    return table;
}

exprtk::symbol_table<float> ValueSymbolTable::toZeroFilledSymbolTable() {
    exprtk::symbol_table<float> table;
    for (auto it = map.begin(); it != map.end(); it++) {
        // Add as constants so that the symbol_table is still valid even after the floats go out of scope

        if (it->second.redelegated) {
            float zero = 0;
            table.add_variable(it->first, zero, true);
        } else {
            table.add_variable(it->first, it->second.value, true);
        }
    }
    return table;
}

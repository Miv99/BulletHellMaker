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

exprtk::symbol_table<float> ExprSymbolTable::getLowerLevelSymbolTable(exprtk::symbol_table<float> higherLevelSymbolTable) {
    exprtk::parser<float> parser;
    exprtk::symbol_table<float> table;
    for (auto it = map.begin(); it != map.end(); it++) {
        it->second.expression = exprtk::expression<float>();
        it->second.expression.register_symbol_table(higherLevelSymbolTable);
        parser.compile(it->second.expressionStr, it->second.expression);
        float value = it->second.expression.value();
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

exprtk::symbol_table<float> ValueSymbolTable::getSymbolTable() {
    exprtk::symbol_table<float> table;
    for (auto it = map.begin(); it != map.end(); it++) {
        if (!it->second.redelegated) {
            table.add_variable(it->first, it->second.value, true);
        }
    }
    return table;
}

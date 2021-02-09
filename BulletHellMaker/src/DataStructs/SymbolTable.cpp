#include <DataStructs/SymbolTable.h>

std::string ExprSymbolTable::format() const {
    std::string res;
    for (auto it = map.begin(); it != map.end(); it++) {
        res += formatString(it->first) + formatString(it->second.expressionStr);
    }
    return res;
}

void ExprSymbolTable::load(std::string formattedString) {
    auto items = split(formattedString, TextMarshallable::DELIMITER);
    map.clear();
    for (int i = 0; i < items.size(); i += 2) {
        map[items.at(i)] = { items.at(i + 1) };
    }
}

nlohmann::json ExprSymbolTable::toJson() {
    nlohmann::json j;

    for (auto it = map.begin(); it != map.end(); it++) {
        j[it->first] = it->second.expressionStr;
    }

    return j;
}

void ExprSymbolTable::load(const nlohmann::json& j) {
    map.clear();
    for (auto item : j.items()) {
        map[item.key()] = { item.value() };
    }
}

void ExprSymbolTable::setSymbol(std::string symbol, std::string expressionStr) {
    map[symbol] = { expressionStr };
}

void ExprSymbolTable::removeSymbol(std::string symbol) {
    map.erase(symbol);
}

exprtk::symbol_table<float> ExprSymbolTable::toLowerLevelSymbolTable(std::vector<exprtk::symbol_table<float>> higherLevelSymbolTables) {
    exprtk::expression<float> expression = exprtk::expression<float>();
    for (int i = higherLevelSymbolTables.size() - 1; i >= 0; i--) {
        expression.register_symbol_table(higherLevelSymbolTables[i]);
    }

    exprtk::parser<float> parser;
    exprtk::symbol_table<float> table;
    for (auto it = map.begin(); it != map.end(); it++) {
        parser.compile(it->second.expressionStr, expression);
        float value = expression.value();
        table.add_constant(it->first, value);
    }
    return std::move(table);
}

exprtk::symbol_table<float> ExprSymbolTable::toLowerLevelSymbolTable(exprtk::expression<float> expression) {
    exprtk::parser<float> parser;
    exprtk::symbol_table<float> table;
    for (auto it = map.begin(); it != map.end(); it++) {
        parser.compile(it->second.expressionStr, expression);
        float value = expression.value();
        table.add_constant(it->first, value);
    }
    return table;
}

std::map<std::string, ExprSymbolDefinition>::const_iterator ExprSymbolTable::getIteratorBegin() {
    return map.begin();
}

std::map<std::string, ExprSymbolDefinition>::const_iterator ExprSymbolTable::getIteratorEnd() {
    return map.end();
}

ExprSymbolDefinition ExprSymbolTable::getSymbolDefinition(std::string symbol) const {
    return map.at(symbol);
}

bool ExprSymbolTable::hasSymbol(std::string symbol) const {
    return map.find(symbol) != map.end();
}

bool ExprSymbolTable::isEmpty() const {
    return map.size() == 0;
}

std::string ValueSymbolTable::format() const {
    std::string res;
    for (auto it = map.begin(); it != map.end(); it++) {
        res += formatString(it->first) + tos(it->second.value) + formatBool(it->second.redelegated);
    }
    return res;
}

void ValueSymbolTable::load(std::string formattedString) {
    auto items = split(formattedString, TextMarshallable::DELIMITER);
    map.clear();
    for (int i = 0; i < items.size(); i += 3) {
        map[items.at(i)] = { std::stof(items.at(i + 1)), unformatBool(items.at(i + 2)) };
    }
}

nlohmann::json ValueSymbolTable::toJson() {
    nlohmann::json j;

    for (auto it = map.begin(); it != map.end(); it++) {
        j[it->first] = nlohmann::json{ {"value", it->second.value}, {"redelegated", it->second.redelegated} };
    }

    return j;
}

void ValueSymbolTable::load(const nlohmann::json& j) {
    map.clear();
    for (auto item : j.items()) {
        float value;
        bool redelegated;
        
        item.value().at("value").get_to(value);
        item.value().at("redelegated").get_to(redelegated);

        map[item.key()] = { value, redelegated };
    }
}

void ValueSymbolTable::setSymbol(std::string symbol, float value, bool redelegated) {
    map[symbol] = { value, redelegated };
}

void ValueSymbolTable::removeSymbol(std::string symbol) {
    map.erase(symbol);
}

exprtk::symbol_table<float> ValueSymbolTable::toExprtkSymbolTable() const {
    exprtk::symbol_table<float> table;
    for (auto it = map.begin(); it != map.end(); it++) {
        // Add as constants so that the symbol_table is still valid even after the floats go out of scope

        if (!it->second.redelegated) {
            table.add_constant(it->first, it->second.value);
        }
    }
    return table;
}

exprtk::symbol_table<float> ValueSymbolTable::toZeroFilledSymbolTable() const {
    exprtk::symbol_table<float> table;
    for (auto it = map.begin(); it != map.end(); it++) {
        // Add as constants so that the symbol_table is still valid even after the floats go out of scope

        if (it->second.redelegated) {
            float zero = 0;
            table.add_constant(it->first, zero);
        } else {
            table.add_constant(it->first, it->second.value);
        }
    }
    return table;
}

std::map<std::string, ValueSymbolDefinition>::const_iterator ValueSymbolTable::getIteratorBegin() {
    return map.begin();
}

std::map<std::string, ValueSymbolDefinition>::const_iterator ValueSymbolTable::getIteratorEnd() {
    return map.end();
}

ValueSymbolDefinition ValueSymbolTable::getSymbolDefinition(std::string symbol) const {
    return map.at(symbol);
}

bool ValueSymbolTable::hasSymbol(std::string symbol) const {
    return map.find(symbol) != map.end();
}

bool ValueSymbolTable::isEmpty() const {
    return map.size() == 0;
}

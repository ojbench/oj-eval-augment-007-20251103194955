/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"
#include <cctype>

#include <cstdint>

/* Implementation of the Statement class */

Statement::Statement() = default;

Statement::~Statement() = default;

// Helpers
static bool isIntegerString(const std::string &s) {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') i = 1;
    if (i == s.size()) return false;
    for (; i < s.size(); ++i) if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    return true;
}

/******** REM ********/
RemStatement::RemStatement(TokenScanner &scanner) {
    // consume rest tokens as comment
    while (scanner.hasMoreTokens()) scanner.nextToken();
}

void RemStatement::execute(EvalState &state, Program &program) {
    (void) state; (void) program;
}

/******** LET ********/
LetStatement::LetStatement(TokenScanner &scanner) {
    std::string t = scanner.nextToken();
    if (t.empty()) error("SYNTAX ERROR");
    TokenType tp = scanner.getTokenType(t);
    if (tp != WORD) error("SYNTAX ERROR");
    var = t;
    std::string eq = scanner.nextToken();
    if (eq != "=") error("SYNTAX ERROR");
    rhs = parseExp(scanner);
}

LetStatement::~LetStatement() {
    delete rhs;
}

void LetStatement::execute(EvalState &state, Program &program) {
    (void) program;
    int val = rhs->eval(state);
    state.setValue(var, val);
}

/******** PRINT ********/
PrintStatement::PrintStatement(TokenScanner &scanner) {
    exp = parseExp(scanner);
}

PrintStatement::~PrintStatement() {
    delete exp;
}

void PrintStatement::execute(EvalState &state, Program &program) {
    (void) program;
    std::cout << exp->eval(state) << std::endl;
}

/******** INPUT ********/
InputStatement::InputStatement(TokenScanner &scanner) {
    std::string t = scanner.nextToken();
    if (t.empty()) error("SYNTAX ERROR");
    // accept WORD or NUMBER as var name (special case allows numeric var name for INPUT)
    TokenType tp = scanner.getTokenType(t);
    if (tp != WORD && tp != NUMBER) error("SYNTAX ERROR");
    var = t;
}

void InputStatement::execute(EvalState &state, Program &program) {
    (void) program;
    while (true) {
        std::cout << " ? ";
        std::string s;
        if (!std::getline(std::cin, s)) return; // EOF
        s = trim(s);
        if (!isIntegerString(s)) {
            std::cout << "INVALID NUMBER" << std::endl;
            continue;
        }
        // echo the input then set value
        std::cout << s << std::endl;
        long long v = 0;
        try {
            v = std::stoll(s);
        } catch (...) {
            std::cout << "INVALID NUMBER" << std::endl;
            continue;
        }
        if (v < INT32_MIN || v > INT32_MAX) {
            std::cout << "INVALID NUMBER" << std::endl;
            continue;
        }
        state.setValue(var, static_cast<int>(v));
        break;
    }
}

/******** END ********/
EndStatement::EndStatement(TokenScanner &scanner) {
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}

void EndStatement::execute(EvalState &state, Program &program) {
    (void) state;
    program.requestStop();
}

/******** GOTO ********/
GotoStatement::GotoStatement(TokenScanner &scanner) {
    std::string t = scanner.nextToken();
    if (t.empty() || scanner.getTokenType(t) != NUMBER) error("SYNTAX ERROR");
    target = stringToInteger(t);
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}

void GotoStatement::execute(EvalState &state, Program &program) {
    (void) state;
    if (program.getSourceLine(target).empty()) error("LINE NUMBER ERROR");
    program.requestJump(target);
}

/******** IF ... THEN ********/
IfStatement::IfStatement(TokenScanner &scanner) {
    // read lhs tokens until relational op
    std::ostringstream lhsText;
    std::string token;
    while (scanner.hasMoreTokens()) {
        token = scanner.nextToken();
        if (token == "<" || token == ">" || token == "=") {
            // check for <=, >=, <> composite operators
            if (token == "<" && scanner.hasMoreTokens()) {
                std::string t2 = scanner.nextToken();
                if (t2 == "=") op = "<=";
                else if (t2 == ">") op = "<>";
                else { op = "<"; scanner.saveToken(t2); }
            } else if (token == ">" && scanner.hasMoreTokens()) {
                std::string t2 = scanner.nextToken();
                if (t2 == "=") op = ">=";
                else { op = ">"; scanner.saveToken(t2); }
            } else {
                op = token; // '='
            }
            break;
        } else {
            lhsText << token << ' ';
        }
    }
    if (op.empty()) error("SYNTAX ERROR");

    // read rhs tokens until THEN
    std::ostringstream rhsText;
    bool seenThen = false;
    while (scanner.hasMoreTokens()) {
        std::string t = scanner.nextToken();
        if (equalsIgnoreCase(t, "THEN")) { seenThen = true; break; }
        rhsText << t << ' ';
    }
    if (!seenThen) error("SYNTAX ERROR");

    // parse target line
    std::string tline = scanner.nextToken();
    if (tline.empty() || scanner.getTokenType(tline) != NUMBER) error("SYNTAX ERROR");
    target = stringToInteger(tline);
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");

    // now parse expressions
    TokenScanner s1(lhsText.str()); s1.ignoreWhitespace(); s1.scanNumbers();
    lhs = parseExp(s1);
    TokenScanner s2(rhsText.str()); s2.ignoreWhitespace(); s2.scanNumbers();
    rhs = parseExp(s2);
}

IfStatement::~IfStatement() {
    delete lhs;
    delete rhs;
}

void IfStatement::execute(EvalState &state, Program &program) {
    int lv = lhs->eval(state);
    int rv = rhs->eval(state);
    bool cond = false;
    if (op == "=") cond = (lv == rv);
    else if (op == "<") cond = (lv < rv);
    else if (op == ">") cond = (lv > rv);
    else if (op == "<=") cond = (lv <= rv);
    else if (op == ">=") cond = (lv >= rv);
    else if (op == "<>") cond = (lv != rv);
    if (cond) {
        if (program.getSourceLine(target).empty()) error("LINE NUMBER ERROR");
        program.requestJump(target);
    }
}

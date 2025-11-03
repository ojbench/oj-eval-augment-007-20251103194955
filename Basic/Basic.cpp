/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);
static void runProgram(Program &program, EvalState &state, int startLine);
static Statement* parseStatementByKeyword(const std::string &kw, TokenScanner &scanner);

/* Main program */

int main() {
    EvalState state;
    Program program;
    //cout << "Stub implementation of BASIC" << endl;
    while (true) {
        try {
            std::string input;
            getline(std::cin, input);
            if (input.empty())
                continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    if (!scanner.hasMoreTokens()) return;
    std::string first = scanner.nextToken();
    TokenType t = scanner.getTokenType(first);

    if (t == NUMBER) {
        int lineNumber = stringToInteger(first);
        if (!scanner.hasMoreTokens()) {
            program.removeSourceLine(lineNumber);
            return;
        }
        // store full source line as given
        program.addSourceLine(lineNumber, line);
        std::string kw = scanner.nextToken();
        if (scanner.getTokenType(kw) != WORD) error("SYNTAX ERROR");
        std::string up = toUpperCase(kw);
        Statement *stmt = parseStatementByKeyword(up, scanner);
        program.setParsedStatement(lineNumber, stmt);
        return;
    }

    if (t == WORD) {
        std::string cmd = toUpperCase(first);
        if (cmd == "LET") {
            LetStatement stmt(scanner);
            stmt.execute(state, program);
            return;
        } else if (cmd == "PRINT") {
            PrintStatement stmt(scanner);
            stmt.execute(state, program);
            return;
        } else if (cmd == "INPUT") {
            InputStatement stmt(scanner);
            stmt.execute(state, program);
            return;
        } else if (cmd == "LIST") {
            if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
            int cur = program.getFirstLineNumber();
            while (cur != -1) {
                std::cout << program.getSourceLine(cur) << std::endl;
                cur = program.getNextLineNumber(cur);
            }
            return;
        } else if (cmd == "CLEAR") {
            if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
            program.clear();
            state.Clear();
            return;
        } else if (cmd == "RUN") {
            if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
            int start = program.getFirstLineNumber();
            if (start != -1) runProgram(program, state, start);
            return;
        } else if (cmd == "GOTO") {
            // immediate GOTO: jump-run from target line
            std::string tkn = scanner.nextToken();
            if (tkn.empty() || scanner.getTokenType(tkn) != NUMBER || scanner.hasMoreTokens()) error("SYNTAX ERROR");
            int target = stringToInteger(tkn);
            if (program.getSourceLine(target).empty()) error("LINE NUMBER ERROR");
            runProgram(program, state, target);
            return;
        } else if (cmd == "IF") {
            // immediate conditional jump and run
            // Reuse IfStatement parsing then execute in a lightweight Program context
            IfStatement stmt(scanner);
            // Instead of storing jump, just evaluate and if true run from target by inspecting stmt via execute state
            // We emulate by calling execute which may set jump; then if jump set, run from that line
            stmt.execute(state, program);
            if (program.hasJump()) {
                int target = program.consumeJump();
                runProgram(program, state, target);
            }
            return;
        } else if (cmd == "REM") {
            // comment in immediate mode
            while (scanner.hasMoreTokens()) scanner.nextToken();
            return;
        } else if (cmd == "END") {
            if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
            // END in immediate mode: no effect
            return;
        } else if (cmd == "QUIT") {
            if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
            std::exit(0);
        }
        error("SYNTAX ERROR");
    }

    error("SYNTAX ERROR");
}

static Statement* parseStatementByKeyword(const std::string &kw, TokenScanner &scanner) {
    if (kw == "REM") return new RemStatement(scanner);
    if (kw == "LET") return new LetStatement(scanner);
    if (kw == "PRINT") return new PrintStatement(scanner);
    if (kw == "INPUT") return new InputStatement(scanner);
    if (kw == "END") return new EndStatement(scanner);
    if (kw == "GOTO") return new GotoStatement(scanner);
    if (kw == "IF") return new IfStatement(scanner);
    error("SYNTAX ERROR");
    return nullptr;
}

static void ensureParsedForLine(Program &program, int lineNumber) {
    if (program.getParsedStatement(lineNumber) != nullptr) return;
    std::string src = program.getSourceLine(lineNumber);
    TokenScanner s(src);
    s.ignoreWhitespace(); s.scanNumbers();
    // consume line number
    std::string lnTok = s.nextToken();
    if (s.getTokenType(lnTok) != NUMBER) error("SYNTAX ERROR");
    if (!s.hasMoreTokens()) error("SYNTAX ERROR");
    std::string kw = s.nextToken();
    if (s.getTokenType(kw) != WORD) error("SYNTAX ERROR");
    std::string up = toUpperCase(kw);
    Statement *stmt = parseStatementByKeyword(up, s);
    program.setParsedStatement(lineNumber, stmt);
}

static void runProgram(Program &program, EvalState &state, int startLine) {
    program.resetControl();
    int current = startLine;
    while (current != -1) {
        ensureParsedForLine(program, current);
        Statement *stmt = program.getParsedStatement(current);
        if (!stmt) error("SYNTAX ERROR");
        stmt->execute(state, program);
        if (program.stopRequested()) break;
        if (program.hasJump()) {
            int target = program.consumeJump();
            if (program.getSourceLine(target).empty()) error("LINE NUMBER ERROR");
            current = target;
        } else {
            current = program.getNextLineNumber(current);
        }
    }
}


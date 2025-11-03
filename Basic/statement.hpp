/*
 * File: statement.h
 * -----------------
 * This file defines the Statement abstract type.  In
 * the finished version, this file will also specify subclasses
 * for each of the statement types.  As you design your own
 * version of this class, you should pay careful attention to
 * the exp.h interface, which is an excellent model for
 * the Statement class hierarchy.
 */

#ifndef _statement_h
#define _statement_h

#include <string>
#include <sstream>
#include "evalstate.hpp"
#include "exp.hpp"
#include "Utils/tokenScanner.hpp"
#include "program.hpp"
#include "parser.hpp"
#include "Utils/error.hpp"
#include "Utils/strlib.hpp"

class Program;

/*
 * Class: Statement
 * ----------------
 * This class is used to represent a statement in a program.
 * The model for this class is Expression in the exp.h interface.
 * Like Expression, Statement is an abstract class with subclasses
 * for each of the statement and command types required for the
 * BASIC interpreter.
 */

class Statement {

public:

    Statement();
    virtual ~Statement();

    virtual void execute(EvalState &state, Program &program) = 0;
};

// REM
class RemStatement : public Statement {
public:
    explicit RemStatement(TokenScanner &scanner);
    void execute(EvalState &state, Program &program) override;
};

// LET
class LetStatement : public Statement {
public:
    explicit LetStatement(TokenScanner &scanner);
    ~LetStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    std::string var;
    Expression *rhs = nullptr;
};

// PRINT
class PrintStatement : public Statement {
public:
    explicit PrintStatement(TokenScanner &scanner);
    ~PrintStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *exp = nullptr;
};

// INPUT
class InputStatement : public Statement {
public:
    explicit InputStatement(TokenScanner &scanner);
    void execute(EvalState &state, Program &program) override;
private:
    std::string var;
};

// END
class EndStatement : public Statement {
public:
    explicit EndStatement(TokenScanner &scanner);
    void execute(EvalState &state, Program &program) override;
};

// GOTO
class GotoStatement : public Statement {
public:
    explicit GotoStatement(TokenScanner &scanner);
    void execute(EvalState &state, Program &program) override;
private:
    int target;
};

// IF ... THEN
class IfStatement : public Statement {
public:
    explicit IfStatement(TokenScanner &scanner);
    ~IfStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *lhs = nullptr;
    std::string op;
    Expression *rhs = nullptr;
    int target;
};

#endif

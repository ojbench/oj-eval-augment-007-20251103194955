/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"
#include "Utils/error.hpp"




Program::Program() = default;

Program::~Program() { clear(); }

void Program::clear() {
    for (auto &p : parsedStmts) {
        delete p.second;
    }
    parsedStmts.clear();
    sourceLines.clear();
    resetControl();
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    sourceLines[lineNumber] = line;
    auto it = parsedStmts.find(lineNumber);
    if (it != parsedStmts.end()) {
        delete it->second;
        parsedStmts.erase(it);
    }
}

void Program::removeSourceLine(int lineNumber) {
    auto itLine = sourceLines.find(lineNumber);
    if (itLine != sourceLines.end()) {
        sourceLines.erase(itLine);
    }
    auto it = parsedStmts.find(lineNumber);
    if (it != parsedStmts.end()) {
        delete it->second;
        parsedStmts.erase(it);
    }
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = sourceLines.find(lineNumber);
    if (it == sourceLines.end()) return "";
    return it->second;
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    if (sourceLines.find(lineNumber) == sourceLines.end()) {
        delete stmt;
        error("LINE NUMBER ERROR");
    }
    auto it = parsedStmts.find(lineNumber);
    if (it != parsedStmts.end()) {
        delete it->second;
        it->second = stmt;
    } else {
        parsedStmts.emplace(lineNumber, stmt);
    }
}

//void Program::removeSourceLine(int lineNumber) {

Statement *Program::getParsedStatement(int lineNumber) {
    auto it = parsedStmts.find(lineNumber);
    if (it == parsedStmts.end()) return nullptr;
    return it->second;
}

int Program::getFirstLineNumber() {
    if (sourceLines.empty()) return -1;
    return sourceLines.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = sourceLines.upper_bound(lineNumber);
    if (it == sourceLines.end()) return -1;
    return it->first;
}


// Control-flow helpers
void Program::requestJump(int lineNumber) {
    pendingJump = lineNumber;
}

bool Program::hasJump() const {
    return pendingJump != -2;
}

int Program::consumeJump() {
    int t = pendingJump;
    pendingJump = -2;
    return t;
}

void Program::requestStop() {
    stopFlag = true;
}

bool Program::stopRequested() const {
    return stopFlag;
}

void Program::resetControl() {
    pendingJump = -2;
    stopFlag = false;
}

//more func to add
//todo



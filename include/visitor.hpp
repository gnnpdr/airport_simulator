#pragma once
#include <cstdint>

class Cell;
class Passenger;
class RegOffice;
class Gate;
class Enterance;
class Field;

class Visitor 
{
public:
    virtual ~Visitor() = default;
    
    virtual void visit(Cell* cell) = 0;
    virtual void visit(Passenger* passenger) = 0;
    virtual void visit(RegOffice* office) = 0;
    virtual void visit(Gate* gate) = 0;
    virtual void visit(Enterance* enter) = 0;
    virtual void visit(Field* field) = 0;
};

class GameObject 
{
public:
    virtual ~GameObject() = default;
    virtual void accept(Visitor& visitor) = 0;
};
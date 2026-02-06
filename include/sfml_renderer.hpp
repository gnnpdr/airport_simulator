#pragma once
#include "visitor.hpp"
#include <SFML/Graphics.hpp>

class SFMLRenderVisitor : public Visitor 
{
private:

    sf::RenderWindow& window_;
    
public:

    explicit SFMLRenderVisitor(sf::RenderWindow& window);
    
    void visit(Cell* cell) override;
    void visit(Passenger* passenger) override;
    void visit(RegOffice* office) override;
    void visit(Gate* gate) override;
    void visit(Enterance* enter) override;
    void visit(Field* field) override;
    sf::Color get_passenger_color(Passenger* psg);
};
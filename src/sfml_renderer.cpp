#include "sfml_renderer.hpp"
#include "field.hpp"
#include "psg.hpp"

SFMLRenderVisitor::SFMLRenderVisitor(sf::RenderWindow& window) : window_(window) {}
    
void SFMLRenderVisitor::visit(Cell* cell)
{
    sf::RectangleShape rect(sf::Vector2f(40, 40));
    //cell->get_ind();
    rect.setPosition(cell->get_x() * 40.0f, cell->get_y() * 40.0f);
    rect.setFillColor(cell->is_passable() ? sf::Color::White : sf::Color::Black);
    window_.draw(rect);
}
    
void SFMLRenderVisitor::visit(Passenger* passenger)
{
    sf::CircleShape circle(15);
    circle.setPosition(passenger->get_x() * 40.0f + 5, passenger->get_y() * 40.0f + 5);
    circle.setFillColor(get_passenger_color(passenger));
    window_.draw(circle);
}


    
void SFMLRenderVisitor::visit(RegOffice* office)
{
    sf::RectangleShape rect(sf::Vector2f(40, 40));
    rect.setPosition(office->get_x() * 40.0f, office->get_y() * 40.0f);
    rect.setFillColor(!office->get_queue_len() ? sf::Color(255, 182, 193) : sf::Color::Red);
    window_.draw(rect);
}
    
void SFMLRenderVisitor::visit(Gate* gate)
{
    sf::RectangleShape rect(sf::Vector2f(40, 40));
    rect.setPosition(gate->get_x() * 40.0f, gate->get_y() * 40.0f);
    rect.setFillColor(sf::Color(173, 216, 230));
    window_.draw(rect);
}

void SFMLRenderVisitor::visit(Enterance* enter)
{
    sf::RectangleShape rect(sf::Vector2f(40, 40));
    rect.setPosition(enter->get_x() * 40.0f, enter->get_y() * 40.0f);
    rect.setFillColor(sf::Color(152, 251, 152));
    window_.draw(rect);
}

void SFMLRenderVisitor::visit(Field* field) 
{
    for (size_t ind = 0; ind < field->get_heig() * field->get_wid(); ind++) 
    {
        Cell* cell = field->get_cell_by_ind(ind);
        
        cell->accept(*this);
        if (!cell->is_psg_nullptr()) 
        {
            Passenger* passenger = cell->get_psg();
            passenger->accept(*this);
        }
    }
}

sf::Color SFMLRenderVisitor::get_passenger_color(Passenger* psg)
{
    switch(psg->get_status())
    {
        case WAITING:
            return sf::Color::Red;
        case NULLPTR:
            return sf::Color::Green;
        case GOING_TO_REG:
        case FIND_REG:
            return sf::Color::Magenta;
        case GOING_TO_GATE:
            return sf::Color::Blue;
        case REG:
            return sf::Color::Yellow;
    }

    return sf::Color::Black;
}
#include <SFML/Graphics.hpp>
#include "field.hpp"
#include "psg.hpp"
#include "sfml_renderer.hpp"

int main()
{
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Airport Simulation");
    window.setFramerateLimit(60);

    std::vector<std::pair<size_t, size_t>> obstacles = {{0, 0}, {5, 5}};
    std::vector<std::pair<size_t, size_t>> reg_offices = {{2, 3}, {4, 3}};
    std::vector<std::pair<size_t, size_t>> gates = {{5, 0}, {5, 3}};
    std::vector<std::pair<size_t, size_t>> enters = {{1, 0}, {3, 0}};

    Field field(6, 6, obstacles, reg_offices, enters, gates);

    SFMLRenderVisitor render_visitor(window);

    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event)) 
        {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (event.type == sf::Event::KeyPressed) 
            {
                if (event.key.code == sf::Keyboard::Space)
                    window.close();
            }
            
        }
        
        // field.update(16.67f);  // 60 FPS = 16.67ms на кадр
        
        window.clear(sf::Color(50, 50, 50)); 
        
        field.accept(render_visitor);
        window.display();
        
        // std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    //Passenger p1(field);
    //std::cout << "p coords " << p1.get_x() << ", " << p1.get_y() << std::endl;
    //p1.start_algo();
    //p1.end_algo();

    return 0;
}
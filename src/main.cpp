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

    sf::Clock frame_clock;
    sf::Clock update_clock; 
    sf::Time time_since_last_update = sf::Time::Zero;

    const sf::Time time_per_update = sf::seconds(1.5f / 1.0f);
    const float simulation_speed = 1.0f; 

    std::vector<std::pair<size_t, size_t>> obstacles = {{0, 0}, {5, 5}};
    std::vector<std::pair<size_t, size_t>> reg_offices = {{2, 3}, {4, 3}};
    std::vector<std::pair<size_t, size_t>> gates = {{5, 0}, {5, 3}};
    std::vector<std::pair<size_t, size_t>> enters = {{1, 0}, {3, 0}};

    Field field(6, 6, obstacles, reg_offices, enters, gates);

    SFMLRenderVisitor render_visitor(window);
    
    Passenger& p1 = field.add_passenger();
    
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

        time_since_last_update += update_clock.restart();
        
        //
        while (time_since_last_update > time_per_update) 
        {
            time_since_last_update -= time_per_update;
            
            field.update(time_per_update.asSeconds() * simulation_speed);

            printf("here\n");
            //int a = 0;
            //scanf("%d", &a);
        }
        
        window.clear(sf::Color(50, 50, 50)); 
        field.accept(render_visitor);
        window.display();
    }

    //
    //std::cout << "p coords " << p1.get_x() << ", " << p1.get_y() << std::endl;
    //p1.start_algo();
    //p1.end_algo();

    return 0;
}
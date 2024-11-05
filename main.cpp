#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <string.h>



#define WIDTH 1240
#define HEIGHT 1200
#define GRID 20

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define NUM_OF_BLOCK 20


float distance(sf::Vector2f p1, sf::Vector2f p2)
{
    return std::pow(std::pow(p1.x-p2.x,2)+std::pow(p1.y-p2.y,2), 0.5);
}



int randomInt(int min, int max)
{
    return rand()%(max-min + 1) + min;
}



bool checkCollision(sf::Vector2f blockPosition, sf::Vector2f blockSize)
{
    float x1 = blockPosition.x;
    float x2 = blockPosition.x + blockSize.x;
    float y1 = blockPosition.y;
    float y2 = blockPosition.y + blockSize.y;

    sf::Vector2f center = sf::Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    float radius = 10;

    float dx1 = std::abs(x1 - WINDOW_WIDTH/2); //(center of player)
    float dx2 = std::abs(x2 - WINDOW_WIDTH/2);
    float dy1 = std::abs(y1 - WINDOW_HEIGHT/2);
    float dy2 = std::abs(y2 - WINDOW_HEIGHT/2);

    if
    (
        ( center.x <= x1 - radius && center.y <= y1 - radius ) ||
        ( center.x <= x1 - radius && center.y >= y2 + radius ) ||
        ( center.x >= x2 + radius && center.y >= y2 + radius ) ||
        ( center.x >= x2 + radius && center.y <= y1 - radius )
    )
    {
        return false;
    }

    if
    (
        ( (x1 < center.x && center.x < x2) && (dy1 < radius || dy2 < radius) )
        ||
        ( (y1 < center.y && center.y < y2) && (dx1 < radius || dx2 < radius) )
    )
    {
        return true;
    }

    if (
        ( ( x1-radius <= center.x && center.x <= x1 && y1-radius <= center.y && center.y <= y1) && ( distance(sf::Vector2f(x1,y1),center) < radius ) )
        ||
        ( ( x2 <= center.x && center.x <= x2+radius && y1-radius <= center.y && center.y <= y1) && ( distance(sf::Vector2f(x2,y1),center) < radius ) )
        ||
        ( ( x2 <= center.x && center.x <= x2+radius && y2 <= center.y && center.y <= y2+radius) && ( distance(sf::Vector2f(x2,y2),center) < radius ) )
        ||
        ( ( x1-radius <= center.x && center.x <= x1 && y2 <= center.y && center.y <= y2+radius) && ( distance(sf::Vector2f(x1,y2),center) < radius ) )
       )
    {
        return true;
    }

    return false;
}



class Grid
{
private:

    sf::Vector2f origin;
    sf::Vertex verticalLines[WIDTH/GRID-1][2];
    sf::Vertex horizontalLines[HEIGHT/GRID-1][2];

public:

    void makeGrid( sf::Vector2f origin )
    {
        this->origin = origin;

        for(short x=1; x<WIDTH/GRID; x++)
        {
            verticalLines[x-1][0].position = origin + sf::Vector2f(x*GRID, 0);
            verticalLines[x-1][0].color = sf::Color::Red;

            verticalLines[x-1][1].position = origin + sf::Vector2f(x*GRID, HEIGHT);
            verticalLines[x-1][1].color = sf::Color::Red;
        }

        for(short y=1; y<HEIGHT/GRID; y++)
        {
            horizontalLines[y-1][0].position = origin + sf::Vector2f(0, y*GRID);
            horizontalLines[y-1][0].color = sf::Color::Red;

            horizontalLines[y-1][1].position = origin + sf::Vector2f(WIDTH, y*GRID);
            horizontalLines[y-1][1].color = sf::Color::Red;
        }
    }



    void drawOn(sf::RenderWindow &window)
    {
        for(short x=1; x<WIDTH/GRID; x++)
        {
            window.draw(verticalLines[x-1],2,sf::Lines);
        }

        for(short y=1; y<HEIGHT/GRID; y++)
        {
            window.draw(horizontalLines[y-1],2,sf::Lines);
        }
    }
};



class Block: public sf::RectangleShape
{
public:

    sf::Vector2f relativePosition;



    Block()
    {
        setFillColor(sf::Color(0,0,255));
    }



    void makeBlock(sf::Vector2f blockSize, sf::Vector2f relativePosition, sf::Vector2f origin)
    {
        this->relativePosition = relativePosition;

        setSize( blockSize );
        setPosition( origin.x + relativePosition.x, origin.y + relativePosition.y);
    }



    void updateBlock(sf::Vector2f origin)
    {
        setPosition( origin.x + relativePosition.x , origin.y + relativePosition.y );
    }
};



class Player: public sf::CircleShape
{
public:

    sf::Vector2i relativePosition;

    void setRelativePosition(sf::Vector2f p)
    {
        relativePosition.x = (int) p.x;
        relativePosition.y = (int) p.y;
    }

    sf::Vector2f getRelativePosition()
    {
        return sf::Vector2f(relativePosition.x,relativePosition.y);
    }

    Player()
    {
        setRadius( 10 );
        setFillColor( sf::Color(255,255,0) );
        setPosition( WINDOW_WIDTH/2-getRadius(), WINDOW_HEIGHT/2-getRadius() );
    }
};



class World : public sf::RectangleShape
{
private:

        sf::Vector2f pastPosition;
        sf::Vector2f playerPastRelativePosition;

public:

    Grid grid;
    Block blocks[NUM_OF_BLOCK];
    Player player;
    Player player2;


    World()
    {
        setSize(sf::Vector2f(WIDTH, HEIGHT));
        setFillColor(sf::Color(255,127,7));

        grid.makeGrid(getPosition());

        player.setRelativePosition( player.getPosition() - getPosition() );

        player2.setFillColor(sf::Color::Green);

        for(short i=0; i<NUM_OF_BLOCK; i++)
        {
            int blockLengthInGrid = randomInt(4,12);
            sf::Vector2i blockSizeInGrid;
            sf::Vector2i blockPositionInGrid;

            while(true)
            {
                if (randomInt(0,1))
                {
                    blockSizeInGrid.x = blockLengthInGrid;
                    blockSizeInGrid.y = 1;

                    blockPositionInGrid.x = randomInt(1, WIDTH/GRID - blockLengthInGrid - 1);
                    blockPositionInGrid.y = randomInt(1, HEIGHT/GRID - 1 - 1);
                }
                else
                {
                    blockSizeInGrid.x = 1;
                    blockSizeInGrid.y = blockLengthInGrid;

                    blockPositionInGrid.x = randomInt(0, WIDTH/GRID - 1 -1);
                    blockPositionInGrid.y = randomInt(0, HEIGHT/GRID - blockLengthInGrid -1);
                }

                //EXTRA //CHECK

                sf::Vector2f blockAbsolutePosition = getPosition() + sf::Vector2f(blockPositionInGrid.x*GRID, blockPositionInGrid.y*GRID);
                sf::Vector2f blockSize = sf::Vector2f(blockSizeInGrid.x*GRID, blockSizeInGrid.y*GRID);

                sf::Vector2f center = sf::Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
                float radius = 10;

                float x1 = blockAbsolutePosition.x;
                float x2 = blockAbsolutePosition.x + blockSize.x;
                float y1 = blockAbsolutePosition.y;
                float y2 = blockAbsolutePosition.y + blockSize.y;

                if( !( center.x >= x1-radius  && center.x <= x2+radius && center.y >= y1-radius && center.y <= y2+radius ) )
                {
                    break;
                }
            }

            blocks[i].makeBlock( sf::Vector2f(blockSizeInGrid.x*GRID, blockSizeInGrid.y*GRID) , sf::Vector2f(blockPositionInGrid.x*GRID, blockPositionInGrid.y*GRID) - getPosition(), getPosition());
        }
    }


    void update(sf::Vector2f d)
    {

        pastPosition = getPosition();
        playerPastRelativePosition = player.getRelativePosition();




        setPosition( getPosition() - d );
        grid.makeGrid( getPosition() );

        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            blocks[i].updateBlock(getPosition());
        }

        player.setRelativePosition( player.getRelativePosition() + d );
    }


    void undo()
    {
        setPosition( pastPosition );
        grid.makeGrid( pastPosition );

        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            blocks[i].updateBlock( pastPosition );
        }

        player.setRelativePosition( playerPastRelativePosition );
    }


    void drawOn(sf::RenderWindow &window)
    {
        window.draw(*(this));

        grid.drawOn(window);

        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            window.draw(blocks[i]);
        }

        window.draw(player);
        window.draw(player2);
    }
};



class AimLine
{
public:

    sf::Vertex line[2];


    AimLine(sf::WindowBase &window)
    {
        line[0].position = sf::Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
        line[0].color = sf::Color::Green;

        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        line[1].position = sf::Vector2f(mousePosition.x, mousePosition.y);
        line[1].color = sf::Color::Green;
    }



    void updateLine(sf::WindowBase &window, World &world)
    {
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        line[1].position = sf::Vector2f(mousePosition.x, mousePosition.y);

        float min_distance = 1000;

        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            sf::Vector2f blockPosition = world.blocks[i].getPosition();
            sf::Vector2f blockSize = world.blocks[i].getSize();

            float x1 = blockPosition.x;
            float x2 = blockPosition.x + blockSize.x;
            float y1 = blockPosition.y;
            float y2 = blockPosition.y + blockSize.y;


            if (line[0].position.y < y1 && line[1].position.y > y1)
            {
                float dx = (y1-line[0].position.y)*(line[1].position.x-line[0].position.x)/(line[1].position.y-line[0].position.y);

                if ( x1 < line[0].position.x + dx && line[0].position.x + dx < x2 )
                {
                    sf::Vector2f line1;
                    line1.x = line[0].position.x + dx;
                    line1.y = y1;

                    if (distance(line[0].position, line1) < min_distance)
                    {
                        line[1].position = line1;

                        min_distance = distance(line[0].position, line[1].position);
                    }
                }
            }

            if (line[0].position.y > y2 && line[1].position.y < y2)
            {
                float dx = (y2-line[0].position.y)*(line[1].position.x-line[0].position.x)/(line[1].position.y-line[0].position.y);

                if ( x1 < line[0].position.x + dx && line[0].position.x + dx < x2 )
                {
                    sf::Vector2f line1;
                    line1.x = line[0].position.x + dx;
                    line1.y = y2;
                    if (distance(line[0].position, line1) < min_distance)
                    {
                        line[1].position = line1;

                        min_distance = distance(line[0].position, line[1].position);
                    }
                }
            }


            if (line[0].position.x < x1 && line[1].position.x > x1)
            {
                float dy = (x1-line[0].position.x)*(line[1].position.y-line[0].position.y)/(line[1].position.x-line[0].position.x);

                if ( y1 < line[0].position.y + dy && line[0].position.y + dy < y2 )
                {
                    sf::Vector2f line1;
                    line1.y = line[0].position.y + dy;
                    line1.x = x1;
                    if (distance(line[0].position, line1) < min_distance)
                    {
                        line[1].position = line1;

                        min_distance = distance(line[0].position, line[1].position);
                    }
                }
            }

            if (line[0].position.x>x2 && line[1].position.x<x2)
            {
                float dy = (x2-line[0].position.x)*(line[1].position.y-line[0].position.y)/(line[1].position.x-line[0].position.x);

                if (  y1 < line[0].position.y + dy && line[0].position.y + dy <y2 )
                {
                    sf::Vector2f line1;
                    line1.y= line[0].position.y + dy;
                    line1.x = x2;
                    if (distance(line[0].position, line1) < min_distance)
                    {
                        line[1].position = line1;

                        min_distance = distance(line[0].position, line[1].position);
                    }
                }
            }
        }
    }

    void drawOn(sf::RenderWindow &window)
    {
        window.draw(line,2,sf::Lines);
    }
};



class MiniWorld: public sf::RectangleShape
{
private:
    sf::Vector2f pastPosition;

public:

    float width;
    float height;

    float scale_factor;
    Block miniBlocks[NUM_OF_BLOCK];
    sf::CircleShape miniPlayer;
    sf::CircleShape miniPlayer2;

    MiniWorld(World world)
    {
        this->height = (float) WINDOW_HEIGHT/4;
        this->scale_factor = height/HEIGHT;
        this->width = WIDTH*scale_factor;

        setFillColor(world.getFillColor());
        setPosition(WINDOW_WIDTH-width-50,50);
        setSize(sf::Vector2f(width,height));
        setOutlineColor(sf::Color::Black);
        setOutlineThickness(1);

        miniPlayer.setRadius(world.player.getRadius()*scale_factor);
        miniPlayer.setFillColor(world.player.getFillColor());

        miniPlayer2.setRadius(world.player2.getRadius()*scale_factor);
        miniPlayer2.setFillColor(world.player2.getFillColor());

        sf::Vector2f positionOfMiniPlayerRelativeToMiniWorld = sf::Vector2f(world.player.relativePosition.x * scale_factor, world.player.relativePosition.y * scale_factor);
        sf::Vector2f positionOfMiniPlayer2RelativeToMiniWorld = sf::Vector2f(world.player2.relativePosition.x * scale_factor, world.player2.relativePosition.y * scale_factor);

        miniPlayer.setPosition(getPosition() + positionOfMiniPlayerRelativeToMiniWorld);
        miniPlayer2.setPosition(getPosition() + positionOfMiniPlayer2RelativeToMiniWorld);

        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            miniBlocks[i].setSize(sf::Vector2f(world.blocks[i].getSize().x*scale_factor, world.blocks[i].getSize().y*scale_factor));

            sf::Vector2f relativePositionOfBlock = world.blocks[i].getPosition() - world.getPosition();
            sf::Vector2f positionOfMiniBlockRelativeToMiniWorld = sf::Vector2f(relativePositionOfBlock.x * scale_factor, relativePositionOfBlock.y * scale_factor);

            miniBlocks[i].setPosition(getPosition() + positionOfMiniBlockRelativeToMiniWorld);
        }
    }

    void update(sf::Vector2f d)
    {
        pastPosition = miniPlayer.getPosition();
        miniPlayer.setPosition(miniPlayer.getPosition() + sf::Vector2f(d.x*scale_factor, d.y*scale_factor));
    }

    void undo()
    {
        miniPlayer.setPosition(pastPosition);
    }

    void drawOn(sf::RenderWindow &window)
    {
        window.draw(*(this));

        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            window.draw(miniBlocks[i]);
        }

        window.draw(miniPlayer);
        window.draw(miniPlayer2);
    }
};



typedef struct x
{
    int id;
    int x;
    int y;
} Position;



bool parse(char buffer[31], Position *p1, Position *p2)
{
    int temp;

    int index=1;

    int sum;

    if (buffer[0]!=';' )
    {
        p1->id = buffer[0] - '0';

        while(buffer[index]!=',')
        {
            index++;
        }
        sum = 0;
        for (int i=index-1; i>=1; i--)
        {
            int digit = buffer[i] - '0';

            for (int j=0; j<index-i-1; j++)
            {
                digit*=10;
            }

            sum+=digit;
        }
        p1->x = sum;

        index+=1;
        temp = index;
        while(buffer[index]!='_')
        {
            index++;
        }
        sum = 0;
        for (int i=index-1; i>=temp; i--)
        {
            int digit = buffer[i] - '0';

            for (int j=0; j<index-i-1; j++)
            {
                digit*=10;
            }

            sum+=digit;
        }
        p1->y = sum;

        while(buffer[index]=='_')
        {
            index++;
        }


        printf("%c", buffer[index]);

        if (buffer[index]!=';')
        {
            p2->id = buffer[index] - '0';

            index+=1;
            temp = index;
            while(buffer[index]!=',')
            {
                index++;
            }
            sum = 0;
            for (int i=index-1; i>=temp; i--)
            {
                int digit = buffer[i] - '0';

                for (int j=0; j<index-i-1; j++)
                {
                    digit*=10;
                }

                sum+=digit;
            }
            p2->x = sum;

            index+=1;
            temp=index;
            while(buffer[index]!='_')
            {
                index++;
            }
            sum = 0;
            for (int i=index-1; i>=temp; i--)
            {
                int digit = buffer[i] - '0';

                for (int j=0; j<index-i-1; j++)
                {
                    digit*=10;
                }

                sum+=digit;
            }
            p2->y = sum;

            return true;
        }
    }
    return false;
}



Position position[2];

World world;

MiniWorld miniWorld(world);



void update(World *world, MiniWorld *miniWorld, Position p)
{
    world->player2.setPosition( sf::Vector2f(p.x, p.y) + world->getPosition() );

    float scale_factor = miniWorld->scale_factor;
    miniWorld->miniPlayer2.setPosition( miniWorld->getPosition() + sf::Vector2f(p.x * scale_factor, p.y * scale_factor) );
}

int id=1;

void receive(sf::TcpSocket *socket)
{
    std::size_t received = 0;

    while(true)
    {
        char buffer[31];
        socket->receive(buffer, sizeof(buffer), received);

        if(received>0)
        {
            std::cout << "The client said: " << buffer << std::endl;

            if (parse(buffer, &position[0], &position[1]))
            {
                for( int i=0; i<2; i++)
                {
                    if (position[i].id != id)
                    {
                        update(&world, &miniWorld, position[i]);
                    }
                }
            }
        }
    }
}


int main()
{
srand(static_cast <unsigned int> (time(0)));

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "ShooterGame");


    AimLine aimLine(window);

    /// SOCKET
    sf::TcpSocket socket;
    socket.connect("10.18.119.86", 8080);

    sf::TcpSocket* socketPtr = &socket;
    sf::Thread th(&receive, socketPtr);
    th.launch();


    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if(event.type == sf::Event::KeyPressed)
            {
                bool collision = false;

                if(event.key.code == sf::Keyboard::Left)
                {
                    world.update( sf::Vector2f(-10, 0) );
                    miniWorld.update( sf::Vector2f(-10, 0) );

                    for (short i=0; i<NUM_OF_BLOCK; i++)
                    {
                        sf::Vector2f blockAbsolutePosition = world.blocks[i].getPosition();
                        sf::Vector2f blockSize = world.blocks[i].getSize();

                        if (checkCollision(blockAbsolutePosition, blockSize))
                        {   collision = true;
                            world.update( sf::Vector2f(10, 0) );
                            miniWorld.update( sf::Vector2f(10, 0) );
                            break;
                        }
                    }
                }

                if(event.key.code == sf::Keyboard::Right)
                {
                    world.update( sf::Vector2f(10, 0) );
                    miniWorld.update( sf::Vector2f(10, 0) );

                    for (short i=0; i<NUM_OF_BLOCK; i++)
                    {
                        sf::Vector2f blockAbsolutePosition = world.blocks[i].getPosition();
                        sf::Vector2f blockSize = world.blocks[i].getSize();

                        if (checkCollision(blockAbsolutePosition, blockSize))
                        {   collision = true;
                            world.update( sf::Vector2f(-10, 0) );
                            miniWorld.update( sf::Vector2f(-10, 0) );
                            break;
                        }
                    }
                }

                if(event.key.code == sf::Keyboard::Up)
                {
                    world.update( sf::Vector2f(0, -10) );
                    miniWorld.update( sf::Vector2f(0, -10) );

                    for (short i=0; i<NUM_OF_BLOCK; i++)
                    {
                        sf::Vector2f blockAbsolutePosition = world.blocks[i].getPosition();
                        sf::Vector2f blockSize = world.blocks[i].getSize();

                        if (checkCollision(blockAbsolutePosition, blockSize))
                        {   collision = true;
                            world.update( sf::Vector2f(0, 10) );
                            miniWorld.update( sf::Vector2f(0, 10) );
                            break;
                        }
                    }
                }

                if(event.key.code == sf::Keyboard::Down)
                {
                    world.update( sf::Vector2f(0, 10) );
                    miniWorld.update( sf::Vector2f(0, 10) );

                    for (short i=0; i<NUM_OF_BLOCK; i++)
                    {
                        sf::Vector2f blockAbsolutePosition = world.blocks[i].getPosition();
                        sf::Vector2f blockSize = world.blocks[i].getSize();

                        if (checkCollision(blockAbsolutePosition, blockSize))
                        {   collision = true;
                            world.update( sf::Vector2f(0, -10) );
                            miniWorld.update( sf::Vector2f(0, -10) );
                            break;
                        }
                    }
                }



                // SENDING MESSAGE
                std::string s = std::to_string(id);
                s += "S";
                s += std::to_string(world.player.relativePosition.x);
                s += ",";
                s += std::to_string(world.player.relativePosition.y);
                s += "E";
                socket.send(s.c_str(), s.size() + 1);

            }

            aimLine.updateLine(window, world);

            /// DO YOUR WORK HERE ///



        }


        window.clear(sf::Color(50,50,50));
        world.drawOn(window);
        aimLine.drawOn(window);
        miniWorld.drawOn(window);
        window.display();
    }

    socket.disconnect();

    return 0;
}

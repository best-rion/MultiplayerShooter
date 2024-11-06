#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <cstdlib>
#include <time.h>
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

#define NUM_OF_BLOCK 40
#define NUM_OF_BULLET 10
#define ID 0

int id = ID;
int v = 10;
float bulletVelocity = 0.05;


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
            verticalLines[x-1][0].color = sf::Color::Black;

            verticalLines[x-1][1].position = origin + sf::Vector2f(x*GRID, HEIGHT);
            verticalLines[x-1][1].color = sf::Color::Black;
        }

        for(short y=1; y<HEIGHT/GRID; y++)
        {
            horizontalLines[y-1][0].position = origin + sf::Vector2f(0, y*GRID);
            horizontalLines[y-1][0].color = sf::Color::Black;

            horizontalLines[y-1][1].position = origin + sf::Vector2f(WIDTH, y*GRID);
            horizontalLines[y-1][1].color = sf::Color::Black;
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
        setFillColor(sf::Color(21, 97, 200));
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



class Bullet: public sf::CircleShape
{
public:

    float angle;
    float r=4;

    Bullet()
    {
        setFillColor(sf::Color::Red);
        setRadius(4);
    }


    void fly()
    {
        float dx = bulletVelocity * std::cos(3.1416/180*angle);
        float dy = bulletVelocity * std::sin(3.1416/180*angle);
        setPosition(getPosition()+sf::Vector2f(dx,dy));
    }
};



class Player: public sf::CircleShape
{
public:

    sf::Vector2i relativePosition;
    sf::RectangleShape gun;

    Bullet bullets[NUM_OF_BULLET];
    int bulletCount=0;
    int bulletIndex=0;


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
        setFillColor( sf::Color::Yellow );
        setPosition( WINDOW_WIDTH/2-getRadius(), WINDOW_HEIGHT/2-getRadius() );



        relativePosition.x = WINDOW_WIDTH/2-10;//radius
        relativePosition.y = WINDOW_HEIGHT/2-10;

        gun.setFillColor(sf::Color::Black);
        gun.setSize(sf::Vector2f(16,4));
        gun.setPosition(WINDOW_WIDTH/2, WINDOW_HEIGHT/2-2);
        gun.setOrigin( sf::Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2) - gun.getPosition());
    }

    void drawOn(sf::RenderWindow &window)
    {
        window.draw(*(this));
        window.draw(gun);
    }
};



class World : public sf::RectangleShape
{

public:

    Grid grid;
    Block blocks[NUM_OF_BLOCK];
    Player player[2];


    World()
    {
        setSize(sf::Vector2f(WIDTH, HEIGHT));
        setFillColor(sf::Color(232, 140, 50));

        grid.makeGrid(getPosition());

        player[ID].setRelativePosition( player[ID].getPosition() - getPosition() );

        player[1].setFillColor(sf::Color::Green);

        srand(42);
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
                    blockSizeInGrid.y = 2;

                    blockPositionInGrid.x = randomInt(1, WIDTH/GRID - blockLengthInGrid - 1);
                    blockPositionInGrid.y = randomInt(1, HEIGHT/GRID - 1 - 1);
                }
                else
                {
                    blockSizeInGrid.x = 2;
                    blockSizeInGrid.y = blockLengthInGrid;

                    blockPositionInGrid.x = randomInt(0, WIDTH/GRID - 1 -1);
                    blockPositionInGrid.y = randomInt(0, HEIGHT/GRID - blockLengthInGrid -1);
                }

                //EXTRA //CHECK

                sf::Vector2f blockAbsolutePosition = getPosition() + sf::Vector2f(blockPositionInGrid.x*GRID, blockPositionInGrid.y*GRID);
                sf::Vector2f blockSize = sf::Vector2f(blockSizeInGrid.x*GRID, blockSizeInGrid.y*GRID);

                sf::Vector2f center = sf::Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
                float radius = 20;

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
        setPosition( getPosition() - d );
        grid.makeGrid( getPosition() );

        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            blocks[i].updateBlock(getPosition());
        }

        player[ID].setRelativePosition( player[ID].getRelativePosition() + d );

        player[1].setPosition( player[1].getRelativePosition() + getPosition() );
        player[1].gun.setPosition( player[1].getPosition() + sf::Vector2f(10,10-2) );
    }


    void drawOn(sf::RenderWindow &window)
    {
        window.draw(*(this));

        grid.drawOn(window);

        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            window.draw(blocks[i]);
        }

        player[1].drawOn(window);
        player[0].drawOn(window);
    }
};



class AimLine
{
public:

    sf::Vertex line[2];


    AimLine(sf::WindowBase &window)
    {
        line[0].position = sf::Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
        line[0].color = sf::Color::Yellow;

        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        line[1].position = sf::Vector2f(mousePosition.x, mousePosition.y);
        line[1].color = sf::Color::Yellow;
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


        float angle = 180/3.1416*std::atan2(mousePosition.y-WINDOW_HEIGHT/2,mousePosition.x-WINDOW_WIDTH/2);

        world.player[ID].gun.rotate( angle -world.player[ID].gun.getRotation());



    }

    void drawOn(sf::RenderWindow &window)
    {
        window.draw(line,2,sf::Lines);
    }
};



class MiniWorld: public sf::RectangleShape
{
public:

    float width;
    float height;

    float scale_factor;
    Block miniBlocks[NUM_OF_BLOCK];
    sf::CircleShape miniPlayer[2];

    MiniWorld(World world)
    {
        this->height = (float) WINDOW_HEIGHT/4;
        this->scale_factor = height/HEIGHT;
        this->width = WIDTH*scale_factor;

        setFillColor(sf::Color(230, 130, 40));
        setPosition(WINDOW_WIDTH-width-50,50);
        setSize(sf::Vector2f(width,height));
        setOutlineColor(sf::Color::Black);
        setOutlineThickness(1);

        miniPlayer[ID].setRadius(world.player[ID].getRadius()*scale_factor);
        miniPlayer[ID].setFillColor(world.player[ID].getFillColor());

        miniPlayer[1].setRadius(world.player[1].getRadius()*scale_factor);
        miniPlayer[1].setFillColor(world.player[1].getFillColor());



        sf::Vector2f positionOfMiniPlayerRelativeToMiniWorld[2];

        positionOfMiniPlayerRelativeToMiniWorld[ID] = sf::Vector2f(world.player[ID].relativePosition.x * scale_factor, world.player[ID].relativePosition.y * scale_factor);
        positionOfMiniPlayerRelativeToMiniWorld[1] = sf::Vector2f(world.player[1].relativePosition.x * scale_factor, world.player[1].relativePosition.y * scale_factor);

        miniPlayer[ID].setPosition(getPosition() + positionOfMiniPlayerRelativeToMiniWorld[ID]);
        miniPlayer[1].setPosition(getPosition() + positionOfMiniPlayerRelativeToMiniWorld[1]);

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
        miniPlayer[ID].setPosition(miniPlayer[ID].getPosition() + sf::Vector2f(d.x*scale_factor, d.y*scale_factor));
    }


    void drawOn(sf::RenderWindow &window)
    {
        window.draw(*(this));

        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            window.draw(miniBlocks[i]);
        }

        window.draw(miniPlayer[ID]);
        window.draw(miniPlayer[1]);
    }
};



typedef struct
{
    int id;
    int x;
    int y;
    float gunAngle;
    bool click;

} PlayerInfo;




bool parse(char buffer[36], PlayerInfo* p1, PlayerInfo* p2)
{
    int temp;

    int index=0;

    int sum;

    if (buffer[index]!=';' )
    {

        p1->id = buffer[index] - '0';

        index+=1;
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
        p1->y = sum;

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
        p1->gunAngle = sum;

        index+=1;
        if(buffer[index] == '1')
        {
            p1->click = true;
        }
        index+=1;

        while(buffer[index]=='_')
        {
            index++;
        }


        printf("%c", buffer[index]);
    }

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
        p2->y = sum;


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
        p2->gunAngle = sum;

        index+=1;
        if(buffer[index] == '1')
        {
            p2->click = true;
        }

        return true;
    }
    return false;
}



PlayerInfo playerInfo[2];

World world;

MiniWorld miniWorld(world);



void update(World *world, MiniWorld *miniWorld, PlayerInfo p)
{
    for (int i=0; i<2; i++)
    {
        if (i != id)
        {
            world->player[1].setPosition( sf::Vector2f(p.x, p.y) + world->getPosition() );

            float scale_factor = miniWorld->scale_factor;
            miniWorld->miniPlayer[1].setPosition( miniWorld->getPosition() + sf::Vector2f(p.x * scale_factor, p.y * scale_factor) );

            sf::Vector2f player2Position = world->player[1].getPosition();
            world->player[1].gun.setPosition(player2Position.x+10,player2Position.y+10);
            world->player[1].gun.rotate(p.gunAngle-world->player[1].gun.getRotation());

            // Shooting bullets (RESTRUCTURE THE CODE)




        }
    }
}


void receive(sf::TcpSocket *socket)
{
    while (true)
    {
        char buffer[36];
        std::size_t received = 0;
        socket->receive(buffer, sizeof(buffer), received);

        if(received>0)
        {
            std::cout << "The server said: " << buffer << std::endl;

            if (parse(buffer, &playerInfo[0], &playerInfo[1]))
            {
                for( int i=0; i<2; i++)
                {
                    if (playerInfo[i].id != id)
                    {
                        update(&world, &miniWorld, playerInfo[i]);
                    }
                }
            }
        }
    }
}


int main()
{

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
            int clickCount = 0;

            if (event.type == sf::Event::Closed)
                window.close();


            if (event.type == sf::Event::MouseButtonPressed)
            {
                clickCount++;

                int indx;
                indx = world.player[ID].bulletIndex + world.player[ID].bulletCount;
                indx = (indx >= NUM_OF_BULLET) ? indx - NUM_OF_BULLET : indx ;

                sf::Vector2i m = sf::Mouse::getPosition(window);

                float angle = 180 / 3.1416 * std::atan2(WINDOW_HEIGHT/2-m.y,WINDOW_WIDTH/2-m.x) + 180;

                world.player[ID].bullets[indx].angle = angle;
                float startX = WINDOW_WIDTH/2 + 10 * std::cos(3.1416/180*angle);
                float startY = WINDOW_HEIGHT/2 + 10 * std::sin(3.1416/180*angle);
                world.player[ID].bullets[indx].setPosition(sf::Vector2f(startX,startY));

                if(world.player[ID].bulletCount < NUM_OF_BULLET)
                {
                    world.player[ID].bulletCount++;
                }
            }


            if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Left)
                {
                    world.update( sf::Vector2f(-v, 0) );
                    miniWorld.update( sf::Vector2f(-v, 0) );

                    for (short i=0; i<NUM_OF_BLOCK; i++)
                    {
                        sf::Vector2f blockAbsolutePosition = world.blocks[i].getPosition();
                        sf::Vector2f blockSize = world.blocks[i].getSize();

                        if (checkCollision(blockAbsolutePosition, blockSize))
                        {
                            world.update( sf::Vector2f(v, 0) );
                            miniWorld.update( sf::Vector2f(v, 0) );
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
                        {
                            world.update( sf::Vector2f(-v, 0) );
                            miniWorld.update( sf::Vector2f(-v, 0) );
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
                        {
                            world.update( sf::Vector2f(0, v) );
                            miniWorld.update( sf::Vector2f(0, v) );
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
                        {
                            world.update( sf::Vector2f(0, -v) );
                            miniWorld.update( sf::Vector2f(0, -v) );
                            break;
                        }
                    }
                }
            }

            /// DO YOUR WORK HERE ///


            aimLine.updateLine(window, world);


            // SENDING MESSAGE
            std::string s = std::to_string(id);
            s += "S";
            s += std::to_string(world.player[ID].relativePosition.x);
            s += ",";
            s += std::to_string(world.player[ID].relativePosition.y);
            s += ",";

            sf::Vector2i m = sf::Mouse::getPosition(window);

            s += std::to_string( (int) ( 180 / 3.1416 * std::atan2( WINDOW_HEIGHT/2 - m.y, WINDOW_WIDTH/2 - m.x ) + 180 ) );
            s += ",";
            s += std::to_string(clickCount);
            s += "E";
            socket.send(s.c_str(), s.size() + 1);
            std::cout << s << std::endl;
        }



        for(int i=world.player[ID].bulletIndex; i < world.player[ID].bulletIndex + world.player[ID].bulletCount; i++)
        {
            if(i >= NUM_OF_BULLET){ i = i - NUM_OF_BULLET; }

            world.player[ID].bullets[i].fly();

            sf::Vector2f bulletPosition = world.player[ID].bullets[i].getPosition() + sf::Vector2f(3,3);

            bool collision = false;

            for (int j=0; j<NUM_OF_BLOCK; j++)
            {
                sf::Vector2f p = world.blocks[j].getPosition();
                sf::Vector2f s = world.blocks[j].getSize();

                float x1 = p.x;
                float y1 = p.y;
                float x2 = p.x + s.x;
                float y2 = p.y + s.y;

                if
                (
                    ( 0 < x1 && x1 < WINDOW_WIDTH  ) ||
                    ( 0 < x2 && x2 < WINDOW_WIDTH  ) ||
                    ( 0 < y1 && y1 < WINDOW_HEIGHT ) ||
                    ( 0 < y2 && y2 < WINDOW_HEIGHT )
                )
                {
                    if(x1 <= bulletPosition.x && bulletPosition.x <= x2 && y1 <= bulletPosition.y && bulletPosition.y <= y2)
                    {
                        collision = true;
                        break;
                    }
                }
            }

            bool hit = false;

            if
            (
                distance
                (
                    bulletPosition + sf::Vector2f(3,3),
                    world.player[1].getPosition() + sf::Vector2f(10,10)
                ) < 13
            )
            {
                std::cout << "AH!" << std::endl;
                hit = true;
            }


            if
            (
                (bulletPosition.x<0 || bulletPosition.x>WINDOW_WIDTH || bulletPosition.y<0 || bulletPosition.y>WINDOW_HEIGHT)
                || collision
                || hit
            )
            {
                world.player[ID].bulletIndex++;
                world.player[ID].bulletCount--;
                if( world.player[ID].bulletIndex == NUM_OF_BULLET )
                {
                    world.player[ID].bulletIndex = 0;
                }
            }
        }

        window.clear(sf::Color(50,50,50));
        world.drawOn(window);
        //aimLine.drawOn(window);
        for(int i=world.player[ID].bulletIndex; i < world.player[ID].bulletIndex + world.player[ID].bulletCount; i++)
        {
            if(i >= NUM_OF_BULLET) { i = i - NUM_OF_BULLET; }

            window.draw(world.player[ID].bullets[i]);
        }
        miniWorld.drawOn(window);
        window.display();
    }

    return 0;
}

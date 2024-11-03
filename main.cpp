#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

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
        ( center.x<=blockPosition.x - radius && center.y<=blockPosition.y -radius ) ||
        ( center.x<=blockPosition.x - radius && center.y>=blockPosition.y + blockSize.y + radius ) ||
        ( center.x>=blockPosition.x + blockSize.x + radius && center.y>=blockPosition.y + blockSize.y + radius ) ||
        ( center.x>=blockPosition.x + blockSize.x + radius && center.y<=blockPosition.y - radius )
    )
    {
        return false;
    }

    if
    (
        ( (x1<center.x && center.x<x2) && (dy1<radius || dy2<radius) )
        ||
        ( (y1<center.y && center.y<y2) && (dx1<radius || dx2<radius) )
    )
    {
        return true;
    }

    if (
        ( (x1-radius<=center.x && center.x<=x1 && y1-radius<=center.y && center.y<=y1) && (distance(sf::Vector2f(x1,y1),center)<radius) )
        ||
        ( (x2<=center.x && center.x<=x2+radius && y1-radius<=center.y && center.y<=y1) && (distance(sf::Vector2f(x2,y1),center)<radius) )
        ||
        ( (x2<=center.x && center.x<=x2+radius && y2<=center.y && center.y<=y2+radius) && (distance(sf::Vector2f(x2,y2),center)<radius) )
        ||
        ( (x1-radius<=center.x && center.x<=x1 && y2<=center.y && center.y<=y2+radius) && (distance(sf::Vector2f(x1,y2),center)<radius) )
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

    void setOrigin(sf::Vector2f origin)
    {
        this->origin = origin;
    }



    void createGrid()
    {
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



    void drawGrid(sf::RenderWindow &window)
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

    sf::Vector2i blockPosition;

    Block()
    {
        setFillColor(sf::Color(0,0,255));
    }


    void createBlock(sf::Vector2i blockSize, sf::Vector2i blockPosition, sf::Vector2f origin)
    {
        this->blockPosition = blockPosition;

        setSize(sf::Vector2f(blockSize.x*GRID, blockSize.y*GRID));
        setPosition(origin.x + blockPosition.x*GRID, origin.y + blockPosition.y*GRID);
    }



    void updateBlock(sf::Vector2f origin)
    {
        setPosition(origin.x + blockPosition.x*GRID , origin.y+blockPosition.y*GRID );
    }



    void draw(sf::RenderWindow &window)
    {
        window.draw(*(this));
    }
};



class World : public sf::RectangleShape
{
private:

public:
    Grid grid;
    Block blocks[NUM_OF_BLOCK];
    sf::CircleShape player;


    World()
    {
        grid.createGrid();

        setSize(sf::Vector2f(WIDTH, HEIGHT));
        setFillColor(sf::Color(255,127,7));

        player.setRadius(10);
        player.setFillColor(sf::Color(255,255,0));
        player.setPosition(sf::Vector2f(WINDOW_WIDTH/2-player.getRadius(), WINDOW_HEIGHT/2-player.getRadius()));

        for(short i=0; i<NUM_OF_BLOCK; i++)
        {
            int blockLength = randomInt(4,12);
            sf::Vector2i blockSize;
            sf::Vector2i blockPosition;

            while(true)
            {
                if (randomInt(0,1))
                {
                    blockSize.x = blockLength;
                    blockSize.y = 1;

                    blockPosition.x = randomInt(0, WIDTH/GRID - blockLength);
                    blockPosition.y = randomInt(0, HEIGHT/GRID - 1);
                }
                else
                {
                    blockSize.x = 1;
                    blockSize.y = blockLength;

                    blockPosition.x = randomInt(0, WIDTH/GRID - 1);
                    blockPosition.y = randomInt(0, HEIGHT/GRID - blockLength);
                }

                //EXTRA //CHECK

                sf::Vector2f blockAbsolutePosition = getPosition() + sf::Vector2f(blockPosition.x*GRID, blockPosition.y*GRID);
                sf::Vector2f blockSize = sf::Vector2f(blockSize.x*GRID, blockSize.y*GRID);

                sf::Vector2f center = sf::Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
                float radius = 10;

                if( !
                    (
                        ( center.x<=blockAbsolutePosition.x - radius && center.y<=blockAbsolutePosition.y -radius ) ||
                        ( center.x<=blockAbsolutePosition.x - radius && center.y>=blockAbsolutePosition.y + blockSize.y + radius ) ||
                        ( center.x>=blockAbsolutePosition.x + blockSize.x + radius && center.y>=blockAbsolutePosition.y + blockSize.y + radius ) ||
                        ( center.x>=blockAbsolutePosition.x + blockSize.x + radius && center.y<=blockAbsolutePosition.y - radius )
                    )
                )
                {
                    continue;
                }
                else
                {
                    break;
                }
            }

            blocks[i].createBlock(blockSize , blockPosition, getPosition());
        }
    }

    void moveLeft(float dx)
    {
        setPosition(getPosition() - sf::Vector2f(dx,0));
        grid.setOrigin(getPosition());
        grid.createGrid();
        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            blocks[i].updateBlock(getPosition());
        }
    }

    void moveRight(float dx)
    {
        setPosition(getPosition() + sf::Vector2f(dx,0));
        grid.setOrigin(getPosition());
        grid.createGrid();
        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            blocks[i].updateBlock(getPosition());
        }
    }

    void moveUp(float dy)
    {
        setPosition(getPosition() - sf::Vector2f(0,dy));
        grid.setOrigin(getPosition());
        grid.createGrid();
        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            blocks[i].updateBlock(getPosition());
        }
    }

    void moveDown(float dy)
    {
        setPosition(getPosition() + sf::Vector2f(0,dy));
        grid.setOrigin(getPosition());
        grid.createGrid();
        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            blocks[i].updateBlock(getPosition());
        }
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(*(this));
        grid.drawGrid(window);
        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            blocks[i].draw(window);
        }
        window.draw(player);
    }
};




class MiniWorld: public sf::RectangleShape
{
public:
    float width;
    float height;
    float scale_factor;
    Block miniBlocks[NUM_OF_BLOCK];
    sf::CircleShape miniPlayer;

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

        sf::Vector2f positionOfPlayerRelativeToWorld = world.player.getPosition() - world.getPosition();
        sf::Vector2f positionOfMiniPlayerRelativeToMiniWorld = sf::Vector2f(positionOfPlayerRelativeToWorld.x * scale_factor, positionOfPlayerRelativeToWorld.y * scale_factor);

        miniPlayer.setPosition(getPosition() + positionOfMiniPlayerRelativeToMiniWorld);

        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            miniBlocks[i].setSize(sf::Vector2f(world.blocks[i].getSize().x*scale_factor, world.blocks[i].getSize().y*scale_factor));

            sf::Vector2f positionOfBlockRelativeToWorld = world.blocks[i].getPosition() - world.getPosition();
            sf::Vector2f positionOfMiniBlockRelativeToMiniWorld = sf::Vector2f(positionOfBlockRelativeToWorld.x * scale_factor, positionOfBlockRelativeToWorld.y * scale_factor);

            miniBlocks[i].setPosition(getPosition() + positionOfMiniBlockRelativeToMiniWorld);
        }
    }

    void moveLeftMiniPlayer(float dx)
    {
        miniPlayer.setPosition(miniPlayer.getPosition() - sf::Vector2f(dx*scale_factor, 0));
    }

    void moveRightMiniPlayer(float dx)
    {
        miniPlayer.setPosition(miniPlayer.getPosition() + sf::Vector2f(dx*scale_factor, 0));
    }

    void moveUpMiniPlayer(float dy)
    {
        miniPlayer.setPosition(miniPlayer.getPosition() - sf::Vector2f(0, dy*scale_factor));
    }

    void moveDownMiniPlayer(float dy)
    {
        miniPlayer.setPosition(miniPlayer.getPosition() + sf::Vector2f(0, dy*scale_factor));
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(*(this));
        for (short i=0; i<NUM_OF_BLOCK; i++)
        {
            miniBlocks[i].draw(window);
        }
        window.draw(miniPlayer);
    }
};




int main()
{
srand(static_cast <unsigned int> (time(0)));


    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "ShooterGame");

    World world;

    MiniWorld miniWorld(world);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::Up)
                {
                    world.moveUp(-10);
                    miniWorld.moveUpMiniPlayer(10);

                    for (short i=0; i<NUM_OF_BLOCK; i++)
                    {
                        sf::Vector2f blockAbsolutePosition = world.blocks[i].getPosition();
                        sf::Vector2f blockSize = world.blocks[i].getSize();

                        if (checkCollision(blockAbsolutePosition, blockSize))
                        {
                            world.moveUp(10);
                            miniWorld.moveUpMiniPlayer(-10);
                            break;
                        }
                    }

                }

                if(event.key.code == sf::Keyboard::Down)
                {
                    world.moveDown(-10);
                    miniWorld.moveDownMiniPlayer(10);

                    for (short i=0; i<NUM_OF_BLOCK; i++)
                    {
                        sf::Vector2f blockAbsolutePosition = world.blocks[i].getPosition();
                        sf::Vector2f blockSize = world.blocks[i].getSize();

                        if (checkCollision(blockAbsolutePosition, blockSize))
                        {
                            world.moveDown(10);
                            miniWorld.moveDownMiniPlayer(-10);
                            break;
                        }
                    }
                }

                if(event.key.code == sf::Keyboard::Left)
                {
                    world.moveLeft(-10);
                    miniWorld.moveLeftMiniPlayer(10);


                    for (short i=0; i<NUM_OF_BLOCK; i++)
                    {
                        sf::Vector2f blockAbsolutePosition = world.blocks[i].getPosition();
                        sf::Vector2f blockSize = world.blocks[i].getSize();

                        if (checkCollision(blockAbsolutePosition, blockSize))
                        {
                            world.moveLeft(10);
                            miniWorld.moveLeftMiniPlayer(-10);
                            break;
                        }
                    }
                }

                if(event.key.code == sf::Keyboard::Right)
                {
                    world.moveRight(-10);
                    miniWorld.moveRightMiniPlayer(10);


                    for (short i=0; i<NUM_OF_BLOCK; i++)
                    {
                        sf::Vector2f blockAbsolutePosition = world.blocks[i].getPosition();
                        sf::Vector2f blockSize = world.blocks[i].getSize();

                        if (checkCollision(blockAbsolutePosition, blockSize))
                        {
                            world.moveRight(10);
                            miniWorld.moveRightMiniPlayer(-10);
                            break;
                        }
                    }
                }
            }
        }

        window.clear(sf::Color(50,50,50));
        world.draw(window);
        miniWorld.draw(window);
        window.display();
    }

    return 0;
}

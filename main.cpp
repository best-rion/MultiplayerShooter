#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <cmath>
#include <iostream>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 720
#define WORLD_WIDTH 1280
#define WORLD_HEIGHT 1000
#define GRID 20
#define NUM_OF_BLOCKS 40
#define NUM_OF_BULLETS 500
#define BULLET_RADIUS 4
#define PLAYER_RADIUS 10
#define TOTAL_PLAYERS 4

int myId = 0;
int numOfPlayers = 0;

const sf::Vector2f window_center(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
sf::Vector2f myRelPosition(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
sf::Vector2i mousePosition;


int playerVelocity = 10;
float bulletVelocity = 15;


float distance(sf::Vector2f p1, sf::Vector2f p2)
{
    return std::pow(std::pow(p1.x-p2.x,2)+std::pow(p1.y-p2.y,2), 0.5);
}


int randomInt(int _min, int _max)
{
    return rand()%(_max - _min + 1) + _min;
}


class Grid
{
private:

    sf::Vector2f origin;
    sf::Vertex verticalLines[WORLD_WIDTH/GRID-1][2];
    sf::Vertex horizontalLines[WORLD_HEIGHT/GRID-1][2];

    sf::Color color = sf::Color(179, 78, 2);

public:

    void makeGrid( sf::Vector2f origin )
    {
        this->origin = origin;

        for(short x=1; x<WORLD_WIDTH/GRID; x++)
        {
            verticalLines[x-1][0].position = origin + sf::Vector2f(x*GRID, 0);
            verticalLines[x-1][0].color = color;

            verticalLines[x-1][1].position = origin + sf::Vector2f(x*GRID, WORLD_HEIGHT);
            verticalLines[x-1][1].color = color;
        }

        for(short y=1; y<WORLD_HEIGHT/GRID; y++)
        {
            horizontalLines[y-1][0].position = origin + sf::Vector2f(0, y*GRID);
            horizontalLines[y-1][0].color = color;

            horizontalLines[y-1][1].position = origin + sf::Vector2f(WORLD_WIDTH, y*GRID);
            horizontalLines[y-1][1].color = color;
        }
    }



    void drawOn(sf::RenderWindow &window)
    {
        for(short x=1; x<WORLD_WIDTH/GRID; x++)
        {
            window.draw(verticalLines[x-1],2,sf::Lines);
        }

        for(short y=1; y<WORLD_HEIGHT/GRID; y++)
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
        setFillColor(sf::Color(18, 84, 179));
    }


    void makeBlock(sf::Vector2f blockSize, sf::Vector2f absPosition, sf::Vector2f origin)
    {
        relativePosition = absPosition - origin;

        setSize( blockSize );
        setPosition( absPosition );
    }


    void updateBlock(sf::Vector2f origin)
    {
        setPosition( origin.x + relativePosition.x , origin.y + relativePosition.y );
    }
};


class Bullet: public sf::CircleShape
{
public:

    float angle; // Degree

    Bullet()
    {
        setFillColor( sf::Color::Red );
        setRadius( BULLET_RADIUS );
    }


    void fly()
    {
        float dx = bulletVelocity * std::cos(3.1416/180*angle);
        float dy = bulletVelocity * std::sin(3.1416/180*angle);
        setPosition( getPosition() + sf::Vector2f(dx,dy) );
    }
};


class HealthBar
{
public:

    sf::RectangleShape bg;
    sf::RectangleShape bar;

    HealthBar()
    {
        bg.setFillColor(sf::Color(100,100,100));
        bg.setOutlineColor(sf::Color::Black);
        bg.setOutlineThickness(1);
    }

    void init(sf::Color color, int id, sf::Vector2f p, sf::Vector2f sz)
    {
        float yPosition = p.y + sz.y + 8 + id * 16;
        bg.setPosition(  sf::Vector2f( p.x, yPosition )  );
        bg.setSize(  sf::Vector2f( sz.x ,8 )  );

        bar.setPosition(  bg.getPosition()  );
        bar.setSize(  bg.getSize()  );
        bar.setFillColor( color );
    }

    void drawOn(sf::RenderWindow &window)
    {
        window.draw(bg);
        window.draw(bar);
    }
};


class Player: public sf::CircleShape
{
public:

    float life = 1;
    HealthBar healthBar;

    float gunAngle;
    bool click;

    sf::RectangleShape gun;
    Bullet bullets[NUM_OF_BULLETS];
    int bulletCount=0;
    int bulletIndex=0;

    Player()
    {
        setRadius( PLAYER_RADIUS );
        setFillColor( sf::Color::Yellow );
        setPosition( WINDOW_WIDTH/2 - PLAYER_RADIUS, WINDOW_HEIGHT/2 - PLAYER_RADIUS );
        setOutlineColor( sf::Color::Yellow );
        setOutlineThickness( 2 );

        gun.setSize( sf::Vector2f(16,4) );
        gun.setFillColor( sf::Color::Black );
        gun.setPosition( WINDOW_WIDTH/2, WINDOW_HEIGHT/2-2 );
        gun.setOrigin( sf::Vector2f( WINDOW_WIDTH/2, WINDOW_HEIGHT/2) - gun.getPosition() );
    }


    void update(sf::Vector2f origin, sf::Vector2f p)
    {
        setPosition( origin + sf::Vector2f( p.x-PLAYER_RADIUS, p.y-PLAYER_RADIUS ) );
    }


    void updateGunAngle(float angle)
    {
        gun.rotate( angle - gun.getRotation() );
    }


    void updateGunPosition()
    {
        gun.setPosition( getPosition() + sf::Vector2f(PLAYER_RADIUS, PLAYER_RADIUS-2) );
    }


    void updateHealthBar()
    {
        healthBar.bar.setSize(  sf::Vector2f( healthBar.bg.getSize().x * life, 8 )  );
    }


    void drawOn(sf::RenderWindow &window)
    {
        window.draw(*(this));
        window.draw(gun);
        healthBar.drawOn(window);
    }
};


class World : public sf::RectangleShape
{
public:

    Grid grid;
    Block blocks[NUM_OF_BLOCKS];
    Player players[TOTAL_PLAYERS];


    void playerInit()
    {
        for( int id=0; id < TOTAL_PLAYERS; id++)
        {
            if ( id != myId )
            {
                players[ id ].setFillColor(sf::Color::Green);
                players[ id ].setOutlineColor(sf::Color::Green);
            }
        }
    }


    World()
    {
        setSize(sf::Vector2f(WORLD_WIDTH, WORLD_HEIGHT));
        setFillColor(sf::Color(232, 140, 50));
        setPosition( 0, 0 );

        grid.makeGrid( getPosition() );

        srand(42);
        for(short i=0; i<NUM_OF_BLOCKS; i++)
        {
            int blockLengthInGrid = randomInt(4,12);
            sf::Vector2i blockRelPositionInGrid;
            sf::Vector2i blockSizeInGrid;

            sf::Vector2f blockAbsPosition;
            sf::Vector2f blockSize;

            while(true)
            {
                if (randomInt(0,1))
                {
                    blockSizeInGrid.x = blockLengthInGrid;
                    blockSizeInGrid.y = 2;

                    blockRelPositionInGrid.x = randomInt(1, WORLD_WIDTH /GRID - 1 - blockLengthInGrid);
                    blockRelPositionInGrid.y = randomInt(1, WORLD_HEIGHT/GRID - 1 - 1);
                }
                else
                {
                    blockSizeInGrid.x = 2;
                    blockSizeInGrid.y = blockLengthInGrid;

                    blockRelPositionInGrid.x = randomInt(0, WORLD_WIDTH /GRID - 1 - 1);
                    blockRelPositionInGrid.y = randomInt(0, WORLD_HEIGHT/GRID - 1 - blockLengthInGrid);
                }

                //CHECK

                blockAbsPosition = getPosition() + sf::Vector2f(blockRelPositionInGrid.x*GRID, blockRelPositionInGrid.y*GRID);
                blockSize = sf::Vector2f(blockSizeInGrid.x*GRID, blockSizeInGrid.y*GRID);

                sf::Vector2f center = sf::Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

                float x1 = blockAbsPosition.x;
                float y1 = blockAbsPosition.y;
                float x2 = blockAbsPosition.x + blockSize.x;
                float y2 = blockAbsPosition.y + blockSize.y;

                if( !( center.x >= x1 - 2*PLAYER_RADIUS  && center.x <= x2 + 2*PLAYER_RADIUS && center.y >= y1 - 2*PLAYER_RADIUS && center.y <= y2 + 2*PLAYER_RADIUS ) )
                {
                    break;
                }
            }

            blocks[i].makeBlock( blockSize , blockAbsPosition, getPosition());
        }
    }


    void update()
    {
        sf::Vector2f worldAbsPosition = sf::Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2) - myRelPosition;

        setPosition( worldAbsPosition );
        grid.makeGrid( getPosition() );

        for (short i=0; i<NUM_OF_BLOCKS; i++)
        {
            blocks[i].updateBlock(getPosition());
        }
    }


    void drawOn(sf::RenderWindow &window)
    {
        window.draw(*(this));

        grid.drawOn(window);

        for (short i=0; i<NUM_OF_BLOCKS; i++)
        {
            window.draw(blocks[i]);
        }

        for( int id=0; id < numOfPlayers; id++)
        {
            players[ id ].drawOn(window);
        }
    }
};


World world;


class MiniWorld: public sf::RectangleShape
{
public:

    float scale_factor;
    Block miniBlocks[NUM_OF_BLOCKS];
    sf::CircleShape miniPlayers[TOTAL_PLAYERS];


    void miniPlayerInit()
    {

        for (int id=0; id < TOTAL_PLAYERS; id++)
        {
            miniPlayers[ id ].setRadius( PLAYER_RADIUS*scale_factor );
            miniPlayers[ id ].setFillColor( world.players[ id ].getFillColor() );

            sf::Vector2f relPos = world.players[ id ].getPosition() - world.getPosition();

            miniPlayers[ id ].setPosition( getPosition() + sf::Vector2f( (relPos.x - PLAYER_RADIUS)*scale_factor, (relPos.y - PLAYER_RADIUS)*scale_factor) );

            world.players[ id ].healthBar.init(  world.players[ id ].getFillColor(), id, getPosition(), getSize() );
        }

    }


    MiniWorld(World &world)
    {
        float height = (float) WINDOW_HEIGHT/4;
        scale_factor = height/WORLD_HEIGHT;
        float width = WORLD_WIDTH*scale_factor;

        setFillColor(sf::Color(230, 130, 40));
        setPosition(WINDOW_WIDTH-width-30,30);
        setSize(sf::Vector2f(width,height));
        setOutlineColor(sf::Color::Black);
        setOutlineThickness(1);


        for (int i=0; i<NUM_OF_BLOCKS; i++)
        {
            miniBlocks[i].setSize(sf::Vector2f( world.blocks[i].getSize().x*scale_factor, world.blocks[i].getSize().y*scale_factor ));

            sf::Vector2f relPos = world.blocks[i].relativePosition;
            sf::Vector2f positionOfMiniBlockRelativeToMiniWorld = sf::Vector2f(relPos.x * scale_factor, relPos.y * scale_factor);

            miniBlocks[i].setPosition(getPosition() + positionOfMiniBlockRelativeToMiniWorld);
        }
    }



    void update()
    {
        for (int id=0; id < numOfPlayers; id++)
        {
            sf::Vector2f relPos = world.players[ id ].getPosition() - world.getPosition();

            miniPlayers[ id ].setPosition( getPosition() + sf::Vector2f( relPos.x*scale_factor, relPos.y*scale_factor) );
        }
    }


    void drawOn(sf::RenderWindow &window)
    {
        window.draw(*(this));

        for (short i=0; i<NUM_OF_BLOCKS; i++)
        {
            window.draw(miniBlocks[i]);
        }
        for (int id=0; id < numOfPlayers; id++)
        {
            window.draw(miniPlayers[ id ]);
        }
    }
};


MiniWorld miniWorld(world);


bool firstMessage = true;


void parse(char buffer[18*TOTAL_PLAYERS])
{
    int id;
    float x;
    float y;
    int temp;
    int index=0;
    int sum=0;


    if (  buffer[1] == 'F' && firstMessage  )
    {
        myId = buffer[0] - '0';
        numOfPlayers = 1;

        world.playerInit();
        miniWorld.miniPlayerInit();

        firstMessage = false;
    }

    if( buffer[1] != 'F' )
    {
        for( int i=0; i<numOfPlayers; i++)
        {
            id = buffer[index] - '0';

            if ( id+1 > numOfPlayers )
            {
                numOfPlayers = id + 1;
            }

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
            x = sum;

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
            y = sum;
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
            world.players[id].gunAngle = sum;
            index+=1;

            if(buffer[index] == '1')
            {
                world.players[id].click = true;
            }
            else{
                world.players[id].click = false;
            }
            index+=1;

            while(buffer[index]!=';')
            {
                index++;
            }

            index+=1;

            //
            if (id == myId)
            {
                myRelPosition = sf::Vector2f( x, y );
            }
            else
            {
                world.players[id].update( world.getPosition(), sf::Vector2f( x, y ) );
                world.players[id].updateGunPosition();
            }

            if ( world.players[id].click )
            {
                int indx;
                indx = world.players[ id ].bulletIndex + world.players[ id ].bulletCount;
                indx = (indx >= NUM_OF_BULLETS) ? indx - NUM_OF_BULLETS : indx ;

                world.players[ id ].bullets[indx].angle = world.players[ id ].gunAngle;
                world.players[ id ].bullets[indx].setPosition( world.players[id].gun.getPosition() - sf::Vector2f(2,2) );

                if(world.players[ id ].bulletCount < NUM_OF_BULLETS)
                {
                    world.players[ id ].bulletCount++;
                }
            }
            //
        }
    }
}


bool checkCollision(sf::Vector2f &p)
{
    for (int i=0; i<NUM_OF_BLOCKS; i++)
    {
        sf::Vector2f blockPosition = world.blocks[i].relativePosition;
        sf::Vector2f blockSize = world.blocks[i].getSize();

        float x1 = blockPosition.x;
        float x2 = blockPosition.x + blockSize.x;
        float y1 = blockPosition.y;
        float y2 = blockPosition.y + blockSize.y;

        sf::Vector2f center = p;

        float dx1 = std::abs(x1 - center.x);
        float dx2 = std::abs(x2 - center.x);
        float dy1 = std::abs(y1 - center.y);
        float dy2 = std::abs(y2 - center.y);

        if
        (
            ( center.x <= x1 - PLAYER_RADIUS && center.y <= y1 - PLAYER_RADIUS ) ||
            ( center.x <= x1 - PLAYER_RADIUS && center.y >= y2 + PLAYER_RADIUS ) ||
            ( center.x >= x2 + PLAYER_RADIUS && center.y >= y2 + PLAYER_RADIUS ) ||
            ( center.x >= x2 + PLAYER_RADIUS && center.y <= y1 - PLAYER_RADIUS )
        )
        {
            continue;
        }

        if
        (
            ( (x1 < center.x && center.x < x2) && (dy1 < PLAYER_RADIUS || dy2 < PLAYER_RADIUS) )
            ||
            ( (y1 < center.y && center.y < y2) && (dx1 < PLAYER_RADIUS || dx2 < PLAYER_RADIUS) )
        )
        {
            return true;
        }

        if (
            ( ( x1-PLAYER_RADIUS <= center.x && center.x <= x1 && y1-PLAYER_RADIUS <= center.y && center.y <= y1) && ( distance(sf::Vector2f(x1,y1),center) < PLAYER_RADIUS ) )
            ||
            ( ( x2 <= center.x && center.x <= x2+PLAYER_RADIUS && y1-PLAYER_RADIUS <= center.y && center.y <= y1) && ( distance(sf::Vector2f(x2,y1),center) < PLAYER_RADIUS ) )
            ||
            ( ( x2 <= center.x && center.x <= x2+PLAYER_RADIUS && y2 <= center.y && center.y <= y2+PLAYER_RADIUS) && ( distance(sf::Vector2f(x2,y2),center) < PLAYER_RADIUS ) )
            ||
            ( ( x1-PLAYER_RADIUS <= center.x && center.x <= x1 && y2 <= center.y && center.y <= y2+PLAYER_RADIUS) && ( distance(sf::Vector2f(x1,y2),center) < PLAYER_RADIUS ) )
           )
        {
            return true;
        }
    }
    return false;
}


bool gameNotOver = true;
std::string overMessage;


void update(sf::TcpSocket* socketPtr)
{
    // std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    while (gameNotOver)
    {
        char buffer[18*TOTAL_PLAYERS];
        std::size_t received = 0;
        socketPtr->receive(buffer, sizeof(buffer), received);
        if(received>0)
        {
            parse(buffer);

            if (!firstMessage)
            {
                /// UPDATE EVERYTHING

                // GUN

                for ( int id=0; id < numOfPlayers; id++)
                {
                    world.players[id].updateGunAngle( world.players[id].gunAngle );

                    for(int i=world.players[ id ].bulletIndex; i < world.players[ id ].bulletIndex + world.players[ id ].bulletCount; i++)
                    {
                        if(i >= NUM_OF_BULLETS){ i = i - NUM_OF_BULLETS; }

                        world.players[ id ].bullets[i].fly();

                        sf::Vector2f bulletPosition = world.players[ id ].bullets[i].getPosition() + sf::Vector2f(2,2);

                        bool collision = false;

                        for (int j=0; j<NUM_OF_BLOCKS; j++)
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

                        for (int id2=0; id2<numOfPlayers; id2++)
                        {
                            if( id2 != id )
                            {
                                if
                                (
                                    distance
                                    (
                                        bulletPosition + sf::Vector2f(3,3),
                                        world.players[ id2 ].getPosition() + sf::Vector2f(10,10)
                                    ) < 13
                                )
                                {
                                    hit = true;

                                    world.players[id2].life -= 0.004;
                                    world.players[id2].updateHealthBar();

                                    sf::Color c = world.players[id2].getFillColor();
                                    world.players[id2].setFillColor( sf::Color(c.r, c.g, c.b,(uint8_t) (255*world.players[id2].life)) );

                                    if ( world.players[id2].life < 0 )
                                    {
                                        if (id2 == myId)
                                        {
                                            overMessage = "YOU LOSE";
                                        }
                                        else
                                        {
                                            overMessage = "YOU WIN!";
                                        }

                                        gameNotOver = false;
                                    }
                                }
                            }
                        }


                        if
                        (
                            (bulletPosition.x<0 || bulletPosition.x>WINDOW_WIDTH || bulletPosition.y<0 || bulletPosition.y>WINDOW_HEIGHT)
                            || collision
                            || hit
                        )
                        {
                            world.players[ id ].bulletIndex++;
                            world.players[ id ].bulletCount--;
                            if( world.players[ id ].bulletIndex == NUM_OF_BULLETS )
                            {
                                world.players[ id ].bulletIndex = 0;
                            }
                        }
                    }
                }
            }

            // MY RELATIVE POSITION
            /* Already done in parse function */

            // WORLD
            world.update();

            // MINI WORLD
            miniWorld.update();

        }
    }
}


void gamePlay(sf::RenderWindow &window, sf::TcpSocket &socket)
{
    int click = 0;

    while (window.isOpen() && gameNotOver)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            mousePosition = sf::Mouse::getPosition(window);
            float angle = 180 / 3.1416 * std::atan2( window_center.y - mousePosition.y, window_center.x - mousePosition.x ) + 180;

            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                click=1;
            }
            if (event.type == sf::Event::MouseButtonReleased)
            {
                click=0;
            }


            sf::Vector2f newRelativePosition = myRelPosition;
            if(event.type == sf::Event::KeyPressed)
            {
                sf::Vector2f d;

                if(event.key.code == sf::Keyboard::Left)
                {
                    d = sf::Vector2f(-playerVelocity, 0);
                }

                if(event.key.code == sf::Keyboard::Right)
                {
                    d = sf::Vector2f(playerVelocity, 0);
                }

                if(event.key.code == sf::Keyboard::Up)
                {
                    d = sf::Vector2f(0, -playerVelocity);
                }

                if(event.key.code == sf::Keyboard::Down)
                {
                    d = sf::Vector2f(0, playerVelocity);
                }


                sf::Vector2f playerTestRelPosition = myRelPosition + d;

                if
                (
                    !checkCollision( playerTestRelPosition )
                    &&
                    (
                        0 < playerTestRelPosition.x && playerTestRelPosition.x < WORLD_WIDTH &&
                        0 < playerTestRelPosition.y && playerTestRelPosition.y < WORLD_HEIGHT
                    )
                )
                {
                    newRelativePosition = myRelPosition + d ;
                }
            }


            std::string s = "S";

            if(firstMessage)
            {
                // SENDING THE FIRST MESSAGE
                s += "F";
            }
            else
            {
                // SENDING MESSAGE
                s += std::to_string((int) newRelativePosition.x);
                s += ",";
                s += std::to_string((int) newRelativePosition.y);
                s += ",";
                s += std::to_string( (int) angle );
                s += ",";
                s += std::to_string( click );
            }

            s += "E";
            socket.send(s.c_str(), s.size() + 1);



        }


        window.clear(sf::Color(50,50,50));
        world.drawOn(window);

        for (int id=0; id<numOfPlayers; id++)
        {
            for(int i=world.players[ id ].bulletIndex; i < world.players[ id ].bulletIndex + world.players[ id ].bulletCount; i++)
            {
                if(i >= NUM_OF_BULLETS) { i = i - NUM_OF_BULLETS; }

                if (i%5==0)
                {
                    window.draw(world.players[ id ].bullets[i]);
                }
            }
        }
        miniWorld.drawOn(window);
        window.display();
    }
    socket.disconnect();
    window.close();
}


int main()
{
    std::string ip;
    std::cout << "IP address of server : ";
    std::cin >> ip;

    //10.18.122.174

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "ShooterGame");

    sf::TcpSocket socket;
    socket.connect(ip, 8040);

    sf::Thread th(&update, &socket);
    th.launch();

    gamePlay(window,  socket);

    std::cout << overMessage << std::endl;

    std::cin >> overMessage;

    return 0;
}


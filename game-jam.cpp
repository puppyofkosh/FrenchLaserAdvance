
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <iostream>
#include <map>
#include <string>
#include <algorithm>

#include <stdio.h>
#include<stdlib.h>

#include <math.h>

#include <sstream>

using std::string;

struct Thing
{
    bool dead;
    sf::Sprite sprite;

    Thing() :dead(false){}
};

bool isThingDead(const Thing& t)
{
    return t.dead;
}

enum Origin
{
    PLAYER,
    BADDIE
};

struct Bullet : public Thing
{
    float xvel;
    float yvel;
    //sf::Sprite sprite;

    Origin origin;
};

struct Baddie : public Thing
{
    float lastBulletTime;
    // sf::Sprite sprite;

    Baddie()
	:lastBulletTime(0)
    {
    }
};


namespace game
{
    int playerScore = 0;

    const int MAX_X = 1280;
    const int BULLET_SPEED = 10;

    sf::Clock clock;


    sf::Font gameFont;

    const int MOVE_RATE = 5;
    const int BADDIE_RATE = 2;
    int max_baddies = 5;

    const int MAX_PLAYER_HP = 42;
    const int SG_COOLDOWN = 3;
    int playerHP = MAX_PLAYER_HP;
    
    float playerSGTime;


    std::vector<Bullet> bullets;
    std::vector<Baddie> baddies;


    enum GameState { ShowingSplash, Playing, Dead, Over };
    GameState state = ShowingSplash;
};

namespace image
{
    sf::Image splash;
    sf::Image player;
    sf::Image bullet;

    sf::Image baddie;

    sf::Image bg;
};

namespace sprite
{
    sf::Sprite splash;
    sf::Sprite player;
    sf::Sprite bullet;
    sf::Sprite bg;

    //sf::Sprite baddie;

};


template<typename T>
std::string toString(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}


void toSFString(const std::string& str,
		sf::Vector2f p,
		sf::String* text,
		const sf::Color& c)
{
    text->SetText(str);
    text->SetFont(game::gameFont);
    text->SetSize(25);
    text->Move(p.x, p.y);
    //text->SetColor(sf::Color(255, 128, 0));
    text->SetColor(c);

}

float distsq(const sf::Vector2f& a, const sf::Vector2f& b)
{
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}



void loadResources()
{
    image::splash.LoadFromFile("splash.png");
    sprite::splash.SetImage(image::splash);
    
    image::player.LoadFromFile("player.png");
    sprite::player.SetImage(image::player);
    sprite::player.Scale(4,4);
    sprite::player.SetCenter(image::player.GetWidth()/2, image::player.GetHeight()/2);

    image::bullet.LoadFromFile("bullet.png");
    sprite::bullet.SetImage(image::bullet);
    
    image::baddie.LoadFromFile("baddie_sprite_sheet.png");
    
    image::bg.LoadFromFile("bg.png");
    sprite::bg.SetImage(image::bg);
    sprite::bg.Scale(1.6,1.7);
    
    game::gameFont.LoadFromFile("font.ttf");
 
}

void drawSplashScreen(sf::RenderWindow & renderWindow)
{	
  //renderWindow.Draw(sprite::splash);


    sf::String welcome;
    toSFString("You are a chicken. \n The french are advancing and \nwant to fry you with lasers.\n\n Space to shoot, Z to fire shotgun",
	       sf::Vector2f(200, 500),
	       &welcome,
	       sf::Color(255, 0, 0));

    renderWindow.Draw(welcome);
    renderWindow.Display();


    sf::Event event;
    bool running = true;
    while (running)
    {
	while(renderWindow.GetEvent(event))
	{
	    if(event.Type == sf::Event::KeyPressed 
	       || event.Type == sf::Event::MouseButtonPressed
	       || event.Type == sf::Event::Closed )
	    {
		return;
	    }
    
	}
    }
}


void drawObjects(sf::RenderWindow & rw)
{
    rw.Draw(sprite::bg);
    rw.Draw(sprite::player);
    //rw.Draw(sprite::baddie);
    
    for (std::vector<Bullet>::iterator i = game::bullets.begin();
	 i != game::bullets.end();
	 ++i)
    {
	rw.Draw(i->sprite);
    }
    for (std::vector<Baddie>::iterator i = game::baddies.begin();
	 i != game::baddies.end();
	 ++i)
    {
	rw.Draw(i->sprite);
    }

    sf::String str;
    toSFString("Score:" + toString(game::playerScore),
     	       sf::Vector2f(50, 950),
     	       &str,
	       sf::Color(255, 128, 0));
    rw.Draw(str);

    sf::String str2;
    toSFString("HP: " + toString(game::playerHP),
	       sf::Vector2f(500, 950),
	       &str2,
	       sf::Color(255, 128, 0));
    rw.Draw(str2);

    if (game::clock.GetElapsedTime() - game::playerSGTime > game::SG_COOLDOWN)
    {
	sf::String str2;
	toSFString("SG READY",
		   sf::Vector2f(800, 950),
		   &str2,
		   sf::Color(255, 0, 0));
	rw.Draw(str2);
    }

}

bool isOffScreen(const Thing& b)
{
    return b.sprite.GetPosition().x >= 2000 ||
	b.sprite.GetPosition().x <= -100 ||
	b.sprite.GetPosition().y <= -100 ||
	b.sprite.GetPosition().y >= 2000;
}

bool boxCollision(const sf::Sprite& a, const sf::Sprite& b)
{
    sf::IntRect aRect(a.GetPosition().x - a.GetSize().x/2,
		      a.GetPosition().y - a.GetSize().y/2,
		      a.GetPosition().x + a.GetSize().x/2,
		      a.GetPosition().y + a.GetSize().y/2);

    sf::IntRect bRect(b.GetPosition().x - b.GetSize().x/2,
		      b.GetPosition().y - b.GetSize().y/2,
		      b.GetPosition().x + b.GetSize().x/2,
		      b.GetPosition().y + b.GetSize().y/2);

    return aRect.Intersects(bRect);
}

Bullet* isBaddieDead(const Baddie& f)
{
    for (std::vector<Bullet>::iterator i = game::bullets.begin();
	 i != game::bullets.end();
	 ++i)
    {
	if (boxCollision(i->sprite, f.sprite) && i->origin == PLAYER)
	    return &(*i);
    }


    return 0;
}

void checkPlayerHits()
{
    short dmg = 0;

    for (std::vector<Bullet>::iterator i = game::bullets.begin();
	 i != game::bullets.end();
	 ++i)
    {
	if (boxCollision(sprite::player, i->sprite) && i->origin == BADDIE)
	{
	    dmg ++;
	    i->dead = true;
	}
    }

    game::playerHP -= dmg;
    sf::Color c = sprite::player.GetColor();
    c.g -= (255.0/ (game::MAX_PLAYER_HP - 1)) * dmg;
    c.b = c.g;
    sprite::player.SetColor(c);
}

void updateObjects()
{    

    for (std::vector<Bullet>::iterator i = game::bullets.begin();
	 i != game::bullets.end();
	 ++i)
    {
	i->sprite.Move(i->xvel, i->yvel);
    }

    // Remove bullets offscreen   
    game::bullets.erase(std::remove_if(game::bullets.begin(),
				       game::bullets.end(),
				       isOffScreen), game::bullets.end());
    game::bullets.erase(std::remove_if(game::bullets.begin(),
				       game::bullets.end(),
				       isThingDead), game::bullets.end());
    
    checkPlayerHits();
    if (game::playerHP <= 0)
    {
	game::state = game::Dead;
	//sprite::player.SetPosition(-100, -100);
    }

    // Add baddies
    if (game::baddies.size() < game::max_baddies)
    {
	Baddie f;
	f.sprite.SetImage(image::baddie);
	f.sprite.FlipX(true);
	f.sprite.SetCenter(10, 16);
	f.sprite.Scale(2,2);
	f.sprite.SetPosition(1500, rand() % 1000);
	f.sprite.SetSubRect(sf::IntRect(0, 0, 20, 32));
	game::baddies.push_back(f);
    }

    for (std::vector<Baddie>::iterator i = game::baddies.begin();
	 i != game::baddies.end();
	 ++i)
    {
	// Kill dead baddies
	Bullet* b = isBaddieDead(*i);
	if (b)
	{
	    if (++game::playerScore % 12 == 0)
	    {
		game::playerHP++;
		//sf::Color c = sprite::player.GetColor();
		//c.g = 255;
	    }
	    if (game::playerScore % 10 == 0)
	    {
		game::max_baddies++;
	    }
	    // if (game::playerHP != game::MAX_PLAYER_HP && 
	    // 	sprite::player.GetColor().g == 255)
	    // {
	    // 	sf::Color c = sprite::player.GetColor();
	    // 	c.g--;
	    // 	sprite::player.SetColor(c);
	    // }
	    

	    i->dead = true;
	    b->dead = true;
	}

	if (distsq(i->sprite.GetPosition(), sprite::player.GetPosition()) > 500)
	{
	    i->sprite.Move(-2, 0);
	}
	
	// Fire bullets
	float curtime = game::clock.GetElapsedTime();
	//std::cout << "curtime is " << curtime << std::endl;
	//std::cout << "last bullet time is" << i->lastBulletTime << std::endl;

	if ((curtime - i->lastBulletTime) > 0.5)
	{
	    //std::cout << "time to make a bullet\n";

	    i->lastBulletTime = curtime;
	    
	    sf::Vector2f dir = sprite::player.GetPosition() - i->sprite.GetPosition();
	    dir.x *= (rand() % 10) - 5;
	    dir.y *= (rand() % 10) - 5;

	    //std::cout << "dir is" << dir.x << " " << dir.y << std::endl;
	    float mag = sqrt(distsq(dir, sf::Vector2f(0,0)));

	    Bullet b;
	    b.xvel = dir.x * game::BULLET_SPEED / mag;
	    b.yvel = dir.y * game::BULLET_SPEED / mag;
	    b.sprite = sprite::bullet;
	    b.sprite.SetPosition(i->sprite.GetPosition());
	    b.origin = BADDIE;
	    game::bullets.push_back(b);
	}
    }

    // Remove dead baddies
    game::baddies.erase(std::remove_if(game::baddies.begin(),
					 game::baddies.end(),
					 isThingDead),
			  game::baddies.end());
					 
        // Remove dead baddies
    game::baddies.erase(std::remove_if(game::baddies.begin(),
					 game::baddies.end(),
					 isOffScreen),
			  game::baddies.end());
					 

}


void checkKeyEvent(const sf::Event& e)
{
    if (e.Type == sf::Event::KeyReleased)
    {
	if (e.Key.Code == sf::Key::Space)
	{
	       Bullet b;
	       b.xvel = game::BULLET_SPEED;
	       b.yvel = 0;
 	       b.sprite = sprite::bullet;
	       b.sprite.SetPosition(sprite::player.GetPosition());
	       b.origin = PLAYER;
	       game::bullets.push_back(b);
	}
	// if (e.Key.Code == sf::Key::Z)
	// {
	//     Bullet b;
	//     b.xvel = game::BULLET_SPEED * -1;
	//     b.yvel = 0;
	//     b.sprite = sprite::bullet;
	//     b.sprite.SetPosition(sprite::baddie.GetPosition());
	//     game::bullets.push_back(b);
	// }

    }

    
}


void fireSG()
{
    for (int i = 0; i != 25; ++i)
    {
	sf::Vector2f dir;
	dir.x = 10;
	dir.y = (rand() % 10) - 5;

	//std::cout << "dir is" << dir.x << " " << dir.y << std::endl;
	float mag = sqrt(distsq(dir, sf::Vector2f(0,0)));

	Bullet b;
	b.xvel = dir.x * game::BULLET_SPEED / mag;
	b.yvel = dir.y * game::BULLET_SPEED / mag;
	b.sprite = sprite::bullet;
	b.sprite.SetPosition(sprite::player.GetPosition());
	b.origin = PLAYER;
	game::bullets.push_back(b);
    }
}

void updateInput(const sf::Input& input)
{
    if (input.IsKeyDown(sf::Key::Left))
    {
	if (sprite::player.GetPosition().x > 0)
	    sprite::player.Move(-1*game::MOVE_RATE,0);
    }
    if (input.IsKeyDown(sf::Key::Right))
    {
      if (sprite::player.GetPosition().x < 1280)
      {
	sprite::player.Move(1*game::MOVE_RATE,0);
      }
    }
    if (input.IsKeyDown(sf::Key::Up))
    {
	if (sprite::player.GetPosition().y > 0)
	{
	    sprite::player.Move(0,-1*game::MOVE_RATE);
	}
    }
    if (input.IsKeyDown(sf::Key::Down))
    {
	if (sprite::player.GetPosition().y < 1024)
	{
	    sprite::player.Move(0,game::MOVE_RATE);
	}
    }
    if (input.IsKeyDown(sf::Key::Z))
    {
	float curtime = game::clock.GetElapsedTime();
	if (curtime - game::playerSGTime > game::SG_COOLDOWN)
	{
	    fireSG();
	    game::playerSGTime = curtime;
	}
    }


    // if (input.IsKeyDown(sf::Key::A))
    // {
    // 	if (sprite::baddie.GetPosition().x > 0)
    // 	    sprite::baddie.Move(-1*game::MOVE_RATE,0);
    // }
    // if (input.IsKeyDown(sf::Key::D))
    // {
    // 	sprite::baddie.Move(1*game::MOVE_RATE,0);
    // }
    // if (input.IsKeyDown(sf::Key::W))
    // {
    // 	sprite::baddie.Move(0,-1*game::MOVE_RATE);
    // }
    // if (input.IsKeyDown(sf::Key::S))
    // {
    // 	sprite::baddie.Move(0,game::MOVE_RATE);
    // }

    if (input.IsKeyDown(sf::Key::K))
    {
     	std::cout << "Bullets size" << game::bullets.size() << std::endl;
     	std::cout << "Baddies are " << game::baddies.size() << std::endl;
    }


}


int main()
{
    sf::RenderWindow app(sf::VideoMode(1280, 1024, 32), "SFML Window");
    const sf::Input& input = app.GetInput();


    app.SetFramerateLimit(60);
    
    game::clock.Reset();

    loadResources();


	// Start main loop
    bool running = true;
    while (running)
    {
	sf::Event event;
	while (app.GetEvent(event))
	{
	    // Window closed:
	    if (event.Type == sf::Event::Closed)
		running = false;

	    // Escape key pressed
	    if ((event.Type == sf::Event::KeyPressed) && 
		(event.Key.Code == sf::Key::Escape))
	    {
		running = false;
	    }

	    checkKeyEvent(event);
	    
	}

	if (game::state == game::ShowingSplash)
	{
	    drawSplashScreen(app);
	    game::state = game::Playing;
	}
	else if (game::state == game::Playing)
	{
	    app.Clear();
	    updateInput(input);
	    updateObjects();

	    drawObjects(app);
	    
	}
	else if (game::state == game::Dead)
	{
	    sf::String yourDead("gg:  Your score was" + toString(game::playerScore), game::gameFont, 52);
	    app.Draw(yourDead);
	    app.Display();
	    while (running)
	    {
		sf::Event event;
		while(app.GetEvent(event))
		{
		    if(event.Type == sf::Event::MouseButtonPressed
		       || event.Type == sf::Event::Closed )
		    {
			game::state = game::Over;
			running = false;
		    }
		    
		}
	    }
	}

        app.Display();
    }

    std::cout << game::bullets.size();
    std::cout << game::baddies.size();

    return EXIT_SUCCESS;
}

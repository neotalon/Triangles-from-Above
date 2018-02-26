#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"
#include <iostream>
#include <list>
#include <vector>

using namespace oxygine;
using namespace std;

//it is our resources
//in real project you would have more than one Resources declarations. It is important on mobile devices with limited memory and you would load/unload them
Resources gameResources;

//DECLARE_SMART is helper, it does forward declaration and declares intrusive_ptr typedef for your class
DECLARE_SMART(MainActor, spMainActor);


const float SCALE = 100.0f;
int score = 0;
//list<spTriangle> triangleList; //unknown size, valid template type?

//spTriangle triangleArray[256];
//std::vector<spTriangle> triangleArray;

b2Vec2 convert(const Vector2& pos)
{
    return b2Vec2(pos.x / SCALE, pos.y / SCALE);
}

Vector2 convert(const b2Vec2& pos)
{
    return Vector2(pos.x * SCALE, pos.y * SCALE);
}

// Circles
DECLARE_SMART(Circle, spCircle);
class Circle : public Sprite
{
public:
    Circle(b2World* world, const Vector2& pos, float scale = 1)
    {
        setResAnim(gameResources.getResAnim("circle")); //set the sprice for the object
        setAnchor(Vector2(0.5f, 0.5f));
        setTouchChildrenEnabled(false);

        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = convert(pos);

        b2Body* body = world->CreateBody(&bodyDef);

        setUserData(body);

        setScale(scale);

        b2CircleShape shape; //could be where the shape is defined
        shape.m_radius = getWidth() / SCALE / 2 * scale;

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.6f; //increased from 0.3f

        body->CreateFixture(&fixtureDef);
        body->SetUserData(this);
    }
};

//Triangles
DECLARE_SMART(Triangle, spTriangle);
class Triangle : public Sprite
{
public:
	Triangle(b2World* world, const Vector2& pos, float scale = 1)
	{
		bool alive = true;
		
		setResAnim(gameResources.getResAnim("triangle")); //set the sprice for the object
		setAnchor(Vector2(0.5f, 0.5f)); //was 0.5f, 0.5f
		setTouchChildrenEnabled(false);

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position = convert(pos);

		b2Body* body = world->CreateBody(&bodyDef);

		setUserData(body);

		setScale(scale);

		//b2PolygonShape shape;
		//b2CircleShape shape; //could be where the shape is defined

		//b2FixtureDef fixtureDef; //again below
		b2PolygonShape shape;
		b2Vec2 vertices[3];

		//1, 1, is too large
		vertices[0].Set(0, -0.5);
		vertices[1].Set(0.55, 0.5); // made wider to be same-length
		vertices[2].Set(-0.55, 0.5); // made wider to be same-length
		shape.Set(vertices, 3);
		//fixtureDef.shape = &shape; //again below

		shape.m_radius = getWidth() / SCALE / 20 * scale; // /2, later / 8

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 10.0f; //increased from 1f
		fixtureDef.friction = 15.0f; //increased from 0.3f

		//parenting?


		//Gravity
		body->SetGravityScale(0);

		body->CreateFixture(&fixtureDef);
		body->SetUserData(this);
	}
};

//Pentagon Background
DECLARE_SMART(Pentagon, spPentagon);
class Pentagon : public Sprite
{
public:
	Pentagon(b2World* world, const Vector2& pos, float scale = 1)
	{
		setResAnim(gameResources.getResAnim("pentagon")); //set the sprice for the object
		setAnchor(Vector2(0.5f, 0.5f)); //was 0.5f, 0.5f
		setTouchChildrenEnabled(true); //was false

		b2BodyDef bodyDef;
		bodyDef.type = b2_staticBody; //b2_dynamicBody
		bodyDef.position = convert(pos);

		b2Body* body = world->CreateBody(&bodyDef);

		setUserData(body);
		setScale(scale);

		b2PolygonShape shape;
		b2Vec2 vertices[5];

		//1, 1, is too large
		vertices[0].Set(0, -0.5);
		vertices[1].Set(0.55, 0.5); 
		vertices[2].Set(1, 1);
		vertices[3].Set(3, -0.5);
		vertices[4].Set(0, 0.5);
		shape.Set(vertices, 5);
		//fixtureDef.shape = &shape; //again below

		shape.m_radius = getWidth() / SCALE / 20 * scale; // /2, later / 8

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 10.0f; //increased from 1f
		fixtureDef.friction = 15.0f; //increased from 0.3f

									 //parenting?


									 //Gravity
		body->SetGravityScale(0);

		//body->CreateFixture(&fixtureDef); /7was
		body->SetUserData(this);
	}
};

DECLARE_SMART(Static, spStatic);
class Static : public Box9Sprite
{
public:
    Static(b2World* world, const RectF& rc)
    {
        //setHorizontalMode(Box9Sprite::TILING_FULL);
        //setVerticalMode(Box9Sprite::TILING_FULL);
        setResAnim(gameResources.getResAnim("pen"));
        setSize(rc.getSize());
        setPosition(rc.getLeftTop());
        setAnchor(Vector2(0.5f, 0.5f));

        b2BodyDef groundBodyDef;
        groundBodyDef.position = convert(getPosition());

        b2Body* groundBody = world->CreateBody(&groundBodyDef);

        b2PolygonShape groundBox;
        b2Vec2 sz = convert(getSize() / 2);
        groundBox.SetAsBox(sz.x, sz.y);
        groundBody->CreateFixture(&groundBox, 0.0f);
    }
};

class MainActor: public Actor
{
public:
    b2World* _world;
    spBox2DDraw _debugDraw;

    MainActor(): _world(0)
    {
        /// BUTTONS AND UI

		setSize(getStage()->getSize());

		//debug button
        /*
		spButton btn = new Button;
        btn->setX(getWidth() - btn->getWidth() - 3);
        btn->setY(3);
        btn->attachTo(this);
        btn->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MainActor::showHideDebug));
		*/

		//gravity button
		spButton gravButton = new Button;
		gravButton->setX(getWidth() - gravButton->getWidth() - 5);
		gravButton->setY(70);
		gravButton->attachTo(this);
		gravButton->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MainActor::enableGravity));

		spTextField gravButtonText = new TextField();
		gravButtonText->attachTo(getStage());
		gravButtonText->setPosition(gravButton->getPosition() + Vector2(-120.0f, 6.0f));
		gravButtonText->setFontSize(30);
		gravButtonText->setPriority(32000); //Ensures the text is drawn on top
		gravButtonText->setText("Gravity");


		//restart button
		spButton restartButton = new Button;
		restartButton->setX(getWidth() - restartButton->getWidth() - 5);
		restartButton->setY(140);
		restartButton->attachTo(this);
		restartButton->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MainActor::restart));

		spTextField restartButtonText = new TextField();
		restartButtonText->attachTo(getStage());
		restartButtonText->setPosition(restartButton->getPosition() + Vector2(-120.0f, 6.0f));
		restartButtonText->setFontSize(30);
		restartButtonText->setPriority(32000); //Ensures the text is drawn on top
		restartButtonText->setText("Restart");


		//next level button
		spButton nextLevelButton = new Button;
		nextLevelButton->setX(getWidth() - restartButton->getWidth() - 5);
		nextLevelButton->setY(210);
		nextLevelButton->attachTo(this);
		nextLevelButton->addEventListener(TouchEvent::CLICK, CLOSURE(this, &MainActor::nextLevel));

		spTextField levelButtonText = new TextField();
		levelButtonText->attachTo(getStage());
		levelButtonText->setPosition(nextLevelButton->getPosition() + Vector2(-120.0f, 6.0f));
		levelButtonText->setFontSize(30);
		levelButtonText->setPriority(32000); //Ensures the text is drawn on top
		levelButtonText->setText("Level");




		//Write some misc text on the screen

		//ResFont *font = gameResources.getResFont("Roboto-Black");
		spTextField text = new TextField();
		text->attachTo(getStage());
		text->setPosition(Vector2(50.0f, 8.0f));
		//text->setFont(font);
		text->setFontSize(30);
		text->setText("Level 1: Spawn Triangles by clicking");


		//Event listener for clicking
        addEventListener(TouchEvent::CLICK, CLOSURE(this, &MainActor::click));


        _world = new b2World(b2Vec2(0, 10));


        spStatic ground = new Static(_world, RectF(getWidth() / 2, getHeight() - 10, getWidth() - 100, 30));
        addChild(ground);

		//Adds a pentagon at the beginning of the game?
		//spPentagon pentagon = new Pentagon(_world, getSize() / 2 + Vector2(0.0f, 92.0f), 1);
		//addChild(pentagon);
    }

	//Update loop for the world
    void doUpdate(const UpdateState& us)
    {
        //in real project you should make steps with fixed dt, check box2d documentation
        _world->Step(us.dt / 1000.0f, 6, 2);

        //update each body position on display
        b2Body* body = _world->GetBodyList();
        while (body)
        {
            Actor* actor = (Actor*)body->GetUserData();
            b2Body* next = body->GetNext();
            if (actor)
            {
                const b2Vec2& pos = body->GetPosition();
                actor->setPosition(convert(pos));
                actor->setRotation(body->GetAngle());

                //remove fallen bodies (outside the game view)
                if (actor->getY() > getHeight() + 50)
                {
                    body->SetUserData(0);
                    _world->DestroyBody(body);

                    actor->detach();
                }
            }

            body = next;
        }
    }

	//Used for showing and hiding information about debug
    void showHideDebug(Event* event)
    {
        TouchEvent* te = safeCast<TouchEvent*>(event);
        te->stopsImmediatePropagation = true;
        if (_debugDraw)
        {
            _debugDraw->detach();
            _debugDraw = 0;
            return;
        }

        _debugDraw = new Box2DDraw;
        _debugDraw->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
        _debugDraw->attachTo(this);
        _debugDraw->setWorld(SCALE, _world);
        _debugDraw->setPriority(1);
    }

	//Enable gravity event
	void enableGravity(Event* event)
	{
		//Get a list of all physics objects		
		b2Body* body = _world->GetBodyList();
		
		while (body)
		{
			b2Body* next = body->GetNext();
			body->SetGravityScale(1);
			body->ApplyForceToCenter(b2Vec2(1, 1), true);

			//go to the next body in the list
			body = next;
		}

	}

	//restart event
	void restart(Event* event)
	{
		b2Body* body = _world->GetBodyList();
		
		while (body)
		{
			Actor* actor = (Actor*)body->GetUserData();
			b2Body* next = body->GetNext();
			if (actor)
			{
				//remove all triangles (restart the level)
				if (actor->getY() < getHeight() -50)
				{
					//body->ApplyForceToCenter(b2Vec2(std::rand() * 10000.0f, std::rand() * -10000.0f), true);
					body->SetUserData(0);
					_world->DestroyBody(body);

					actor->detach();
				}
			}

			body = next;
		}
	}

	//next level event
	void nextLevel(Event* event)
	{
		
	}

    void click(Event* event)
    {
        TouchEvent* te = safeCast<TouchEvent*>(event);

		if (event->target.get() == this)
		{
			//spawn a circle
			//spCircle circle = new Circle(_world, te->localPosition);
			//circle->attachTo(this);

			//spawn a triangle
			spTriangle triangle = new Triangle(_world, te->localPosition);
			//triangleArray.push_back(triangle); //adds the triangle to the end of the list			
			triangle->attachTo(this);
        }

        if (event->target->getUserData())
        {
            //shot to triangle: this deletes the triangle
			spActor actor = safeSpCast<Actor>(event->target);
			b2Body* body = (b2Body*)actor->getUserData();

			body->SetUserData(0);
			_world->DestroyBody(body);

			actor->detach();

            /*spActor actor = safeSpCast<Actor>(event->target);
            b2Body* body = (b2Body*)actor->getUserData();

            Vector2 dir = actor->getPosition() - te->localPosition;
            dir = dir / dir.length() * body->GetMass() * 200;

            body->ApplyForceToCenter(b2Vec2(dir.x, dir.y), true);

            spSprite sprite = new Sprite();
            sprite->setResAnim(gameResources.getResAnim("shot"));
            Vector2 local = actor->parent2local(te->localPosition);
            sprite->setPosition(local);
            sprite->setAnchor(Vector2(0.5f, 0.5f));
            sprite->attachTo(actor);*/
        }
    }
};

//MOVED TRIANGLE TO HERE




void example_preinit()
{
}

void example_init()
{
    //load xml file with resources definition
    gameResources.loadXML("res.xml");

    //lets create our client code simple actor
    //prefix 'sp' here means it is intrusive Smart Pointer
    //it would be deleted automatically when you lost ref to it
    spMainActor actor = new MainActor;
    //and add it to Stage as child
    getStage()->addChild(actor);


	//background
	spSprite background = new Sprite;
	background->setResAnim(gameResources.getResAnim("level1"));
	background->setAnchor(Vector2(0.5f, 1.0f));
	background->setPosition(Vector2(480, 610.0f));
	background->attachTo(actor);
}

void example_destroy()
{
    gameResources.free();
}

void example_update()
{

}
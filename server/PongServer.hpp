#ifndef PONG_SERVER_HPP_
#define PONG_SERVER_HPP_

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <ctime>
#include <limits.h>

#include "Renderer.hpp"


// TODO add function:
/*

   *
  *
|*
| *
|  *



|
|
|*
  *
   *

*/


typedef boost::shared_ptr<boost::asio::ip::tcp::tcp::socket> sockptr;
typedef boost::asio::ip::tcp btcp;

class PongServer {
public:
	enum CollisionState { P1_SCORE, P2_SCORE, CHANGE_DIRECTION, NONE };

	typedef struct Player_t {
		sockptr s;
		uint16_t pos;
		uint16_t size;
		fullcircle::RGB_t color;
	} Player;

	typedef struct Ball_t {
		enum Movement { STRAIGHT, DOWN, UP };
		uint16_t x;
		uint16_t y;
		Movement move;
		bool direction;
		uint8_t speed;
		fullcircle::RGB_t color;
	} Ball;

	PongServer(uint16_t width, uint16_t height, uint16_t fps);
	virtual ~PongServer();

	void waitForPlayers(uint16_t port);

	void startGame(btcp::resolver::iterator* iterator);

protected:
	void _setupPlayer(Player& p, sockptr s);
	void _handlePlayer(Player* p); // TODO use reference (why doesn't it work with boost::bind?!)
	void _setupBall();
	void _initRenderer(btcp::resolver::iterator* iterator);
	void _unInitRenderer();
	void _mainLoop();
	CollisionState _checkCollision();
	void _moveBall();
	void _tick();
	void _changeBallMovement();
	void _score(bool who);
	long _msdiff(struct timeval* t1, struct timeval* t2);
	void _drawBackground();
	void _drawPlayers();
	void _drawBall();

	uint16_t _width;
	uint16_t _height;
	uint16_t _fps;

	Player _p1;
	Player _p2;
	Ball _ball;
	Renderer* _render;
	boost::asio::io_service _io_service;
};


#endif // PONG_SERVER_HPP_

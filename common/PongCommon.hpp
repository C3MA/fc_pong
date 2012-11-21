#ifndef PONG_COMMON_HP_
#define PONG_COMMON_HP_

#include <libfullcircle/common.hpp>
#include <boost/asio.hpp>

typedef boost::shared_ptr<boost::asio::ip::tcp::tcp::socket> sockptr;

typedef struct Player_t {
	sockptr s;
	uint16_t pos;
	uint16_t size;
	fullcircle::RGB_t color;
	const char* name;
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

typedef struct PongState_t {
	uint16_t pos1;
	uint16_t pos2;
	uint16_t size1;
	uint16_t size2;
	uint16_t ballx;
	uint16_t bally;
	bool direction;
	Ball::Movement move;
} PongState;

#endif // PONG_COMMON_HP_

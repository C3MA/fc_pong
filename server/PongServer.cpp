#include "PongServer.hpp"

PongServer::PongServer(uint16_t width, uint16_t height, uint16_t fps) : _width(width), _height(height), _fps(fps), _render(NULL) { }

PongServer::~PongServer() { }

void PongServer::waitForPlayers(uint16_t port) {
	btcp::acceptor acp(_io_service, btcp::endpoint(btcp::v4(), port));

	sockptr s1(new btcp::socket(_io_service));
	acp.accept(*s1);
	_setupPlayer(_p1, s1);
	boost::thread t1(&PongServer::_handlePlayer, this, &_p1);

	sockptr s2(new btcp::socket(_io_service));
	acp.accept(*s2);
	_setupPlayer(_p2, s2);
	boost::thread t2(&PongServer::_handlePlayer, this, &_p2);
}


void PongServer::_setupPlayer(Player& p, sockptr s){
	p.s = s;
	p.pos = _height / 2;
	p.size = _width / 3;
	p.color = fullcircle::WHITE;
}

void PongServer::_handlePlayer(Player* p){
	size_t len;
	char cmds[16];

	try {
		for(;;) {
			len = p->s->read_some(boost::asio::buffer(cmds));
			for(size_t i = 0; i < len; i++) {
				switch(cmds[i]){
				case '+':
					std::cout << "Received a + command" << std::endl;
					if(_height >= (p->pos + p->size)){
						std::cout << "Executed a + command" << std::endl;
						p->pos++;
					}
					break;
				case '-':
					if(p->pos > 0)
						p->pos--;
					break;
				case '?':
					// TODO
					break;
				}
			}
		}
	}catch(std::exception &e){
		std::cerr << "Handling a client failed: " << e.what() << std::endl;
	}
}

void PongServer::_initRenderer(btcp::resolver::iterator* iterator) {
	_render = new Renderer(&_io_service, iterator, _width, _height, _fps);
	
	_render->run();
	boost::thread t(boost::bind(&boost::asio::io_service::run, &_io_service));
}

void PongServer::startGame(btcp::resolver::iterator* iterator) {
	std::cout << "Initializing renderer" << std::endl;
	_initRenderer(iterator);

	while(!_render->canRender())
		usleep(100 * 1000);

	_mainLoop();

	_unInitRenderer();
}

void PongServer::_unInitRenderer() {
	if(_render){
		delete _render;
		_render = NULL;
	}
}

void PongServer::_setupBall() {
	_ball.x = 1;
	_ball.y = _p1.pos + (_p1.size / 2);
	_ball.move = Ball::STRAIGHT;
	_ball.speed = 1;
	_ball.direction = true;
	_ball.color = fullcircle::WHITE;
}

void PongServer::_moveBall() {
	if(_ball.direction)
		_ball.x++;
	else
		_ball.x--;

	switch(_ball.move){
	case Ball::DOWN:
		_ball.y++;
		break;
	case Ball::UP:
		_ball.y--;
		break;
	case Ball::STRAIGHT:
		// Y-Value is right
		break;		
	}
}

void PongServer::_changeBallMovement() {
	if(_ball.y == 0 && _ball.move == Ball::UP)
		_ball.move = Ball::DOWN;
	if(_ball.y == (_height - 1) && _ball.move == Ball::DOWN)
		_ball.move = Ball::UP;
}

PongServer::CollisionState PongServer::_checkCollision() {
	uint16_t next_y;

	if(_ball.x == 1 && !_ball.direction) {
		switch(_ball.move) {
		case Ball::DOWN:
			next_y = _ball.y + 1;
			break;
		case Ball::UP:
			next_y = _ball.y - 1;
			break;
		case Ball::STRAIGHT:
			break;
		}
		if( (_p1.pos <= next_y) && ( (_p1.pos + _p1.size - 1) >= next_y) )
			return CHANGE_DIRECTION;
		else
			return P2_SCORE;
	}

	if((_ball.x == _width - 2) && _ball.direction) {
		switch(_ball.move) {
		case Ball::DOWN:
			next_y = _ball.y + 1;
			break;
		case Ball::UP:
			next_y = _ball.y - 1;
			break;
		case Ball::STRAIGHT:
			next_y = _ball.y;
			break;
		}
		if( (_p2.pos <= next_y) && ( (_p2.pos + _p2.size - 1) >= next_y) )
			return CHANGE_DIRECTION;
		else
			return P1_SCORE;
	}

	return NONE;
}

void PongServer::_tick() {
	_changeBallMovement();
	switch(_checkCollision()) {
	case P1_SCORE:
		_score(false);
		break;
	case P2_SCORE:
		_score(true);
		break;
	case CHANGE_DIRECTION:
		_ball.direction = !_ball.direction;
		if(_ball.move == Ball::STRAIGHT)
			_ball.move = Ball::DOWN; // TODO rand
		break;
	case NONE:
		break;
	}
	_moveBall();
	if(_ball.speed != UCHAR_MAX)
		_ball.speed++;
}

void PongServer::_score(bool who) {
	std::cout << "SCOOOOOOOREEEEEE" << std::endl;
	_render->fillWhole(fullcircle::YELLOW);
	_drawPlayers();
	if(!who)
		_render->drawRect(_width - 1, 0, 1, _height, fullcircle::WHITE);
	else
		_render->drawRect(0, 0, 1, _height, fullcircle::WHITE);
	_render->drawScene();
	usleep( 5 * 1000 * 1000 );
	_setupBall(); // TODO give it to the right player
}

long PongServer::_msdiff(struct timeval* t1, struct timeval* t2) {
	return ((t2->tv_sec - t1->tv_sec) * 1000 + (t2->tv_usec - t1->tv_usec)/1000.0) + 0.5;
}

void PongServer::_drawBackground() {
	_render->fillWhole(fullcircle::GREEN);
}

void PongServer::_drawPlayers() {
	_render->drawRect(0, _p1.pos, 1, _p1.size, _p1.color);
	_render->drawRect(_width - 1, _p2.pos, 1, _p2.size, _p2.color);
}

void PongServer::_drawBall() {
	_render->setPixel(_ball.x, _ball.y, fullcircle::YELLOW);
}

void PongServer::_mainLoop() {
	struct timeval start, current;

	_setupBall();
	while(1){
		gettimeofday(&start, NULL);
		do{
			_drawBackground();
			_drawPlayers();
			_drawBall();
			_render->drawScene();
			usleep(1000);
			gettimeofday(&current, NULL);
		}while(_msdiff(&start, &current) < (750 - _ball.speed * 2));

		_tick();
	}
}

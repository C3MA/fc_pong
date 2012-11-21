#include "Renderer.hpp"

Renderer::Renderer(boost::asio::io_service *client_io_service, boost::asio::ip::tcp::resolver::iterator *iterator, uint16_t width, uint16_t height, uint16_t fps) 
	: Client(client_io_service, iterator),
	_width(width),
	_height(height),
	_fps(fps),
	_can_render(false) {
	initBuffers();
}

Renderer::~Renderer() { }

void Renderer::idle() {
	fullcircle::BinarySequenceMetadata* meta = new fullcircle::BinarySequenceMetadata();
	meta->set_frames_per_second(_fps);
	meta->set_width(_width);
	meta->set_height(_height);
	meta->set_generator_name(RENDERER_NAME);
	meta->set_generator_version(RENDERER_VERSION);

	// TODO what's the color ?
	send_request("blue", 14, meta);
}

void Renderer::ack() { }

void Renderer::nack() { }

void Renderer::start() {
	double tps = 1000 / _fps;
	_can_render = true;
	for(;;) {
		_dispatcher->send_frame(_frontBuffer);
		usleep(tps * 1000);
	}
	_state = fullcircle::IDLE;
}

void Renderer::initBuffers() {
	// TODO assign directly !
	fullcircle::Frame::Ptr fb(new fullcircle::Frame(_width, _height));
	fullcircle::Frame::Ptr bb(new fullcircle::Frame(_width, _height));
	_frontBuffer = fb;
	_frontBuffer->fill_whole(fullcircle::BLACK);
	_backBuffer = bb;
	_backBuffer->set_pixel_window(0, 0, _frontBuffer);
}

void Renderer::swapBuffers() {
	fullcircle::Frame::Ptr tmp;
	tmp = _frontBuffer;
	_frontBuffer = _backBuffer;
	_backBuffer = tmp;
}

void Renderer::drawScene() {
	swapBuffers();
}

void Renderer::setPixel(uint16_t x, uint16_t y, const fullcircle::RGB_t& c){
	_backBuffer->set_pixel(x, y, c);
}

void Renderer::drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const fullcircle::RGB_t& c){
	for(uint16_t ix = 0; ix < width; ix++)
		for(uint16_t iy = 0; iy < height; iy++)
			setPixel(x + ix, y + iy, c);
}

void Renderer::fillWhole(const fullcircle::RGB_t& c) {
	_backBuffer->fill_whole(c);
}

bool Renderer::canRender() {
	return _can_render;
}

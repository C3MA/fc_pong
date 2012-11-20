#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include <libfullcircle/common.hpp>
#include <libfullcircle/net/client.hpp>

#define RENDERER_NAME		"Fullcircle Live Renderer"
#define RENDERER_VERSION	"0.1"

class Renderer : public fullcircle::Client {
public:
	Renderer(boost::asio::io_service *client_io_service, boost::asio::ip::tcp::resolver::iterator *iterator, uint16_t width, uint16_t height, uint16_t fps);

	virtual ~Renderer();

	void idle();
	virtual void ack();
	virtual void nack();
	virtual void start();

	void initBuffers();

	void swapBuffers();
	void drawScene();

	void setPixel(uint16_t x, uint16_t y, const fullcircle::RGB_t& c);
	void drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const fullcircle::RGB_t& c);
	void fillWhole(const fullcircle::RGB_t& c);

	bool canRender();

protected:
	uint16_t _width;
	uint16_t _height;
	uint16_t _fps;
	bool _can_render;

	fullcircle::Frame::Ptr _frontBuffer;
	fullcircle::Frame::Ptr _backBuffer;
};

#endif // RENDERER_HPP_

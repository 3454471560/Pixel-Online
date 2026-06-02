#include<Render/Backend/SDL/SDLRenderer.h>

Online::Render::Renderer* Online::Render::Renderer::Factory::Create(Online::Render::API api)
{
	switch (api)
	{
	case Online::Render::API::SDL2D:
		return ONLINE_NEW(Online::Render::SDLRenderer);

	case Online::Render::API::Unknown:
		throw std::exception("Unknown Render Backend API");

	default:
		throw std::exception("Unknown Render Backend API");
	}
}
#include <Window/Backend/SDL/SDLWindow.h>
#include <SDL_image.h>

namespace Online::Window
{
	bool SDLWindow::Initialize(int w, int h, const char* title)
	{
		width = w;
		height = h;

		if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
		{
			if (SDL_Init(SDL_INIT_VIDEO) != 0) { return false; }
		}

		RegisterKeyMap();

		const Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
		window = SDL_CreateWindow(
			title ? title : "SDLWindow",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			width, height, flags
		);

		if (!window) { return false; }
		SDL_StartTextInput();
		shouldClose = false;
		return true;
	}

	void SDLWindow::Release()
	{
		SDL_StopTextInput();
		if (window)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
		}
	}

	void SDLWindow::CloseWindow()
	{
		shouldClose = true;
		EmitClose();
	}

	bool SDLWindow::IsClose() const { return shouldClose; }
	void* SDLWindow::GetNativeWindow() const { return window; }

	void SDLWindow::ResetSize(int w, int h)
	{
		width = w;
		height = h;
		if (window) { SDL_SetWindowSize(window, width, height); }
		EmitResize(width, height);
	}

	void SDLWindow::RegisterKeyMap() noexcept
	{
		std::fill(std::begin(map), std::end(map), Online::Input::KeyCode::Unknown);

		map[SDL_SCANCODE_0] = Online::Input::KeyCode::Key0;
		map[SDL_SCANCODE_1] = Online::Input::KeyCode::Key1;
		map[SDL_SCANCODE_2] = Online::Input::KeyCode::Key2;
		map[SDL_SCANCODE_3] = Online::Input::KeyCode::Key3;
		map[SDL_SCANCODE_4] = Online::Input::KeyCode::Key4;
		map[SDL_SCANCODE_5] = Online::Input::KeyCode::Key5;
		map[SDL_SCANCODE_6] = Online::Input::KeyCode::Key6;
		map[SDL_SCANCODE_7] = Online::Input::KeyCode::Key7;
		map[SDL_SCANCODE_8] = Online::Input::KeyCode::Key8;
		map[SDL_SCANCODE_9] = Online::Input::KeyCode::Key9;

		map[SDL_SCANCODE_A] = Online::Input::KeyCode::A;
		map[SDL_SCANCODE_B] = Online::Input::KeyCode::B;
		map[SDL_SCANCODE_C] = Online::Input::KeyCode::C;
		map[SDL_SCANCODE_D] = Online::Input::KeyCode::D;
		map[SDL_SCANCODE_E] = Online::Input::KeyCode::E;
		map[SDL_SCANCODE_F] = Online::Input::KeyCode::F;
		map[SDL_SCANCODE_G] = Online::Input::KeyCode::G;
		map[SDL_SCANCODE_H] = Online::Input::KeyCode::H;
		map[SDL_SCANCODE_I] = Online::Input::KeyCode::I;
		map[SDL_SCANCODE_J] = Online::Input::KeyCode::J;
		map[SDL_SCANCODE_K] = Online::Input::KeyCode::K;
		map[SDL_SCANCODE_L] = Online::Input::KeyCode::L;
		map[SDL_SCANCODE_M] = Online::Input::KeyCode::M;
		map[SDL_SCANCODE_N] = Online::Input::KeyCode::N;
		map[SDL_SCANCODE_O] = Online::Input::KeyCode::O;
		map[SDL_SCANCODE_P] = Online::Input::KeyCode::P;
		map[SDL_SCANCODE_Q] = Online::Input::KeyCode::Q;
		map[SDL_SCANCODE_R] = Online::Input::KeyCode::R;
		map[SDL_SCANCODE_S] = Online::Input::KeyCode::S;
		map[SDL_SCANCODE_T] = Online::Input::KeyCode::T;
		map[SDL_SCANCODE_U] = Online::Input::KeyCode::U;
		map[SDL_SCANCODE_V] = Online::Input::KeyCode::V;
		map[SDL_SCANCODE_W] = Online::Input::KeyCode::W;
		map[SDL_SCANCODE_X] = Online::Input::KeyCode::X;
		map[SDL_SCANCODE_Y] = Online::Input::KeyCode::Y;
		map[SDL_SCANCODE_Z] = Online::Input::KeyCode::Z;

		map[SDL_SCANCODE_SPACE] = Online::Input::KeyCode::Space;
		map[SDL_SCANCODE_SEMICOLON] = Online::Input::KeyCode::Semicolon;
		map[SDL_SCANCODE_EQUALS] = Online::Input::KeyCode::Equal;
		map[SDL_SCANCODE_LEFTBRACKET] = Online::Input::KeyCode::LeftBracket;
		map[SDL_SCANCODE_RIGHTBRACKET] = Online::Input::KeyCode::RightBracket;
		map[SDL_SCANCODE_COMMA] = Online::Input::KeyCode::Comma;

		map[SDL_SCANCODE_ESCAPE] = Online::Input::KeyCode::Escape;
		map[SDL_SCANCODE_RETURN] = Online::Input::KeyCode::Enter;
		map[SDL_SCANCODE_TAB] = Online::Input::KeyCode::Tab;
		map[SDL_SCANCODE_BACKSPACE] = Online::Input::KeyCode::Backspace;
		map[SDL_SCANCODE_INSERT] = Online::Input::KeyCode::Insert;
		map[SDL_SCANCODE_DELETE] = Online::Input::KeyCode::Delete;

		map[SDL_SCANCODE_RIGHT] = Online::Input::KeyCode::Right;
		map[SDL_SCANCODE_LEFT] = Online::Input::KeyCode::Left;
		map[SDL_SCANCODE_DOWN] = Online::Input::KeyCode::Down;
		map[SDL_SCANCODE_UP] = Online::Input::KeyCode::Up;

		map[SDL_SCANCODE_PAGEUP] = Online::Input::KeyCode::PageUp;
		map[SDL_SCANCODE_PAGEDOWN] = Online::Input::KeyCode::PageDown;
		map[SDL_SCANCODE_HOME] = Online::Input::KeyCode::Home;
		map[SDL_SCANCODE_END] = Online::Input::KeyCode::End;
		map[SDL_SCANCODE_CAPSLOCK] = Online::Input::KeyCode::CapsLock;

		map[SDL_SCANCODE_F1] = Online::Input::KeyCode::F1;
		map[SDL_SCANCODE_F2] = Online::Input::KeyCode::F2;
		map[SDL_SCANCODE_F3] = Online::Input::KeyCode::F3;
		map[SDL_SCANCODE_F4] = Online::Input::KeyCode::F4;
		map[SDL_SCANCODE_F5] = Online::Input::KeyCode::F5;
		map[SDL_SCANCODE_F6] = Online::Input::KeyCode::F6;
		map[SDL_SCANCODE_F7] = Online::Input::KeyCode::F7;
		map[SDL_SCANCODE_F8] = Online::Input::KeyCode::F8;
		map[SDL_SCANCODE_F9] = Online::Input::KeyCode::F9;
		map[SDL_SCANCODE_F10] = Online::Input::KeyCode::F10;
		map[SDL_SCANCODE_F11] = Online::Input::KeyCode::F11;
		map[SDL_SCANCODE_F12] = Online::Input::KeyCode::F12;

		map[SDL_SCANCODE_LSHIFT] = Online::Input::KeyCode::LeftShift;
		map[SDL_SCANCODE_LCTRL] = Online::Input::KeyCode::LeftControl;
		map[SDL_SCANCODE_LALT] = Online::Input::KeyCode::LeftAlt;
		map[SDL_SCANCODE_LGUI] = Online::Input::KeyCode::LeftSuper;

		map[SDL_SCANCODE_RSHIFT] = Online::Input::KeyCode::RightShift;
		map[SDL_SCANCODE_RCTRL] = Online::Input::KeyCode::RightControl;
		map[SDL_SCANCODE_RALT] = Online::Input::KeyCode::RightAlt;
		map[SDL_SCANCODE_RGUI] = Online::Input::KeyCode::RightSuper;
	}

	Online::Input::KeyCode SDLWindow::ToMouseKeyCode(uint8_t button) noexcept
	{
		switch (button)
		{
		case SDL_BUTTON_LEFT:   return Online::Input::KeyCode::Mouse0;
		case SDL_BUTTON_RIGHT:  return Online::Input::KeyCode::Mouse1;
		case SDL_BUTTON_MIDDLE: return Online::Input::KeyCode::Mouse2;
		case SDL_BUTTON_X1:     return Online::Input::KeyCode::Mouse3;
		case SDL_BUTTON_X2:     return Online::Input::KeyCode::Mouse4;
		default:                return Online::Input::KeyCode::Unknown;
		}
	}

	void SDLWindow::EmitClose()
	{
		auto args = Online::Event::CloseEventArgs();
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::WindowClose, &args));
	}

	void SDLWindow::EmitResize(int w, int h)
	{
		auto args = Online::Event::FramebufferResetSizeEventArgs(w, h);
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::WindowFramebufferResetSize, &args));
	}

	void SDLWindow::EmitDropFile(std::vector<std::string> paths)
	{
		auto args = Online::Event::DropFileEventArgs(std::move(paths));
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::DropFile, &args));
	}

	void SDLWindow::EmitMouseMove(double x, double y, double xOffset, double yOffset)
	{
		auto args = Online::Event::MouseCurrsorMoveEventArgs(x, y, xOffset, yOffset);
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::MouseCurrsorMove, &args));
	}

	void SDLWindow::EmitMouseScroll(double x, double y)
	{
		auto args = Online::Event::MouseScrollEventArgs(x, y);
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::MouseScroll, &args));
	}

	void SDLWindow::EmitKey(int scancode, int action, int mods) const
	{
		const auto kc = IntToKeyCode(scancode);
		auto args = Online::Event::KeyEventArgs(kc, scancode, action, mods);
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::Key, &args));
	}

	void SDLWindow::EmitMouseButton(uint8_t button, int action, int mods) const
	{
		const auto bc = ToMouseKeyCode(button);
		auto args = Online::Event::MouseButtonEventArgs(bc, action, mods);
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::MouseButton, &args));
	}

	void SDLWindow::EmitWindowFocusGained()
	{
		auto args = Online::Event::WindowFocusEventArgs(true);
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::WindowFocusGained, &args));
	}

	void SDLWindow::EmitWindowFocusLost()
	{
		auto args = Online::Event::WindowFocusEventArgs(false);
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::WindowFocusLost, &args));
	}

	void SDLWindow::EmitMouseEnterWindow()
	{
		auto args = Online::Event::MouseWindowEnterLeaveEventArgs(true);
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::MouseEnterWindow, &args));
	}

	void SDLWindow::EmitMouseLeaveWindow()
	{
		auto args = Online::Event::MouseWindowEnterLeaveEventArgs(false);
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::MouseLeaveWindow, &args));
	}

	void SDLWindow::EmitTextInput(const char* text)
	{
		auto args = Online::Event::TextInputEventArgs(text);
		Online::Event::Emit(Online::Event::Event(Online::Event::EventType::TextInput, &args));
	}

	void SDLWindow::PollEvents()
	{
		SDL_Event e{};
		dropFilesTemp.clear();
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				shouldClose = true;
				EmitClose();
				break;

			case SDL_WINDOWEVENT:
				switch (e.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:// 与SDL_QUIT不同，当程序为多窗口时关闭其中一个窗口只会调用这里，关最后一个会调用SDL_QUIT
					shouldClose = true;
					EmitClose();
					break;

				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					width = (int)e.window.data1;
					height = e.window.data2;
					EmitResize(width, height);
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
				{
					EmitWindowFocusGained();
					break;
				}
				case SDL_WINDOWEVENT_FOCUS_LOST:
				{
					EmitWindowFocusLost();
					Online::Input::ResetAllState();
					break;
				}
				case SDL_WINDOWEVENT_ENTER:
				{
					EmitMouseEnterWindow();
					break;
				}
				case SDL_WINDOWEVENT_LEAVE:
				{
					EmitMouseLeaveWindow();
					Online::Input::ResetMouseState();
					break;
				}
				default:
					break;
				}
				break;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{	
				const int action = (e.type == SDL_KEYDOWN) ? 1 : 0;
				const int scancode = (int)e.key.keysym.scancode;
				const int mods = (int)e.key.keysym.mod;
				EmitKey(scancode, action, mods);
				break;
			}

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				const int action = (e.type == SDL_MOUSEBUTTONDOWN) ? 1 : 0;
				const int mods = (int)SDL_GetModState();
				EmitMouseButton(e.button.button, action, mods);
				break;
			}

			case SDL_MOUSEMOTION:
			{
				EmitMouseMove((double)e.motion.x, (double)e.motion.y,
					(double)e.motion.xrel, (double)e.motion.yrel);
				break;
			}
			case SDL_MOUSEWHEEL:
			{
				double x = (double)e.wheel.x;
				double y = (double)e.wheel.y;
				if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) { x = -x; y = -y; }
				EmitMouseScroll(x, y);
				break;
			}

			case SDL_DROPFILE:
			{
				if (e.drop.file) { dropFilesTemp.emplace_back(e.drop.file); }
				SDL_free(e.drop.file);
				break;
			}
			case SDL_TEXTINPUT:
			{
				EmitTextInput(e.text.text);
				break;
			}
			default:
				break;
			}
		}
	}
}

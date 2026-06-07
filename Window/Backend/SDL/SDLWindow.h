#pragma once

#include <Window/Frontend/Window.h>

#include <Event/Common/Event.h>
#include <Event/Common/EventArgs.h>
#include <Event/Common/EventType.h>
#include <Event/Common/FuncTable.h>

#include <Input/Common/KeyCode.h>
#include <Input/Common/FuncTable.h>

#include <SDL.h>
#include <algorithm>
#include <string>
#include <vector>

namespace Online::Runtime { class Runtime; }

namespace Online::Window
{
	class SDLWindow : public Online::Window::Window
	{
		friend class Online::Window::Window;

	private:
		SDLWindow() = default;
		~SDLWindow() override = default;

	public:
		SDLWindow(const SDLWindow&) = delete;
		SDLWindow& operator=(const SDLWindow&) = delete;
		SDLWindow(SDLWindow&&) = delete;
		SDLWindow& operator=(SDLWindow&&) = delete;

	private:
		bool Initialize(int width, int height, const char* title) override;
		void Release() override;

	public:
		void PollEvents() override;
		void CloseWindow() override;

	public:
		bool IsClose() const override;
		void* GetNativeWindow() const override;

	public:
		void ResetSize(int width, int height);

	private:
		void RegisterKeyMap() noexcept;
		static Online::Input::KeyCode ToMouseKeyCode(uint8_t button) noexcept;

	private:
		static void EmitClose();
		static void EmitResize(int w, int h);
		static void EmitDropFile(std::vector<std::string> paths);
		static void EmitMouseMove(double x, double y, double xOffset, double yOffset);
		static void EmitMouseScroll(double x, double y);

		void EmitKey(int scancode, int action, int mods) const;
		void EmitMouseButton(uint8_t button, int action, int mods) const;
		void EmitTextEditing(const char* text, int start, int length);

		void EmitWindowFocusGained();
		void EmitWindowFocusLost();
		void EmitMouseEnterWindow();
		void EmitMouseLeaveWindow();
		void EmitTextInput(const char* text);

		void OnTextInputStart(const Online::Event::Event& event);
		void OnTextInputStop(const Online::Event::Event& event);
		void OnSetTextInputRect(const Online::Event::Event& event);

		static void OnTextInputStartThunk(void* listener, const Online::Event::Event& event);
		static void OnTextInputStopThunk(void* listener, const Online::Event::Event& event);
		static void OnSetTextInputRectThunk(void* listener, const Online::Event::Event& event);

	private:
		SDL_Window* window = nullptr;
		mutable bool shouldClose = false;
		std::vector<std::string> dropFilesTemp;

		Online::Event::EventToken textInputStartToken;
		Online::Event::EventToken textInputStopToken;
		Online::Event::EventToken textInputRectToken;
	};
}

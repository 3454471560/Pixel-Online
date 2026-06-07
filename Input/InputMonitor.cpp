#include <Input/InputMonitor.h>
#include <Log/Common/FuncTable.h>

bool Online::Input::InputMonitor::Initialize()
{
    keyToken = Online::Event::Subscribe(Online::Event::EventType::Key, &InputMonitor::OnKeyThunk, this);
    mouseButtonToken = Online::Event::Subscribe(Online::Event::EventType::MouseButton, &InputMonitor::OnMouseButtonThunk, this);
    mouseMoveToken = Online::Event::Subscribe(Online::Event::EventType::MouseCurrsorMove, &InputMonitor::OnMouseMoveThunk, this);
    textInputToken = Online::Event::Subscribe(Online::Event::EventType::TextInput, &InputMonitor::OnTextInputThunk, this);
    textEditingToken = Online::Event::Subscribe(Online::Event::EventType::TextEditing, &InputMonitor::OnTextEditingThunk, this);

    if (keyToken.type == Online::Event::EventType::Invalid ||
        mouseButtonToken.type == Online::Event::EventType::Invalid ||
        textInputToken.type == Online::Event::EventType::Invalid ||
        mouseMoveToken.type == Online::Event::EventType::Invalid ||
        textEditingToken.type == Online::Event::EventType::Invalid)
    {
        Online::Log::Warning("InputMonitor subscribe event failed");
        return false;
    }

    std::memset(currentFrameState, 0, sizeof(currentFrameState));
    std::memset(previousFrameState, 0, sizeof(previousFrameState));
    std::memset(hardwareState, 0, sizeof(hardwareState));
    return true;
}

void Online::Input::InputMonitor::Release()
{
    if (keyToken.type != Online::Event::EventType::Invalid)
    {
        Online::Event::UnSubscribe(keyToken);
        keyToken = Online::Event::EventToken{};
    }
    if (mouseButtonToken.type != Online::Event::EventType::Invalid)
    {
        Online::Event::UnSubscribe(mouseButtonToken);
        mouseButtonToken = Online::Event::EventToken{};
    }
    if (mouseMoveToken.type != Online::Event::EventType::Invalid)
    {
        Online::Event::UnSubscribe(mouseMoveToken);
        mouseMoveToken = Online::Event::EventToken{};
    }
    if (textInputToken.type != Online::Event::EventType::Invalid)
    {
        Online::Event::UnSubscribe(textInputToken);
        textInputToken = Online::Event::EventToken{};
    }
    if (textEditingToken.type != Online::Event::EventType::Invalid)
    {
        Online::Event::UnSubscribe(textEditingToken);
        textEditingToken = Online::Event::EventToken{};
    }
}

std::string Online::Input::InputMonitor::GetTextInputBuffer() noexcept
{
    std::string ret = std::move(textInputBuffer);
    textInputBuffer.clear();
    return ret;
}

std::string Online::Input::InputMonitor::GetCompositionText() const noexcept
{
    return compositionText;
}

int Online::Input::InputMonitor::GetCompositionCursor() const noexcept
{
    return compositionStart;
}

void Online::Input::InputMonitor::StartTextInput() noexcept
{
    Online::Event::TextInputStartEventArgs args;
    Online::Event::Emit(Online::Event::Event(Online::Event::EventType::TextInputStart, &args));
}

void Online::Input::InputMonitor::StopTextInput() noexcept
{
    Online::Event::TextInputStopEventArgs args;
    Online::Event::Emit(Online::Event::Event(Online::Event::EventType::TextInputStop, &args));
}

void Online::Input::InputMonitor::OnKey(const Online::Event::Event& event)
{
    if (event.type != Online::Event::EventType::Key) return;
    const Online::Event::KeyEventArgs& args = event.As<Online::Event::KeyEventArgs>();
    if (args.keycode == Online::Input::KeyCode::Unknown) return;
    const bool newState = (args.action != 0);
    const size_t index = static_cast<size_t>(args.keycode);
    if (hardwareState[index] != newState)
    {
        hardwareState[index] = newState;
    }
}

void Online::Input::InputMonitor::OnMouseButton(const Online::Event::Event& event)
{
    if (event.type != Online::Event::EventType::MouseButton) return;
    const Online::Event::MouseButtonEventArgs& args = event.As<Online::Event::MouseButtonEventArgs>();
    if (args.button == Online::Input::KeyCode::Unknown) return;
    const bool newState = (args.action != 0);
    const size_t index = static_cast<size_t>(args.button);
    if (hardwareState[index] != newState)
    {
        hardwareState[index] = newState;
    }
}

void Online::Input::InputMonitor::OnMouseMove(const Online::Event::Event& event)
{
    if (event.type != Online::Event::EventType::MouseCurrsorMove) return;
    const auto& args = event.As<Online::Event::MouseCurrsorMoveEventArgs>();
    mouseHardWareState.mouseX = args.xPosition;
    mouseHardWareState.mouseY = args.yPosition;
}

void Online::Input::InputMonitor::OnTextInput(const Online::Event::Event& event)
{
    if (event.type != Online::Event::EventType::TextInput) return;
    const auto& args = event.As<Online::Event::TextInputEventArgs>();
    textInputBuffer += args.text;
}

void Online::Input::InputMonitor::OnTextEditing(const Online::Event::Event& event)
{
    if (event.type != Online::Event::EventType::TextEditing) return;
    const auto& args = event.As<Online::Event::TextEditingEventArgs>();
    compositionText = args.text;
    compositionStart = args.start;
    compositionLength = args.length;
}

void Online::Input::InputMonitor::OnKeyThunk(void* listener, const Online::Event::Event& event)
{
    static_cast<Online::Input::InputMonitor*>(listener)->OnKey(event);
}

void Online::Input::InputMonitor::OnMouseButtonThunk(void* listener, const Online::Event::Event& event)
{
    static_cast<Online::Input::InputMonitor*>(listener)->OnMouseButton(event);
}

void Online::Input::InputMonitor::OnMouseMoveThunk(void* listener, const Online::Event::Event& event)
{
    static_cast<Online::Input::InputMonitor*>(listener)->OnMouseMove(event);
}

void Online::Input::InputMonitor::OnTextInputThunk(void* listener, const Online::Event::Event& event)
{
    static_cast<InputMonitor*>(listener)->OnTextInput(event);
}

void Online::Input::InputMonitor::OnTextEditingThunk(void* listener, const Online::Event::Event& event)
{
    static_cast<InputMonitor*>(listener)->OnTextEditing(event);
}

void Online::Input::InputMonitor::SetTextInputRect(int x, int y, int w, int h) noexcept
{
    Online::Event::SetTextInputRectEventArgs args(x, y, w, h);
    Online::Event::Emit(Online::Event::Event(Online::Event::EventType::SetTextInputRect, &args));
}
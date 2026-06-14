#include <Input/InputMonitor.h>
#include <Log/Common/FuncTable.h>

#ifdef PIXEL_SERVER
#include <Net/Server/Common/FuncTable.h>
#include <Net/Common/PlayerInputPacket.h>
#include <Input/Common/FuncTable.h>
#endif // PIXEL_SERVER

bool Online::Input::InputMonitor::Initialize()
{
#ifdef PIXEL_CLIENT
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
#endif

#ifdef PIXEL_SERVER
    keyHoldStates.clear();
    keyTriggers.clear();
#endif
    return true;
}

void Online::Input::InputMonitor::Release()
{
#ifdef PIXEL_CLIENT
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
    ResetAllState();
#endif

#ifdef PIXEL_SERVER
    keyHoldStates.clear();
    keyTriggers.clear();
#endif
}

void Online::Input::InputMonitor::FixedUpdate()
{
#ifdef PIXEL_SERVER
    auto& recvMsgQueue = Net::Server::GetMessageQueue(Net::PacketType::PlayerInput);
    Online::Net::NetMessage recvMsg;
    while (recvMsgQueue.Pop(recvMsg))
    {
        Online::Net::PlayerInputPacket inputPkt;
        if (!inputPkt.DeserializeFromPayload(recvMsg.body)) continue;

        uint32_t srcConnId = static_cast<uint32_t>(recvMsg.connectionId);
        if (srcConnId != inputPkt.connId) continue;

        SetKeyHoldState(srcConnId, Online::Input::KeyCode::A, inputPkt.keyA_Hold);
        SetKeyHoldState(srcConnId, Online::Input::KeyCode::D, inputPkt.keyD_Hold);
        // Ð´ÈëË²Ê±ÌøÔ¾°´¼ü Space
        if (inputPkt.keySpace_Press)
        {
            AddKeyTrigger(srcConnId, Online::Input::KeyCode::Space);
        }
    }
#endif // PIXEL_SERVER
}

std::string Online::Input::InputMonitor::GetTextInputBuffer() noexcept
{
#ifdef PIXEL_CLIENT
    std::string ret = std::move(textInputBuffer);
    textInputBuffer.clear();
    return ret;
#endif
	return {};
}

std::string Online::Input::InputMonitor::GetCompositionText() const noexcept
{
#ifdef PIXEL_CLIENT
    return compositionText;
#endif
	return {};
}

int Online::Input::InputMonitor::GetCompositionCursor() const noexcept
{
#ifdef PIXEL_CLIENT
    return compositionStart;
#endif
	return 0;
}

void Online::Input::InputMonitor::StartTextInput() noexcept
{
#ifdef PIXEL_CLIENT
    Online::Event::TextInputStartEventArgs args;
    Online::Event::Emit(Online::Event::Event(Online::Event::EventType::TextInputStart, &args));
#endif
}

void Online::Input::InputMonitor::StopTextInput() noexcept
{
#ifdef PIXEL_CLIENT
    Online::Event::TextInputStopEventArgs args;
    Online::Event::Emit(Online::Event::Event(Online::Event::EventType::TextInputStop, &args));
#endif
  
}

void Online::Input::InputMonitor::SetKeyHoldState(uint32_t connId, Online::Input::KeyCode key, bool isHold)
{
#ifdef PIXEL_SERVER
    keyHoldStates[connId][key] = isHold;
#endif
}

void Online::Input::InputMonitor::AddKeyTrigger(uint32_t connId, Online::Input::KeyCode key)
{
#ifdef PIXEL_SERVER
    keyTriggers[connId][key]++;
#endif
}

bool Online::Input::InputMonitor::IsClientKeyHold(uint32_t connId, Online::Input::KeyCode key) const
{
#ifdef PIXEL_SERVER
    auto connIt = keyHoldStates.find(connId);
    if (connIt == keyHoldStates.end())
        return false;

    auto keyIt = connIt->second.find(key);
    if (keyIt == connIt->second.end())
        return false;

    return keyIt->second; 
#endif
    return false;
}

bool Online::Input::InputMonitor::ConsumeClientTrigger(uint32_t connId, Online::Input::KeyCode key)
{
#ifdef PIXEL_SERVER
    auto connIt = keyTriggers.find(connId);
    if (connIt == keyTriggers.end())
        return false;

    auto keyIt = connIt->second.find(key);
    if (keyIt == connIt->second.end() || keyIt->second == 0)
        return false;

    keyIt->second = 0;
    return true; 
#endif
	return false;
}

void Online::Input::InputMonitor::RemoveClientInput(uint32_t connId)
{
#ifdef PIXEL_SERVER
    keyHoldStates.erase(connId);
	keyTriggers.erase(connId);
#endif
}

void Online::Input::InputMonitor::ClearAllClientInput()
{
#ifdef PIXEL_SERVER
    keyHoldStates.clear();
	keyTriggers.clear();
#endif
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
#ifdef PIXEL_CLIENT
    Online::Event::SetTextInputRectEventArgs args(x, y, w, h);
    Online::Event::Emit(Online::Event::Event(Online::Event::EventType::SetTextInputRect, &args));
#endif // PIXEL_CLIENT
}
#pragma once
#include <Core/Color/Color.h>
#include <Core/String/String.h>
#include <Script/Common/ScriptFunctionID.h>
#include <Script/Common/ScriptFunctionInfo.h>
#include <Script/Common/Tool.h>
#include <Game/Entity/GameObject.h>
#include <Game/Component/Transform.h>
#include <Game/Component/Sprite.h>
#include <Game/Component/Text.h>
#include <Game/Common/FuncTable.h>
#include <Input/Common/FuncTable.h>
#include <Asset/Common/FuncTable.h>
#include <functional>

namespace Online::Script
{
    struct TextInput
    {
        static const Script::ScriptFunctionID ID = Script::ScriptFunctionID::TextInput;

        class TextInputData
        {
            friend TextInput;
        public:
            std::string placeholder = "Enter text...";
            int maxChars = 32;
            std::function<void(Game::GameObject*, const std::string&)> onTextChanged;
            std::function<void(Game::GameObject*, const std::string&)> onEnterPressed;

            Game::GameObject* uiCamera = nullptr;
            Game::Sprite* background = nullptr;
            Game::Text* textComp = nullptr;
            Game::Sprite* cursor = nullptr;

            std::string buffer;
            std::string compositionText;
            int compositionStart = 0;
            bool focused = false;
            float cursorBlinkTimer = 0.0f;
            bool cursorVisible = true;
            int cursorIndex = 0;

            void SetOnTextChanged(std::function<void(Game::GameObject*, const std::string&)> callback) {
                onTextChanged = std::move(callback);
            }
            void SetOnEnterPressed(std::function<void(Game::GameObject*, const std::string&)> callback) {
                onEnterPressed = std::move(callback);
            }

            int TotalCodePoints() const {
                return static_cast<int>(Core::Utf8ToUtf32(buffer).size() + Core::Utf8ToUtf32(compositionText).size());
            }

            float GetCursorXOffset(float scale, float spacing, Online::Asset::FontID fontID) const {
                std::u32string full32 = Core::Utf8ToUtf32(buffer + compositionText);
                float x = 0.0f;
                int count = (cursorIndex < static_cast<int>(full32.size())) ? cursorIndex : static_cast<int>(full32.size());
                for (int i = 0; i < count; ++i) {
                    char32_t ch = full32[i];
                    int advance;

                    if (ch == U' ') {
                        advance = Online::Asset::GetFontAtlasAdvance(fontID, U'n');
                        if (advance <= 0) advance = static_cast<int>(Online::Asset::GetFontSize(fontID) * 0.5f);
                    }
                    else if (ch == U'　') {
                        advance = Online::Asset::GetFontAtlasAdvance(fontID, U'一');
                        if (advance <= 0) advance = Online::Asset::GetFontSize(fontID);
                    }
                    else {
                        advance = Online::Asset::GetFontAtlasAdvance(fontID, ch);
                    }

                    x += (static_cast<float>(advance) + spacing) * scale;
                }
                return x;
            }

            void CalculateTextSize(const std::string& text, float scale, float spacing,
                Online::Asset::FontID fontID, float& outWidth, float& outHeight, int& outLines) const
            {
                std::u32string u32text = Core::Utf8ToUtf32(text);
                const int cellSize = Online::Asset::GetFontSize(fontID);
                const float lineHeight = static_cast<float>(cellSize) * scale;

                int numLines = 1;
                float maxLineWidth = 0.0f;
                float currentLineWidth = 0.0f;

                for (const char32_t ch : u32text)
                {
                    if (ch == U'\n')
                    {
                        maxLineWidth = maxLineWidth > currentLineWidth ? maxLineWidth : currentLineWidth;
                        currentLineWidth = 0.0f;
                        ++numLines;
                        continue;
                    }

                    int advance;
                    if (ch == U' ') {
                        advance = Online::Asset::GetFontAtlasAdvance(fontID, U'n');
                        if (advance <= 0) advance = static_cast<int>(cellSize * 0.5f);
                    }
                    else if (ch == U'　') {
                        advance = Online::Asset::GetFontAtlasAdvance(fontID, U'一');
                        if (advance <= 0) advance = cellSize;
                    }
                    else {
                        advance = Online::Asset::GetFontAtlasAdvance(fontID, ch);
                    }

                    currentLineWidth += (static_cast<float>(advance) + spacing) * scale;
                }
                maxLineWidth = maxLineWidth > currentLineWidth ? maxLineWidth : currentLineWidth;

                outWidth = maxLineWidth;
                outHeight = static_cast<float>(numLines) * lineHeight;
                outLines = numLines;
            }

            glm::vec2 CalculateAnchorOffset(Core::Anchor anchor, float totalWidth, float totalHeight) const
            {
                glm::vec2 anchorOffset{ 0.0f, 0.0f };
                switch (anchor)
                {
                    using enum Core::Anchor;
                case TopLeft:      anchorOffset = { 0.0f, 0.0f };                               break;
                case TopCenter:    anchorOffset = { totalWidth * 0.5f, 0.0f };                  break;
                case TopRight:     anchorOffset = { totalWidth, 0.0f };                         break;
                case CenterLeft:   anchorOffset = { 0.0f, totalHeight * 0.5f };                 break;
                case Center:       anchorOffset = { totalWidth * 0.5f, totalHeight * 0.5f };    break;
                case CenterRight:  anchorOffset = { totalWidth, totalHeight * 0.5f };           break;
                case BottomLeft:   anchorOffset = { 0.0f, totalHeight };                        break;
                case BottomCenter: anchorOffset = { totalWidth * 0.5f, totalHeight };           break;
                case BottomRight:  anchorOffset = { totalWidth, totalHeight };                  break;
                default: break;
                }
                return anchorOffset;
            }

            size_t CodePointIndexToByteOffset(const std::string& str, int cpIndex) const {
                size_t byteOffset = 0;
                int cpCount = 0;
                const char* ptr = str.data();
                while (cpCount < cpIndex && byteOffset < str.size()) {
                    unsigned char c = static_cast<unsigned char>(ptr[byteOffset]);
                    if ((c & 0x80) == 0x00) byteOffset += 1;
                    else if ((c & 0xE0) == 0xC0) byteOffset += 2;
                    else if ((c & 0xF0) == 0xE0) byteOffset += 3;
                    else if ((c & 0xF8) == 0xF0) byteOffset += 4;
                    else break;
                    ++cpCount;
                }
                return byteOffset;
            }

            void InsertTextAt(int insertCp, const std::string& utf8) {
                std::u32string u32 = Core::Utf8ToUtf32(utf8);
                std::string newBuffer;
                size_t byteIns = CodePointIndexToByteOffset(buffer, insertCp);
                newBuffer = buffer.substr(0, byteIns);
                for (char32_t ch : u32) {
                    if (static_cast<int>(Core::Utf8ToUtf32(newBuffer).size()) >= maxChars) break;
                    char temp[4];
                    size_t len = Core::CodepointToUtf8(ch, temp);
                    newBuffer.append(temp, len);
                }
                newBuffer += buffer.substr(byteIns);
                buffer = newBuffer;
                cursorIndex = insertCp + static_cast<int>(u32.size());
            }

            void InsertText(const std::string& utf8) {
                if (utf8.empty()) return;
                int insertPos = cursorIndex;
                if (!compositionText.empty()) insertPos = compositionStart;
                InsertTextAt(insertPos, utf8);
                compositionText.clear();
                compositionStart = cursorIndex;
                RefreshDisplay();
                NotifyChange();
            }

            void DeleteBack() {
                if (cursorIndex <= 0) return;
                if (!compositionText.empty()) {
                    compositionText.clear();
                    cursorIndex = compositionStart;
                    RefreshDisplay();
                    NotifyChange();
                    return;
                }
                int delPos = cursorIndex - 1;
                size_t byteStart = CodePointIndexToByteOffset(buffer, delPos);
                size_t byteEnd = CodePointIndexToByteOffset(buffer, cursorIndex);
                buffer.erase(byteStart, byteEnd - byteStart);
                cursorIndex = delPos;
                compositionStart = cursorIndex;
                RefreshDisplay();
                NotifyChange();
            }

            void DeleteForward() {
                int totalBuf = static_cast<int>(Core::Utf8ToUtf32(buffer).size());
                if (cursorIndex >= totalBuf && compositionText.empty()) return;
                if (!compositionText.empty()) {
                    compositionText.clear();
                    cursorIndex = compositionStart;
                    RefreshDisplay();
                    NotifyChange();
                    return;
                }
                int delPos = cursorIndex;
                size_t byteStart = CodePointIndexToByteOffset(buffer, delPos);
                size_t byteEnd = CodePointIndexToByteOffset(buffer, delPos + 1);
                buffer.erase(byteStart, byteEnd - byteStart);
                compositionStart = cursorIndex;
                RefreshDisplay();
                NotifyChange();
            }

            void Clear() {
                buffer.clear();
                compositionText.clear();
                cursorIndex = 0;
                compositionStart = 0;
                RefreshDisplay();
                NotifyChange();
            }

            void SetText(const std::string& text) {
                buffer = text.substr(0, maxChars);
                compositionText.clear();
                cursorIndex = static_cast<int>(Core::Utf8ToUtf32(buffer).size());
                compositionStart = cursorIndex;
                RefreshDisplay();
            }

        private:
            void RefreshDisplay() {
                if (!textComp) return;
                std::string displayStr;
                if (focused && !compositionText.empty()) {
                    int startCp = compositionStart;
                    std::u32string buf32 = Core::Utf8ToUtf32(buffer);
                    if (startCp > static_cast<int>(buf32.size())) startCp = static_cast<int>(buf32.size());
                    size_t byteOff = CodePointIndexToByteOffset(buffer, startCp);
                    std::string part1 = buffer.substr(0, byteOff);
                    std::string part2 = buffer.substr(byteOff);
                    displayStr = part1 + compositionText + part2;
                }
                else {
                    displayStr = buffer;
                }
                if (displayStr.empty() && !focused)
                    textComp->SetText(placeholder);
                else
                    textComp->SetText(displayStr);
            }

            void NotifyChange() {
                if (onTextChanged && textComp)
                    onTextChanged(textComp->gameObject, buffer);
            }
            void NotifyEnter() {
                if (onEnterPressed && textComp)
                    onEnterPressed(textComp->gameObject, buffer);
            }
        };

        static void TextInputData_Construct(void* p) { new (p) TextInputData(); }
        static void TextInputData_Destruct(void* p) { static_cast<TextInputData*>(p)->~TextInputData(); }

        static void TextInputData_OnEnable(Game::GameObject* go)
        {
            auto* data = go->GetScriptData<TextInputData>(ID);
            if (!data) 
                return;

            data->uiCamera = Game::FindGameObjectByTag("UICamera");
            if (!data->uiCamera) 
                throw std::runtime_error("当前场景无UI摄像机");

            data->textComp = go->GetComponent<Game::Text>();
            if (!data->textComp) 
                throw std::runtime_error("TextInput 需要挂载 Text 组件");

            data->background = go->GetComponent<Game::Sprite>();
            if (auto* cursorObj = Game::FindGameObjectByName("Cursor"))
                data->cursor = cursorObj->GetComponent<Game::Sprite>();

            data->cursorIndex = static_cast<int>(Core::Utf8ToUtf32(data->buffer + data->compositionText).size());
            data->compositionStart = data->cursorIndex;
            data->RefreshDisplay();
            if (data->cursor)
                data->cursor->OnDisable();
        }

        static void TextInputData_Update(Game::GameObject* self, float dt)
        {
            auto* data = self->GetScriptData<TextInputData>(ID);
            if (!data || !data->textComp) return;

            auto* trans = self->GetTransform();
            auto* bg = data->background;

            glm::vec2 mousePos = Input::OnGetMousePosition()
                + data->uiCamera->GetComponent<Game::Transform>()->GetWorldPosition();

            SDL_FRect hitRect;
            if (bg) hitRect = bg->GetDstRect(trans->GetWorldPosition(), trans->GetWorldScale());
            else    hitRect = { trans->GetWorldPosition().x, trans->GetWorldPosition().y, 200.0f, 50.0f };

            if (data->focused) 
            {
                auto* camTrans = data->uiCamera->GetComponent<Game::Transform>();
                if (camTrans) 
                {
                    glm::vec2 screenPos = trans->GetWorldPosition() - glm::vec2(camTrans->GetWorldPosition());
                    Input::SetTextInputRect(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y),
                        static_cast<int>(hitRect.w * trans->GetWorldScale().x),
                        static_cast<int>(hitRect.h * trans->GetWorldScale().y));
                }
            }

            if (Input::GetKeyPressed(Input::KeyCode::Mouse0)) 
            {
                bool inside = PointInRect(mousePos, hitRect);
                if (inside && !data->focused) 
                {
                    data->focused = true;
                    Input::StartTextInput();
                    data->compositionText.clear();
                    data->cursorIndex = static_cast<int>(Core::Utf8ToUtf32(data->buffer).size());
                    data->compositionStart = data->cursorIndex;
                    data->RefreshDisplay();
                }
                else if (!inside && data->focused) 
                {
                    data->focused = false;
                    Input::StopTextInput();
                    data->compositionText.clear();
                    data->RefreshDisplay();
                }
            }

            if (data->focused) 
            {
                std::string inputStr = Input::GetTextInputBuffer();
                if (!inputStr.empty()) {
                    data->InsertText(inputStr);
                    data->compositionText.clear();
                    data->compositionStart = data->cursorIndex;
                }

                std::string comp = Input::GetCompositionText();
                if (comp != data->compositionText) 
                {
                    if (data->compositionText.empty() && !comp.empty())
                        data->compositionStart = data->cursorIndex;
                    data->compositionText = comp;
                    if (!comp.empty())
                        data->cursorIndex = data->compositionStart + static_cast<int>(Core::Utf8ToUtf32(comp).size());
                    else
                        data->cursorIndex = data->compositionStart;
                    data->RefreshDisplay();
                }

                if (data->compositionText.empty()) 
                {
                    if (Input::GetKeyPressed(Input::KeyCode::Left) && data->cursorIndex > 0) 
                    {
                        --data->cursorIndex;
                        data->compositionStart = data->cursorIndex;
                        data->cursorBlinkTimer = 0.0f;
                        data->cursorVisible = true;
                    }
                    if (Input::GetKeyPressed(Input::KeyCode::Right)) 
                    {
                        int total = data->TotalCodePoints();
                        if (data->cursorIndex < total) 
                        {
                            ++data->cursorIndex;
                            data->compositionStart = data->cursorIndex;
                            data->cursorBlinkTimer = 0.0f;
                            data->cursorVisible = true;
                        }
                    }
                }

                if (Input::GetKeyPressed(Input::KeyCode::Backspace)) data->DeleteBack();
                if (Input::GetKeyPressed(Input::KeyCode::Delete)) data->DeleteForward();
                if (Input::GetKeyPressed(Input::KeyCode::Enter)) data->NotifyEnter();
            }

            if (data->cursor) 
            {
                if (data->focused) 
                {
                    data->cursorBlinkTimer += dt;
                    if (data->cursorBlinkTimer >= 0.53f) 
                    {
                        data->cursorBlinkTimer = 0.0f;
                        data->cursorVisible = !data->cursorVisible;
                    }
                    data->cursorVisible ? data->cursor->OnEnable() : data->cursor->OnDisable();

                    if (data->textComp) 
                    {
                        glm::vec2 worldPos = trans->GetWorldPosition();
                        glm::vec2 textOffset = data->textComp->GetOffset();
                        float scale = trans->GetWorldScale().x;
                        auto fontID = data->textComp->GetFont();
                        float spacing = data->textComp->GetLetterSpacing();
                        Core::Anchor anchor = data->textComp->GetAnchor();
                        const int cellSize = Online::Asset::GetFontSize(fontID);
                        const float lineHeight = static_cast<float>(cellSize) * scale;

                        std::string displayText;
                        if (!data->compositionText.empty()) 
                        {
                            int startCp = data->compositionStart;
                            size_t byteOff = data->CodePointIndexToByteOffset(data->buffer, startCp);
                            displayText = data->buffer.substr(0, byteOff) + data->compositionText + data->buffer.substr(byteOff);
                        }
                        else 
                        {
                            displayText = data->buffer;
                        }

                        float textWidth, textHeight;
                        int numLines;
                        data->CalculateTextSize(displayText, scale, spacing, fontID, textWidth, textHeight, numLines);

                        glm::vec2 anchorOffset = data->CalculateAnchorOffset(anchor, textWidth, textHeight);

                        glm::vec2 textTopLeft = worldPos + textOffset - anchorOffset;

                        float cursorXOffset = data->GetCursorXOffset(scale, spacing, fontID);
                        float cursorYOffset = lineHeight * 0.5f;

                        glm::vec2 cursorPos = textTopLeft;
                        cursorPos.x += cursorXOffset;
                        cursorPos.y += cursorYOffset;

                        if (auto* cursorTrans = data->cursor->gameObject->GetTransform()) 
                        {
                            cursorTrans->SetWorldPosition(cursorPos);
                            float texH = 1.0f;
                            SDL_Texture* cursorTex = Online::Asset::GetTexture(data->cursor->GetTexture());
                            if (cursorTex) 
                            {
                                int w, h; SDL_QueryTexture(cursorTex, nullptr, nullptr, &w, &h);
                                texH = static_cast<float>(h);
                            }
                            cursorTrans->SetLocalScale(glm::vec2(cursorTrans->GetLocalScale().x, lineHeight / texH));
                        }
                    }
                }
                else 
                {
                    data->cursor->OnDisable();
                }
            }
        }

        static ScriptFunctionInfo Information() {
            return {
                ID, sizeof(TextInputData),
                TextInputData_Construct, TextInputData_Destruct,
                TextInputData_OnEnable, nullptr,
                TextInputData_Update, nullptr
            };
        }
    };
}
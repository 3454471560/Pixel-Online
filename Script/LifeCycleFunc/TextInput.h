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
#include <algorithm>
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
            bool focused = false;
            float cursorBlinkTimer = 0.0f;
            bool cursorVisible = true;

            void SetOnTextChanged(std::function<void(Game::GameObject*, const std::string&)> callback)
            {
                onTextChanged = std::move(callback);
            }

            void SetOnEnterPressed(std::function<void(Game::GameObject*, const std::string&)> callback)
            {
                onEnterPressed = std::move(callback);
            }

            void InsertText(const std::string& utf8)
            {
                if (utf8.empty()) return;

                std::u32string u32 = Online::Core::Utf8ToUtf32(utf8);
                for (char32_t ch : u32)
                {
                    if (buffer.size() >= static_cast<size_t>(maxChars))
                        break;

                    char temp[4];
                    size_t len = Online::Core::CodepointToUtf8(ch, temp);
                    buffer.append(temp, len);
                }
                compositionText.clear();
                RefreshDisplay();
                NotifyChange();
            }

            void DeleteBack()
            {
                if (buffer.empty()) return;
                auto it = buffer.end();
                while (it != buffer.begin())
                {
                    --it;
                    if ((*it & 0xC0) != 0x80) break;
                }
                buffer.erase(it, buffer.end());
                RefreshDisplay();
                NotifyChange();
            }

            void Clear()
            {
                buffer.clear();
                compositionText.clear();
                RefreshDisplay();
                NotifyChange();
            }

            void SetText(const std::string& text)
            {
                buffer = text.substr(0, maxChars);
                compositionText.clear();
                RefreshDisplay();
            }

        private:
            void RefreshDisplay()
            {
                if (!textComp) return;
                std::string displayStr = buffer;
                if (focused && !compositionText.empty())
                    displayStr += compositionText;
                if (displayStr.empty() && !focused)
                    textComp->SetText(placeholder);
                else
                    textComp->SetText(displayStr);
            }

            void NotifyChange()
            {
                if (onTextChanged && textComp)
                    onTextChanged(textComp->gameObject, buffer);
            }

            void NotifyEnter()
            {
                if (onEnterPressed && textComp)
                    onEnterPressed(textComp->gameObject, buffer);
            }
        };

        static void TextInputData_Construct(void* p) { new (p) TextInputData(); }
        static void TextInputData_Destruct(void* p) { static_cast<TextInputData*>(p)->~TextInputData(); }

        static void TextInputData_OnEnable(Game::GameObject* go)
        {
            auto* data = go->GetScriptData<TextInputData>(ID);
            if (!data) return;

            data->uiCamera = Game::FindGameObjectByTag("UICamera");
            if (!data->uiCamera)
                throw std::runtime_error("当前场景无UI摄像机");

            data->textComp = go->GetComponent<Game::Text>();
            if (!data->textComp)
                throw std::runtime_error("TextInput 需要挂载 Text 组件");

            data->background = go->GetComponent<Game::Sprite>();
            if (auto* cursorObj = Game::FindGameObjectByName("Cursor"))
                data->cursor = cursorObj->GetComponent<Game::Sprite>();

            data->RefreshDisplay();
            if (data->cursor) data->cursor->OnDisable();
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
            if (bg)
                hitRect = bg->GetDstRect(trans->GetWorldPosition(), trans->GetWorldScale());
            else
                hitRect = { trans->GetWorldPosition().x, trans->GetWorldPosition().y, 200.0f, 50.0f };

            if (data->focused)
            {
                auto* camTrans = data->uiCamera->GetComponent<Game::Transform>();
                if (camTrans)
                {
                    glm::vec2 screenPos = trans->GetWorldPosition() - glm::vec2(camTrans->GetWorldPosition());
                    Input::SetTextInputRect(
                        static_cast<int>(screenPos.x),
                        static_cast<int>(screenPos.y),
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
                if (!inputStr.empty())
                    data->InsertText(inputStr);

                std::string comp = Input::GetCompositionText();
                if (comp != data->compositionText)
                {
                    data->compositionText = comp;
                    data->RefreshDisplay();
                }

                if (Input::GetKeyPressed(Input::KeyCode::Backspace))
                    data->DeleteBack();
                if (Input::GetKeyPressed(Input::KeyCode::Delete))
                    data->Clear();

                if (Input::GetKeyPressed(Input::KeyCode::Enter))
                {
                    data->NotifyEnter();
                }
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
                        glm::vec2 basePos = trans->GetWorldPosition() + data->textComp->GetOffset();
                        float scale = trans->GetWorldScale().x;
                        auto fontID = data->textComp->GetFont();
                        float spacing = data->textComp->GetLetterSpacing();
                        Core::Anchor anchor = data->textComp->GetAnchor();

                        std::u32string buf32 = Online::Core::Utf8ToUtf32(data->buffer);
                        std::u32string comp32 = Online::Core::Utf8ToUtf32(data->compositionText);
                        float bufWidth = 0.0f, compWidth = 0.0f;
                        for (char32_t ch : buf32)
                            bufWidth += (Asset::GetFontAtlasAdvance(fontID, ch) + spacing) * scale;
                        for (char32_t ch : comp32)
                            compWidth += (Asset::GetFontAtlasAdvance(fontID, ch) + spacing) * scale;
                        float totalWidth = bufWidth + compWidth;

                        float lineHeight = static_cast<float>(data->textComp->GetFontHeight()) * scale;

                        glm::vec2 anchorOffset(0.0f);
                        switch (anchor)
                        {
                            using enum Core::Anchor;
                        case TopLeft:      anchorOffset = { 0.0f, 0.0f }; break;
                        case TopRight:     anchorOffset = { totalWidth, 0.0f }; break;
                        case BottomLeft:   anchorOffset = { 0.0f, lineHeight }; break;
                        case BottomRight:  anchorOffset = { totalWidth, lineHeight }; break;
                        case Center:       anchorOffset = { totalWidth * 0.5f, lineHeight * 0.5f }; break;
                        default: break;
                        }
                        glm::vec2 textStartPos = basePos - anchorOffset;

                        glm::vec2 cursorPos = textStartPos;
                        cursorPos.x += totalWidth + 2.0f;
                        cursorPos.y += lineHeight * 0.5f;

                        if (auto* cursorTrans = data->cursor->gameObject->GetTransform())
                        {
                            cursorTrans->SetWorldPosition(cursorPos);
                            float texH = 1.0f;
                            SDL_Texture* cursorTex = Asset::GetTexture(data->cursor->GetTexture());
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

        static ScriptFunctionInfo Information()
        {
            return {
                ID, sizeof(TextInputData),
                TextInputData_Construct, TextInputData_Destruct,
                TextInputData_OnEnable, nullptr,
                TextInputData_Update, nullptr
            };
        }
    };
}
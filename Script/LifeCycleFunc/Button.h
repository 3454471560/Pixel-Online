#pragma once
#include <Core/Color/Color.h>
#include <Script/Common/ScriptFunctionID.h>
#include <Script/Common/ScriptFunctionInfo.h>
#include <Script/Common/ButtonMode.h>
#include <Script/Common/ButtonState.h>
#include <Script/Common/Tool.h>
#include <Game/Entity/GameObject.h>
#include <Game/Common/FuncTable.h>
#include <Input/Common/FuncTable.h>
namespace Online::Script
{
    struct Button
    {
        static const Script::ScriptFunctionID ID = Script::ScriptFunctionID::Button;
        class ButtonData
        {
        public:
            ButtonMode mode = ButtonMode::Color;

            Asset::TextureID normalTex;
            Asset::TextureID hoverTex;
            Asset::TextureID pressedTex;

            glm::vec4 normalColor = Core::Color::White;
            glm::vec4 hoverColor = Core::Color::Yellow;
            glm::vec4 pressedColor = Core::Color::Blue;

            std::function<void(Game::GameObject*)> onClick;

            Game::GameObject* uiCamera = nullptr;
            Game::Sprite* sprite = nullptr; 
            bool wasHovered = false;

            static void ApplyState(Game::Sprite* sprite, ButtonData* data, ButtonState state)
            {
                if (data->mode == ButtonMode::Texture)
                {
                    Asset::TextureID tex = data->normalTex;
                    switch (state)
                    {
                    case ButtonState::Hovered:  tex = data->hoverTex;  break;
                    case ButtonState::Pressed:  tex = data->pressedTex; break;
                    default: break;
                    }
                    sprite->SetTexture(tex);
                }
                else
                {
                    glm::vec4 color = data->normalColor;
                    switch (state)
                    {
                    case ButtonState::Hovered:  color = data->hoverColor;  break;
                    case ButtonState::Pressed:  color = data->pressedColor; break;
                    default: break;
                    }
                    sprite->SetColor(color);
                }
            }

            void SetOnClick(std::function<void(Game::GameObject*)> callback)
            {
                onClick = std::move(callback);
            }
        };
        static void ButtonData_OnEnable(Game::GameObject* go)
        {
            auto* data = go->GetScriptData<ButtonData>(ID);

            data->uiCamera = Game::FindGameObjectByTag("UICamera");
            data->sprite = go->GetComponent<Game::Sprite>();
            if (!data->uiCamera)
            {
                throw std::runtime_error("当前场景无UI摄像机");
            }

            if (data->sprite)
            {
                data->sprite->SetRenderQueue(Render::RenderQueue::UI);
            }

        }
        static void ButtonData_Update(Game::GameObject* self, float dt)
        {
            auto* data = self->GetScriptData<ButtonData>(ID);
            auto* trans = self->GetTransform();
            auto* sprite = data->sprite;

            if (!sprite)
            {
                return;
            }

            glm::vec2 mousePos = Input::OnGetMousePosition() + data->uiCamera->GetComponent<Game::Transform>()->GetWorldPosition();

            SDL_FRect rect = sprite->GetDstRect(trans->GetWorldPosition(), trans->GetWorldScale());

            bool hover = PointInRect(mousePos, rect);
            bool pressed = Input::GetKeyPressed(Input::KeyCode::Mouse0);
            bool released = Input::GetKeyReleased(Input::KeyCode::Mouse0);

            if (hover && pressed)
                ButtonData::ApplyState(sprite, data, ButtonState::Pressed);
            else if (hover)
                ButtonData::ApplyState(sprite, data, ButtonState::Hovered);
            else
                ButtonData::ApplyState(sprite, data, ButtonState::Normal);

            if (hover && released && data->onClick)
                data->onClick(self);
        }
        static void ButtonData_Construct(void* p)
        {
            new (p) ButtonData();
        }
        static void ButtonData_Destruct(void* p)
        {
            static_cast<ButtonData*>(p)->~ButtonData();
        }
        static ScriptFunctionInfo Information()
        {
            return
            {
                ID,
                sizeof(ButtonData),
                ButtonData_Construct,
                ButtonData_Destruct,
                ButtonData_OnEnable,
                nullptr,
                ButtonData_Update,
                nullptr
            };
        }
    };

}

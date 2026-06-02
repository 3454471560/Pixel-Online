#pragma once

#include <Core/Allocate/Allocate.h>
#include <Context/Common/Module.h>
#include <Asset/Common/FuncTable.h>
#include <Core/Listen/ListenSnapshot.h>
#include <Audio/Common/ActiveSound.h>
#include <Audio/Common/ChannelInfo.h>
#include <Audio/Common/SoundSubmission.h>

#include <SDL_mixer.h>

#include <glm.hpp>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace Online::Audio
{
    class AudioPlayer
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<AudioPlayer>;
        private:
            static AudioPlayer* Create() { return ONLINE_NEW(AudioPlayer); }
            static void Destroy(AudioPlayer* p) { ONLINE_DELETE(p); }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<AudioPlayer>;
        private:
            static bool Initialize(AudioPlayer* player) { return player->Initialize(); }
            static void Release(AudioPlayer* player) { player->Release(); }
            static void BeginFrame(AudioPlayer* player) { player->BeginFrame(); }
            static void EndFrame(AudioPlayer* player) { player->EndFrame(); }
        };

    public:
        AudioPlayer() = default;
        ~AudioPlayer() = default;

        void SubmitListener(const Core::ListenSnapshot& snapshot)
        {
            currentListener = snapshot;
        }

        void SubmitBackgroundMusic(Asset::MusicID id, float volume, bool paused)
        {
            pendingMusicID = id;
            pendingMusicVolume = volume;
            pendingMusicPaused = paused;
        }

        void SubmitSound(uint32_t id,
            const glm::vec2& worldPos,
            Asset::SoundID soundId,
            float volume,
            bool loop,
            bool isPlaying,
            float spatialBlend,
            AudioQueue priority)
        {
            if (id == 0) return;
            soundSubmissions.push_back({ id, worldPos, soundId, volume, loop, isPlaying, spatialBlend, priority });
        }

    private:
        bool Initialize()
        {
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
                return false;

            constexpr int MAX_CHANNELS = 32;
            Mix_AllocateChannels(MAX_CHANNELS);
            channels.resize(MAX_CHANNELS);
            for (auto& ch : channels)
            {
                ch.id = 0;
                ch.priority = AudioQueue(0);
            }
            return true;
        }

        void Release()
        {
            for (auto& pair : activeSounds)
            {
                if (pair.second.channel >= 0)
                    Mix_HaltChannel(pair.second.channel);
            }
            activeSounds.clear();

            Mix_HaltMusic();
            Mix_CloseAudio();
        }

        void BeginFrame()
        {
            soundSubmissions.clear();

            // 将现有活跃声音标记为“未访问”，便于后续清理
            for (auto& pair : activeSounds)
                pair.second.visitedThisFrame = false;
        }

        void EndFrame()
        {
            // 1. 背景音乐同步
            HandleBackgroundMusic();

            // 2. 处理本帧所有音效提交
            for (const auto& sub : soundSubmissions)
            {
                if (!sub.isPlaying)
                {
                    auto it = activeSounds.find(sub.id);
                    if (it != activeSounds.end())
                    {
                        FreeChannel(it->second.channel);
                        activeSounds.erase(it);
                    }
                    continue;
                }

                auto it = activeSounds.find(sub.id);
                if (it == activeSounds.end())
                {
                    // 新声音
                    ActiveSound as;
                    as.id = sub.id;
                    as.soundId = sub.soundId;
                    as.baseVolume = sub.volume;
                    as.spatialBlend = sub.spatialBlend;
                    as.loop = sub.loop;
                    as.priority = sub.priority;
                    as.worldPos = sub.worldPos;
                    as.visitedThisFrame = true;

                    if (AllocateAndPlay(as))
                        activeSounds[sub.id] = as;
                }
                else
                {
                    // 已存在的声音：更新属性
                    ActiveSound& as = it->second;
                    bool soundChanged = (as.soundId != sub.soundId);

                    as.soundId = sub.soundId;
                    as.baseVolume = sub.volume;
                    as.spatialBlend = sub.spatialBlend;
                    as.loop = sub.loop;
                    as.priority = sub.priority;
                    as.worldPos = sub.worldPos;
                    as.visitedThisFrame = true;

                    // 仅在声音资源改变时重新播放
                    if (soundChanged)
                    {
                        FreeChannel(as.channel);
                        as.channel = -1;
                        if (!AllocateAndPlay(as))
                            activeSounds.erase(it);
                    }
                }
            }

            // 3. 移除本帧未提交（已停止/销毁）的声音
            for (auto it = activeSounds.begin(); it != activeSounds.end(); )
            {
                if (!it->second.visitedThisFrame)
                {
                    FreeChannel(it->second.channel);
                    it = activeSounds.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            // 4. 空间化所有活跃音效
            ApplySpatialization();
        }

        void HandleBackgroundMusic()
        {
            Asset::MusicID targetID = pendingMusicID;
            float         targetVolume = pendingMusicVolume;
            bool          targetPaused = pendingMusicPaused;

            if (targetID == Asset::MusicID::Invalid)
            {
                if (currentMusic)
                {
                    Mix_HaltMusic();
                    currentMusic = nullptr;
                }
                return;
            }

            Mix_Music* targetMusic = Online::Asset::GetMusic(targetID);
            if (!targetMusic)
            {
                if (currentMusic)
                {
                    Mix_HaltMusic();
                    currentMusic = nullptr;
                }
                return;
            }

            if (currentMusic != targetMusic)
            {
                Mix_HaltMusic();
                currentMusic = targetMusic;
                Mix_PlayMusic(currentMusic, -1);
            }

            if (targetPaused)
                Mix_PauseMusic();
            else
                Mix_ResumeMusic();

            float finalVol = targetVolume * currentListener.MasterVolume;
            int sdlVol = glm::clamp(static_cast<int>(finalVol * MIX_MAX_VOLUME), 0, MIX_MAX_VOLUME);
            Mix_VolumeMusic(sdlVol);
        }

        bool AllocateAndPlay(ActiveSound& sound)
        {
            Mix_Chunk* chunk = Online::Asset::GetSound(sound.soundId);
            if (!chunk) return false;

            int channel = AllocateChannel(sound.priority);
            if (channel < 0) return false;

            Mix_Volume(channel, static_cast<int>(sound.baseVolume * MIX_MAX_VOLUME));
            Mix_SetPanning(channel, 255, 255);

            int loops = sound.loop ? -1 : 0;
            int playedChannel = Mix_PlayChannelTimed(channel, chunk, loops, -1);
            if (playedChannel < 0)
            {
                channels[channel].id = 0;
                return false;
            }

            sound.channel = playedChannel;
            channels[playedChannel].id = sound.id;
            channels[playedChannel].priority = sound.priority;
            return true;
        }

        int AllocateChannel(AudioQueue priority)
        {
            // 1. 寻找空闲通道
            for (int i = 0; i < (int)channels.size(); ++i)
                if (channels[i].id == 0)
                    return i;

            // 2. 抢占优先级最低的通道
            int victimIdx = -1;
            uint16_t minPrio = static_cast<uint16_t>(priority);
            for (int i = 0; i < (int)channels.size(); ++i)
            {
                uint16_t chPrio = static_cast<uint16_t>(channels[i].priority);
                if (chPrio < minPrio)
                {
                    minPrio = chPrio;
                    victimIdx = i;
                }
            }

            if (victimIdx >= 0)
            {
                uint32_t oldId = channels[victimIdx].id;
                auto it = activeSounds.find(oldId);
                if (it != activeSounds.end())
                {
                    Mix_HaltChannel(it->second.channel);
                    it->second.channel = -1;
                    activeSounds.erase(it);
                }
                channels[victimIdx].id = 0;
                return victimIdx;
            }

            return -1;
        }

        void FreeChannel(int channel)
        {
            if (channel >= 0 && channel < (int)channels.size())
            {
                Mix_HaltChannel(channel);
                channels[channel].id = 0;
            }
        }

        void ApplySpatialization()
        {
            for (auto& pair : activeSounds)
            {
                ActiveSound& sound = pair.second;
                if (sound.channel < 0) continue;

                float finalVolume = sound.baseVolume * currentListener.MasterVolume;
                float leftW = 1.0f, rightW = 1.0f;

                if (sound.spatialBlend > 0.0f)
                {
                    float distance = glm::distance(currentListener.Position, sound.worldPos);
                    float range = currentListener.Range;
                    float attenuation = 1.0f;
                    if (range > 0.0f)
                        attenuation = 1.0f - glm::clamp(distance / range, 0.0f, 1.0f);
                    float spatialVolume = finalVolume * attenuation;

                    float dx = sound.worldPos.x - currentListener.Position.x;
                    float pan = glm::clamp(dx / (range * 0.5f), -1.0f, 1.0f);

                    leftW = (1.0f + pan) * 0.5f;
                    rightW = (1.0f - pan) * 0.5f;

                    float blend = sound.spatialBlend;
                    finalVolume = glm::mix(finalVolume, spatialVolume, blend);
                    leftW = glm::mix(1.0f, leftW, blend);
                    rightW = glm::mix(1.0f, rightW, blend);
                }

                int vol = glm::clamp(static_cast<int>(finalVolume * MIX_MAX_VOLUME), 0, MIX_MAX_VOLUME);
                Mix_Volume(sound.channel, vol);

                Uint8 leftU8 = static_cast<Uint8>(glm::clamp(leftW * 255.0f, 0.0f, 255.0f));
                Uint8 rightU8 = static_cast<Uint8>(glm::clamp(rightW * 255.0f, 0.0f, 255.0f));
                Mix_SetPanning(sound.channel, leftU8, rightU8);
            }
        }

    private:
        Core::ListenSnapshot currentListener;

        std::vector<SoundSubmission> soundSubmissions;
        std::unordered_map<uint32_t, ActiveSound> activeSounds;
        std::vector<ChannelInfo> channels;

        Asset::MusicID pendingMusicID = Asset::MusicID::Invalid;
        float         pendingMusicVolume = 1.0f;
        bool          pendingMusicPaused = false;

        Mix_Music* currentMusic = nullptr;
    };
}
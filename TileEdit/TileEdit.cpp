#include <TileEdit/TileEdit.h>
#include <TileEdit/Dialog/Dialog.h>
#include <Core/Time/time.h>
#include <Core/String/String.h>
#include <Core/Color/Color.h>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include<stdexcept>
#include<fstream>
#include<sstream>

int Online::TileEdit::TileEdit::Execute()
{
	if (!Initialize())
	{
		return -1;
	}
	while (IsRunning())
	{
		BeginFrame();
		ImGui();
		Update();
		LateUpdate();
		Render();
		EndFrame();
		FrameSync();
	}

	Release();
	return 0;
}
void Online::TileEdit::TileEdit::Terminate()
{
	Release();
}

bool Online::TileEdit::TileEdit::Initialize()
{
	if (SDL_Init(SDL_INIT_EVERYTHING)) { throw std::runtime_error("SDL Initialize Fail!"); }
	if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) { throw std::runtime_error("SDL_image Initialize Fail!"); }
	if (TTF_Init()) { throw std::runtime_error("SDL_ttf Initialize Fail!"); }

	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

	window = SDL_CreateWindow(u8"TileEdit", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == nullptr) { throw std::runtime_error("Create Window Fail!"); }

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	if (renderer == nullptr) { throw std::runtime_error("Create Renderer Fail!"); }

	freq = static_cast<double>(SDL_GetPerformanceFrequency());
	startCounter = SDL_GetPerformanceCounter();
	lastCounter = startCounter;

	ImGui::CreateContext();

	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.FramePadding.y = 6;
	style.FramePadding.x = 6;
	style.ItemSpacing.x = 6;
	style.FrameBorderSize = 1.0f;
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.FrameRounding = 2.0f;

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\msyh.ttc)", 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

	std::string tilesetPath = Online::Core::Prepend(Online::Core::GetExeDir(), "Tileset.png");
	tilesetSurface = IMG_Load(tilesetPath.c_str());
	if (tilesetSurface == nullptr) {
		throw std::runtime_error("Failed to load tileset surface: " + tilesetPath);
	}
	tilesetTexture = SDL_CreateTextureFromSurface(renderer, tilesetSurface);
	if (tilesetTexture == nullptr) {
		throw std::runtime_error("Failed to create tileset texture from surface");
	}	
	return true;
}
bool Online::TileEdit::TileEdit::IsRunning() const
{
	return isRunning;
}
void Online::TileEdit::TileEdit::BeginFrame()
{
	Uint64 current_counter = SDL_GetPerformanceCounter();
	frame_time = (double)(current_counter - lastCounter) / freq;
	lastCounter = current_counter;

	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_MOUSEBUTTONDOWN:
			break;
		case SDL_KEYDOWN:
			break;
		default:
			break;
		}
	}
}
void Online::TileEdit::TileEdit::ImGui()
{
#pragma region NewFrame
	ImGui_ImplSDL2_NewFrame();
	ImGui_ImplSDLRenderer_NewFrame();
	ImGui::NewFrame();
#pragma endregion

#pragma region Position
	static int mainWinW, mainWinH;
	SDL_GetWindowSize(window, &mainWinW, &mainWinH);
	float contentH = mainWinH - 20.0f;

	float leftW = mainWinW * 0.35f;
	float rightW = mainWinW * 0.65f;
#pragma endregion

#pragma region MainMenu
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2((float)mainWinW, 30), ImGuiCond_Always);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
	if (ImGui::Begin("##TitleBar", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu(u8"文件"))
			{
				if (ImGui::MenuItem(u8"保存全部")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(u8"调试"))
			{
				ImGui::MenuItem(u8"启动调试");
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}
	ImGui::End();
	ImGui::PopStyleVar(3);
#pragma endregion

#pragma region TileSelection
	ImGui::SetNextWindowPos(ImVec2(0, 25), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(leftW, contentH), ImGuiCond_Always);
	if (ImGui::Begin(u8"瓦片选取器", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_MenuBar))
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu(u8"文件"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(u8"视图"))
			{
				if (ImGui::BeginMenu(u8"缩放"))
				{
					if (ImGui::MenuItem(u8"小")) scale = 1.3f;
					if (ImGui::MenuItem(u8"中")) scale = 1.6f;
					if (ImGui::MenuItem(u8"大")) scale = 1.8f;
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (tilesetTexture != nullptr && tilesetSurface != nullptr)
		{
			static const int tileSize = 32;
			const int spacing = 0;

			int texW, texH;
			SDL_QueryTexture(tilesetTexture, nullptr, nullptr, &texW, &texH);
			int tilesInTexRow = texW / tileSize;
			int tilesInTexCol = texH / tileSize;

			std::vector<std::pair<int, int>> validTiles;
			for (int ty = 0; ty < tilesInTexCol; ++ty) 
			{
				for (int tx = 0; tx < tilesInTexRow; ++tx) 
				{
					if (!IsTileEmpty(tx, ty)) 
					{
						validTiles.emplace_back(tx, ty);
					}
				}
			}

			ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
			ImVec2 canvas_size = ImGui::GetContentRegionAvail();
			float scaledTileSize = tileSize * scale;

			int cols = static_cast<int>(canvas_size.x) / static_cast<int>(scaledTileSize + spacing);
			cols = cols < 1 ? 1 : cols;
			int rows = (static_cast<int>(validTiles.size()) + cols - 1) / cols;

			ImGui::InvisibleButton("##TileCanvas",
				ImVec2(cols * (scaledTileSize + spacing), rows * (scaledTileSize + spacing)));

			std::vector<ImVec2> tilePosList;
			ImVec2 mousePos = ImGui::GetMousePos();
			int hoveredIdx = -1;

			for (int i = 0; i < static_cast<int>(validTiles.size()); ++i)
			{
				int col = i % cols;
				int row = i / cols;

				ImVec2 p0(canvas_p0.x + col * (scaledTileSize + spacing),
					canvas_p0.y + row * (scaledTileSize + spacing));
				ImVec2 p1(p0.x + scaledTileSize, p0.y + scaledTileSize);
				tilePosList.emplace_back(p0.x, p0.y);

				if (mousePos.x >= p0.x && mousePos.x < p1.x &&
					mousePos.y >= p0.y && mousePos.y < p1.y)
				{
					hoveredIdx = i;
				}

				int tx = validTiles[i].first;
				int ty = validTiles[i].second;
				ImVec2 uv0((tx * tileSize) / (float)texW, (ty * tileSize) / (float)texH);
				ImVec2 uv1(((tx + 1) * tileSize) / (float)texW, ((ty + 1) * tileSize) / (float)texH);
				draw_list->AddImage((ImTextureID)tilesetTexture, p0, p1, uv0, uv1);
			}

			if (hoveredIdx != -1)
			{
				ImVec2 p0 = tilePosList[hoveredIdx];
				ImVec2 p1(p0.x + scaledTileSize, p0.y + scaledTileSize);
				draw_list->AddRect(p0, p1, IM_COL32(255, 255, 255, 255), 0.0f, ImDrawFlags_None, 2.0f);
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hoveredIdx != -1)
			{
				selectedTileId = validTiles[hoveredIdx].first + validTiles[hoveredIdx].second * tilesInTexRow;
			}

			if (selectedTileId >= 0)
			{
				int selTx = selectedTileId % tilesInTexRow;
				int selTy = selectedTileId / tilesInTexRow;
				auto it = std::find(validTiles.begin(), validTiles.end(), std::make_pair(selTx, selTy));

				if (it != validTiles.end())
				{
					int idx = (int)(it - validTiles.begin());
					ImVec2 p0 = tilePosList[idx];
					ImVec2 p1(p0.x + scaledTileSize, p0.y + scaledTileSize);
					draw_list->AddRect(p0, p1, IM_COL32(255, 255, 255, 255), 0.0f, ImDrawFlags_None, 3.0f);
				}
			}
		}
		ImGui::End();
	}
#pragma endregion
#pragma region MapEdit
	ImGui::SetNextWindowPos(ImVec2(leftW, 25), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(rightW, contentH), ImGuiCond_Always);
	if (ImGui::Begin(u8"地图编辑器", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_AlwaysVerticalScrollbar |
		ImGuiWindowFlags_AlwaysHorizontalScrollbar
	))
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu(u8"文件"))
			{
				if (ImGui::MenuItem(u8"新建", "Ctrl+N"))
				{
					if (tileMap == nullptr || !isChanged)
						showMapCreat = true;
					else
						NewshowSaveWarn = true;
				}
				if (ImGui::MenuItem(u8"打开", "Ctrl+O"))
				{
					if (tileMap == nullptr || !isChanged)
						Open();
					else
						OpenshowSaveWarn = true;
				}
				ImGui::BeginDisabled(tileMap == nullptr);
				if (ImGui::MenuItem(u8"保存", "Ctrl+S"))
				{
					Save();
				}
				if (ImGui::MenuItem(u8"另存为"))
				{
					SaveAs();
				}
				if (ImGui::MenuItem(u8"保存当前视图"))
				{
					SaveCurrentView();
				}
				ImGui::EndDisabled();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(u8"编辑"))
			{
				if (ImGui::BeginMenu(u8"编辑模式"))
				{
					if (ImGui::MenuItem(u8"放置模式", nullptr, !EditMode))
					{
						EditMode = false;
					}
					if (ImGui::MenuItem(u8"擦除模式", nullptr, EditMode))
					{
						EditMode = true;
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(u8"层级模式"))
				{
					if (ImGui::MenuItem(u8"背景瓦片", nullptr, editLayer == LAYER_BACKGROUND))
					{
						editLayer = LAYER_BACKGROUND;
					}
					if (ImGui::MenuItem(u8"地形瓦片", nullptr, editLayer == LAYER_TERRAIN))
					{
						editLayer = LAYER_TERRAIN;
					}
					if (ImGui::MenuItem(u8"装饰瓦片", nullptr, editLayer == LAYER_DECORATION))
					{
						editLayer = LAYER_DECORATION;
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(u8"全部覆盖"))
				{
					if (ImGui::MenuItem(u8"当前层级全部覆盖"))
					{
						if (tileMap != nullptr && selectedTileId >= 0)
						{
							int mapW = tileMap->getMapWidth();
							int mapH = tileMap->getMapHeight();
							for (int y = 0; y < mapH; y++)
							{
								for (int x = 0; x < mapW; x++)
								{
									tileMap->SetTileLayer(x, y, editLayer, selectedTileId);
								}
							}
							isChanged = true;
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(u8"视图"))
			{
				if (ImGui::BeginMenu(u8"缩放比例"))
				{
					if (ImGui::MenuItem(u8"小")) { mapScale = 1.3f; }
					if (ImGui::MenuItem(u8"中")) { mapScale = 1.6f; }
					if (ImGui::MenuItem(u8"大")) { mapScale = 1.8f; }
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem(u8"背景瓦片", nullptr, &showBackgroundTiles)) {}
				if (ImGui::MenuItem(u8"地形瓦片", nullptr, &showTerrainTiles)) {}
				if (ImGui::MenuItem(u8"装饰瓦片", nullptr, &showDecorationTiles)) {}
				if (ImGui::MenuItem(u8"显示网格", nullptr, &showGrid)) {} 
				ImGui::EndMenu();
			}

			ImVec2 winPos = ImGui::GetWindowPos();
			ImVec2 winSize = ImGui::GetWindowSize();
			float textX = winPos.x + winSize.x - 290;
			float textY = winPos.y + 4;
			std::string info = tileMap == nullptr ? u8"当前地图: 无 | 缩放: 1.0x" :
				std::string(u8"当前地图: ") + (isFileOpen ? (Online::Core::GetFileNameFromFullPath(mapFilePath) + (isChanged ? "*" : "")).c_str() : u8"未命名")
				+ u8" | 缩放: " + std::to_string(mapScale).substr(0, 3) + "x";
			draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(textX, textY), IM_COL32_WHITE, info.c_str());

			ImGui::EndMenuBar();
		}

		if (tileMap != nullptr && tilesetTexture != nullptr)
		{
			int mapW = tileMap->getMapWidth();
			int mapH = tileMap->getMapHeight();
			const int tileSize = 32;
			float scaledTile = tileSize * mapScale;

			ImVec2 canvas_start = ImGui::GetCursorScreenPos();
			ImGui::InvisibleButton("##MapCanvas", ImVec2(mapW * scaledTile, mapH * scaledTile));

			float scrollX = ImGui::GetScrollX();
			float scrollY = ImGui::GetScrollY();
			ImVec2 draw_pos = canvas_start;
			draw_pos.x -= scrollX;
			draw_pos.y -= scrollY;

			// ========== 网格绘制（受showGrid控制）==========
			if (showGrid)
			{
				for (int x = 0; x <= mapW; x++)
				{
					ImVec2 p1(draw_pos.x + x * scaledTile, draw_pos.y);
					ImVec2 p2(draw_pos.x + x * scaledTile, draw_pos.y + mapH * scaledTile);
					draw_list->AddLine(p1, p2, IM_COL32(255, 255, 255, 120));
				}
				for (int y = 0; y <= mapH; y++)
				{
					ImVec2 p1(draw_pos.x, draw_pos.y + y * scaledTile);
					ImVec2 p2(draw_pos.x + mapW * scaledTile, draw_pos.y + y * scaledTile);
					draw_list->AddLine(p1, p2, IM_COL32(255, 255, 255, 120));
				}
			}

			int texW, texH;
			SDL_QueryTexture(tilesetTexture, nullptr, nullptr, &texW, &texH);
			int tilesPerRow = texW / tileSize;

			for (int y = 0; y < mapH; y++)
			{
				for (int x = 0; x < mapW; x++)
				{
					ImVec2 p0(draw_pos.x + x * scaledTile, draw_pos.y + y * scaledTile);
					ImVec2 p1(p0.x + scaledTile, p0.y + scaledTile);
					auto& tile = tileMap->GetTile(x, y);

					if (showBackgroundTiles && tile.background >= 0)
					{
						int tx = tile.background % tilesPerRow;
						int ty = tile.background / tilesPerRow;
						draw_list->AddImage((ImTextureID)tilesetTexture, p0, p1,
							ImVec2(tx * tileSize / (float)texW, ty * tileSize / (float)texH),
							ImVec2((tx + 1) * tileSize / (float)texW, (ty + 1) * tileSize / (float)texH));
					}
					if (showTerrainTiles && tile.terrain >= 0)
					{
						int tx = tile.terrain % tilesPerRow;
						int ty = tile.terrain / tilesPerRow;
						draw_list->AddImage((ImTextureID)tilesetTexture, p0, p1,
							ImVec2(tx * tileSize / (float)texW, ty * tileSize / (float)texH),
							ImVec2((tx + 1) * tileSize / (float)texW, (ty + 1) * tileSize / (float)texH));
					}
					if (showDecorationTiles && tile.decoration >= 0)
					{
						int tx = tile.decoration % tilesPerRow;
						int ty = tile.decoration / tilesPerRow;
						draw_list->AddImage((ImTextureID)tilesetTexture, p0, p1,
							ImVec2(tx * tileSize / (float)texW, ty * tileSize / (float)texH),
							ImVec2((tx + 1) * tileSize / (float)texW, (ty + 1) * tileSize / (float)texH));
					}
				}
			}

			ImVec2 mousePos = ImGui::GetMousePos();
			int hoverX = -1, hoverY = -1;

			if (ImGui::IsItemHovered())
			{
				hoverX = (int)((mousePos.x - draw_pos.x) / scaledTile);
				hoverY = (int)((mousePos.y - draw_pos.y) / scaledTile);
			}

			// ========== 悬停预览（区分放置/擦除模式）==========
			if (hoverX >= 0 && hoverY >= 0 && hoverX < mapW && hoverY < mapH)
			{
				ImVec2 p0(draw_pos.x + hoverX * scaledTile, draw_pos.y + hoverY * scaledTile);
				ImVec2 p1(p0.x + scaledTile, p0.y + scaledTile);

				// 擦除模式：纯黑半透明 + 白色边框，不显示瓦片
				if (EditMode)
				{
					draw_list->AddRectFilled(p0, p1, IM_COL32(0, 0, 0, 180)); // 纯黑半透明
					draw_list->AddRect(p0, p1, IM_COL32_WHITE, 0, 0, 2.0f);
				}
				// 放置模式：白色半透明 + 边框 + 选中瓦片预览
				else
				{
					if (selectedTileId >= 0)
					{
						draw_list->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, 80));
						draw_list->AddRect(p0, p1, IM_COL32_WHITE, 0, 0, 2.0f);

						int tx = selectedTileId % tilesPerRow;
						int ty = selectedTileId / tilesPerRow;
						draw_list->AddImage((ImTextureID)tilesetTexture, p0, p1,
							ImVec2(tx * tileSize / (float)texW, ty * tileSize / (float)texH),
							ImVec2((tx + 1) * tileSize / (float)texW, (ty + 1) * tileSize / (float)texH));
					}
				}
			}

			// 鼠标绘制逻辑（不变）
			if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Left)) && ImGui::IsItemHovered())
			{
				if (hoverX >= 0 && hoverY >= 0 && hoverX < mapW && hoverY < mapH)
				{
					if (!EditMode)
					{
						if (selectedTileId >= 0)
						{
							tileMap->SetTileLayer(hoverX, hoverY, editLayer, selectedTileId);
							isChanged = true;
						}
					}
					else
					{
						tileMap->SetTileLayer(hoverX, hoverY, editLayer, -1);
						isChanged = true;
					}
				}
			}
		}

		ImGui::End();
	}
#pragma endregion
#pragma region CreateWin
	if (showMapCreat)
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::OpenPopup(u8"新建关卡地图");
	}

	if (ImGui::BeginPopupModal(
		u8"新建关卡地图",
		&showMapCreat,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove
	)) {
		ImGui::TextDisabled(u8"*新建的地图默认保存到 map.csv 文件中");

		ImGui::Spacing();
		ImGui::Text(u8"尺寸:");
		ImGui::SameLine(0, 20);

		ImGui::PushItemWidth(80);
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
		ImGui::InputText("##MapWidth", &mapWidthStr, ImGuiInputTextFlags_CharsDecimal);
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		ImGui::SameLine(0, 15);

		ImGui::Text("x");
		ImGui::SameLine();
		ImGui::SameLine(0, 15);

		ImGui::PushItemWidth(80);
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
		ImGui::InputText("##MapHeight", &mapHeightStr, ImGuiInputTextFlags_CharsDecimal);
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();


		try {
			int w = std::stoi(mapWidthStr);
			int h = std::stoi(mapHeightStr);
			if (w < 1) mapWidthStr = "1";
			if (h < 1) mapHeightStr = "1";
		}
		catch (...) {
			mapWidthStr = "28";
			mapHeightStr = "15";
		}

		ImGui::SetCursorPosX(15);
		if (ImGui::Button(u8"确定", ImVec2(120, 0)))
		{
			NewBuilt();
			ImGui::CloseCurrentPopup();
			showMapCreat = false;
		}
		ImGui::SameLine(0, 22);
		if (ImGui::Button(u8"取消", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
			showMapCreat = false;
		}

		ImGui::EndPopup();
	}

#pragma endregion

#pragma region OpenSaveWarn
	if (OpenshowSaveWarn)
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::OpenPopup(u8"是否保存");
	}

	if (ImGui::BeginPopupModal(
		u8"是否保存",
		&OpenshowSaveWarn,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove
	)) {
		ImGui::TextDisabled(u8"*您似乎更改了文件，是否保存");

		ImGui::SetCursorPosX(15);
		if (ImGui::Button(u8"保存", ImVec2(120, 0)))
		{
			Save();
			Open();
			ImGui::CloseCurrentPopup();
			OpenshowSaveWarn = false;
		}
		ImGui::SameLine(0, 22);
		if (ImGui::Button(u8"不保存", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
			OpenshowSaveWarn = false;
		}

		ImGui::EndPopup();
	}
#pragma endregion

#pragma region NewSaveWarn
	if (NewshowSaveWarn)
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::OpenPopup(u8"保存是否");
	}

	if (ImGui::BeginPopupModal(
		u8"保存是否",
		&NewshowSaveWarn,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove
	)) {
		ImGui::TextDisabled(u8"*您似乎更改了文件，是否保存");

		ImGui::SetCursorPosX(15);
		if (ImGui::Button(u8"保存", ImVec2(120, 0)))
		{
			Save();
			showMapCreat = true;
			ImGui::CloseCurrentPopup();
			NewshowSaveWarn = false;
		}
		ImGui::SameLine(0, 22);
		if (ImGui::Button(u8"不保存", ImVec2(120, 0)))
		{
			showMapCreat = true;
			ImGui::CloseCurrentPopup();
			NewshowSaveWarn = false;
		}

		ImGui::EndPopup();
	}
#pragma endregion

}
void Online::TileEdit::TileEdit::Update()
{
}
void Online::TileEdit::TileEdit::LateUpdate()
{
}
void Online::TileEdit::TileEdit::Render()
{
	SDL_SetRenderDrawColor(renderer, Online::Core::Color::SkyBlue.r * 255, Online::Core::Color::SkyBlue.g * 255, Online::Core::Color::SkyBlue.b * 255, 255);
	SDL_RenderClear(renderer);

	ImGui::Render();
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
}
void Online::TileEdit::TileEdit::EndFrame()
{
	SDL_RenderPresent(renderer);
}
void Online::TileEdit::TileEdit::FrameSync()
{
	if (frame_time * 1000 > 1000.0 / 60)
		SDL_Delay((Uint32(1000.0 / 60) - frame_time));
}
void Online::TileEdit::TileEdit::Release()
{
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	if (tilesetSurface != nullptr)
	{
		SDL_FreeSurface(tilesetSurface);
		tilesetSurface = nullptr;
	}
	if (tilesetTexture != nullptr)
	{
		SDL_DestroyTexture(tilesetTexture);
		tilesetTexture = nullptr;
	}

	if (renderer != nullptr)
	{
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}
	if (window != nullptr)
	{
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	isRunning = false;
}

void Online::TileEdit::TileEdit::Save()
{
	if (tileMap == nullptr) return;
	isChanged = false;
	isFileOpen ? tileMap->serialize(mapFilePath) : tileMap->serialize();
}
void Online::TileEdit::TileEdit::SaveAs()
{
	if (tileMap == nullptr) return;
	isChanged = false;
	std::string path = Online::TileEdit::Dialog::SaveFileCsvDialog();
	path += ".csv";
	isFileOpen = true;
	mapFilePath = path;
	tileMap->serialize(path);
}
void Online::TileEdit::TileEdit::SaveCurrentView()
{
	std::string savePath = Online::TileEdit::Dialog::SaveFilePngDialog();
	if (savePath.empty())
		return;

	tileMap->SaveAsPNG(
		savePath,
		renderer,
		tilesetTexture,
		mapScale,
		showBackgroundTiles,
		showTerrainTiles,
		showDecorationTiles
	);
}

void Online::TileEdit::TileEdit::Open()
{
	std::string path = Online::TileEdit::Dialog::OpenFileDialog();
	if (!path.empty()) 
	{
		if(tileMap != nullptr)
			delete tileMap;
		tileMap = nullptr;
		tileMap = new TileMap();
		mapFilePath = path;
		isFileOpen = true;
		tileMap->deserialize(path);
	}
}
void Online::TileEdit::TileEdit::NewBuilt()
{
	if (tileMap != nullptr)
		delete tileMap;
	tileMap = nullptr;
	tileMap = new TileMap();
	int w = std::stoi(mapWidthStr);
	int h = std::stoi(mapHeightStr);
	tileMap->InitTileMap(w, h);
}

bool Online::TileEdit::TileEdit::IsTileEmpty(int tileX, int tileY)
{
	if (tilesetSurface == nullptr) return true;

	static const int tileSize = 32;
	int startX = tileX * tileSize;
	int startY = tileY * tileSize;

	if (startX + tileSize > tilesetSurface->w || startY + tileSize > tilesetSurface->h)
		return true;

	SDL_LockSurface(tilesetSurface);
	bool isEmpty = true;

	for (int y = startY; y < startY + tileSize; ++y)
	{
		for (int x = startX; x < startX + tileSize; ++x)
		{
			int bpp = tilesetSurface->format->BytesPerPixel;
			Uint8* pPixel = (Uint8*)tilesetSurface->pixels + y * tilesetSurface->pitch + x * bpp;
			Uint32 pixel = 0;

			if (bpp == 4) pixel = *(Uint32*)pPixel;
			else if (bpp == 3) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				pixel = (pPixel[0] << 16) | (pPixel[1] << 8) | pPixel[2];
#else
				pixel = (pPixel[2] << 16) | (pPixel[1] << 8) | pPixel[0];
#endif
			}
			else if (bpp == 2) pixel = *(Uint16*)pPixel;
			else if (bpp == 1) pixel = *pPixel;

			Uint8 r, g, b, a;
			SDL_GetRGBA(pixel, tilesetSurface->format, &r, &g, &b, &a);
			if (a > 0) { isEmpty = false; goto CheckEnd; }
		}
	}

CheckEnd:
	SDL_UnlockSurface(tilesetSurface);
	return isEmpty;
	return false;
}


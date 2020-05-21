#include "renderer.hpp"
#include "fonts.hpp"

namespace Prestige
{

	IDXGISwapChain* cRenderer::m_Swapchain;
	ID3D11Device* cRenderer::m_Device;
	ID3D11DeviceContext* cRenderer::m_Context;
	ID3D11RenderTargetView* cRenderer::m_RTV;
	HWND cRenderer::hWnd;
	ImFont* cRenderer::m_pFont;

	int cRenderer::Width;
	int cRenderer::Height;

	void cRenderer::Initialize(IDXGISwapChain* swapChain)
	{
		m_Swapchain = swapChain;
		if (FAILED(m_Swapchain->GetDevice(__uuidof(ID3D11Device), (void**)(&m_Device))))
			return;

		m_Device->GetImmediateContext(&m_Context);

		hWnd = FindWindow("UnrealWindow", NULL);

		RECT rct;
		GetClientRect(hWnd, &rct);
		Width = rct.right - rct.left;
		Height = rct.bottom - rct.top;

		m_pFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(fonts_compressed_data, fonts_compressed_size, 14.f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesKorean());

		ImGui::CreateContext();
		ImGui_ImplDX11_Init(hWnd, m_Device, m_Context);
		ImGui_ImplDX11_CreateDeviceObjects();

		CreateRenderTarget();
	}

	void cRenderer::ClearRenderTarget()
	{
		if (m_RTV)
		{
			m_RTV->Release();
			m_RTV = NULL;
		}
	}

	void cRenderer::CreateRenderTarget()
	{
		ID3D11Texture2D* pBackBuffer;
		m_Swapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		m_Device->CreateRenderTargetView(pBackBuffer, NULL, &m_RTV);
		pBackBuffer->Release();
	}

	void cRenderer::BeginScene()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::Begin("##Backbuffer", reinterpret_cast<bool*>(true), ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);

		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
		ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiSetCond_Always);
	}

	void cRenderer::EndScene()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		window->DrawList->PushClipRectFullScreen();

		ImGui::End();
		ImGui::PopStyleColor();

		ImGui::Render();
		m_Context->OMSetRenderTargets(1, &m_RTV, NULL);
		ImGui_ImplDX11_RenderDrawLists(ImGui::GetDrawData());
	}

	void cRenderer::PreResize()
	{
		RECT rct;
		GetClientRect(hWnd, &rct);
		Width = rct.right - rct.left;
		Height = rct.bottom - rct.top;
		ImGui_ImplDX11_InvalidateDeviceObjects();
		ClearRenderTarget();
	}

	void cRenderer::PostResize()
	{
		CreateRenderTarget();
		ImGui_ImplDX11_CreateDeviceObjects();
	}

	void cRenderer::DrawBox(const ImVec2& pos, const ImVec2& headPosition, uint32_t color)
	{
		DrawLine(ImVec2(pos.x, pos.y), ImVec2(pos.x + headPosition.x, pos.y), color, 1.f);
		DrawLine(ImVec2(pos.x, pos.y), ImVec2(pos.x, pos.y + headPosition.y), color, 1.f);
		DrawLine(ImVec2(pos.x, pos.y + headPosition.y), ImVec2(pos.x + headPosition.x, pos.y + headPosition.y), color, 1.f);
		DrawLine(ImVec2(pos.x + headPosition.x, pos.y), ImVec2(pos.x + headPosition.x, pos.y + headPosition.y), color, 1.f);
	}

	float cRenderer::DrawOutlinedText(const std::string& text, const ImVec2& position, float size, uint32_t color, bool center)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		float a = (color >> 24) & 0xff;
		float r = (color >> 16) & 0xff;
		float g = (color >> 8) & 0xff;
		float b = (color) & 0xff;
		float y = 0.0f;
		int i = 0;
		ImVec2 textSize = m_pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

		if (center)
		{
			window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) + 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) - 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) + 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) - 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());

			window->DrawList->AddText(m_pFont, size, { position.x - textSize.x / 2.0f, position.y + textSize.y * i }, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), text.c_str());
		}
		else
		{
			window->DrawList->AddText(m_pFont, size, { (position.x) + 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x) - 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x) + 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x) - 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());

			window->DrawList->AddText(m_pFont, size, { position.x, position.y + textSize.y * i }, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), text.c_str());
		}
		return 1.f;
	}

	float cRenderer::DrawOutlinedText(const std::wstring& wtext, const ImVec2& position, float size, uint32_t color, bool center)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		std::string text(wtext.begin(), wtext.end());

		float a = (color >> 24) & 0xff;
		float r = (color >> 16) & 0xff;
		float g = (color >> 8) & 0xff;
		float b = (color) & 0xff;
		float y = 0.0f;
		int i = 0;
		ImVec2 textSize = m_pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

		if (center)
		{
			window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) + 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) - 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) + 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x - textSize.x / 2.0f) - 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());

			window->DrawList->AddText(m_pFont, size, { position.x - textSize.x / 2.0f, position.y + textSize.y * i }, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), text.c_str());
		}
		else
		{
			window->DrawList->AddText(m_pFont, size, { (position.x) + 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x) - 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x) + 1.0f, (position.y + textSize.y * i) - 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());
			window->DrawList->AddText(m_pFont, size, { (position.x) - 1.0f, (position.y + textSize.y * i) + 1.0f }, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, a / 255.0f }), text.c_str());

			window->DrawList->AddText(m_pFont, size, { position.x, position.y + textSize.y * i }, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), text.c_str());
		}
		return 1.f;
	}

	void cRenderer::DrawBoxFilled(const ImVec2& from, const ImVec2& to, uint32_t color, float rounding, uint32_t roundingCornersFlags)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		float a = (color >> 24) & 0xFF;
		float r = (color >> 16) & 0xFF;
		float g = (color >> 8) & 0xFF;
		float b = (color) & 0xFF;

		window->DrawList->AddRectFilled(from, to, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), rounding, roundingCornersFlags);
	}


	void cRenderer::DrawLine(const ImVec2& from, const ImVec2& to, uint32_t color, float thickness)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		float a = (color >> 24) & 0xff;
		float r = (color >> 16) & 0xff;
		float g = (color >> 8) & 0xff;
		float b = (color) & 0xff;

		window->DrawList->AddLine(from, to, ImGui::GetColorU32(ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f)), thickness);
	}

	void cRenderer::DrawCircle(const ImVec2& position, float radius, uint32_t color, float thickness)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		float a = (color >> 24) & 0xff;
		float r = (color >> 16) & 0xff;
		float g = (color >> 8) & 0xff;
		float b = (color) & 0xff;

		window->DrawList->AddCircle(position, radius, ImGui::GetColorU32(ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f)), 64, thickness);
	}

	void cRenderer::DrawCircleScale(const ImVec2& position, float radius, uint32_t color, const ImVec2& scalepos, const ImVec2& scaleheadPosition, float thickness)
	{
		float rad = (scaleheadPosition.y + 15 - scalepos.y) / 10.5f;
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		float a = (color >> 24) & 0xff;
		float r = (color >> 16) & 0xff;
		float g = (color >> 8) & 0xff;
		float b = (color) & 0xff;

		window->DrawList->AddCircle(position, rad, ImGui::GetColorU32(ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f)), 64, thickness);
	}

	void cRenderer::DrawCircleFilled(const ImVec2& position, float radius, uint32_t color)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		float a = (color >> 24) & 0xff;
		float r = (color >> 16) & 0xff;
		float g = (color >> 8) & 0xff;
		float b = (color) & 0xff;

		window->DrawList->AddCircleFilled(position, radius, ImGui::GetColorU32(ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f)), 64);
	}

	void cRenderer::RenderCircle(const ImVec2& position, float radius, uint32_t color, float thickness, uint32_t segments)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		float a = (color >> 24) & 0xff;
		float r = (color >> 16) & 0xff;
		float g = (color >> 8) & 0xff;
		float b = (color) & 0xff;

		window->DrawList->AddCircle(position, radius, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), segments, thickness);
	}

	void cRenderer::DrawImage(int x, int y, int w, int h, ID3D11ShaderResourceView* texture)
	{
		float OriginX = ImGui::GetCursorPosX();
		float OriginY = ImGui::GetCursorPosY();
		ImGui::SetCursorPosX(x);  ImGui::SetCursorPosY(y);
		ImGui::Image((void*)texture, ImVec2((float)w, (float)h), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(255, 0, 0, 0));
		ImGui::SetCursorPosX(OriginX);  ImGui::SetCursorPosY(OriginY);
	}

	void cRenderer::DrawSkeleton(uint64_t Mesh, FCameraCacheEntry CameraCache, uint32_t color, bool IsMale)
	{
		int skeleton[][4] = {
			{ 7, 8, 0, 0},
			{ 7, IsMale ? 47 : 46, IsMale ? 48 : 47, IsMale ? 49 : 48 },
			{ 7, 21, 22, 23 },
			{ 7, 5, 4, 3 },
			{ 3, IsMale ? 82 : 80, IsMale ? 83 : 81, IsMale ? 85 : 83 },
			{ 3, IsMale ? 75 : 73, IsMale ? 76 : 74, IsMale ? 78 : 76 }
		};

		for (auto part : skeleton)
		{
			FVector previous{};
			for (int i = 0; i < 4; i++)
			{
				if (!part[i]) break;

				FVector current = Engine::GetBoneWithRotation(Mesh, part[i]);
				if (previous.X == 0.f)
				{
					previous = current;
					continue;
				}
				FVector p1{}, c1{};
				p1 = Engine::WorldToScreen(previous, CameraCache);
				c1 = Engine::WorldToScreen(current, CameraCache);
				cRenderer::DrawLine(ImVec2(p1.X, p1.Y), ImVec2(c1.X, c1.Y), color);
				previous = current;
			}
		}
	}

}
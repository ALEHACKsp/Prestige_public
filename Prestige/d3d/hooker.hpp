#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <d3d11.h>
#include "minhook/MinHook.h"
#include "../memory.hpp"
#include "../utils.hpp"
#pragma comment (lib, "MinHook.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3d11.lib")

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static IDXGISwapChain* GetSwapChain()
{
	HMODULE hDXGIDLL = 0;
	do
	{
		hDXGIDLL = GetModuleHandle("dxgi.dll");
		Sleep(1);
	} while (!hDXGIDLL);

	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(swapDesc));
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = FindWindow((LPCSTR)"UnrealWindow", (LPCSTR)nullptr);
	swapDesc.SampleDesc.Count = 1;
	swapDesc.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	IDXGISwapChain* pSwapChain = 0;
	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pContext = NULL;
	if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDesc,
		&pSwapChain,
		&pDevice,
		NULL,
		&pContext)))
	{
		pContext->Release();
		pDevice->Release();
		return pSwapChain;
	}
	else
		return NULL;
}

extern void* o_present;
HRESULT hk_present(IDXGISwapChain* dis, UINT syncInterval, UINT flags);
extern void* o_resizebuffers;
HRESULT hk_resizebuffers(IDXGISwapChain* dis, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);

typedef struct HookBackUp
{
	char pad[0x14];
};

static void HookD3D11()
{
	IDXGISwapChain* m_pSwapChain = GetSwapChain();

	if (m_pSwapChain)
	{
		utils::ThreadStuff(true);
		DWORD_PTR* pSwapChainVtable = (DWORD_PTR*)m_pSwapChain;
		pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];
		uint64_t dwPresent = (uint64_t)(DWORD_PTR*)pSwapChainVtable[8];
		uint64_t dwResizeBuffers = (uint64_t)(DWORD_PTR*)pSwapChainVtable[13];

		DWORD OldProtect{}, OldProtect2{};
		VirtualProtect((void*)dwPresent, 2, PAGE_EXECUTE_READWRITE, &OldProtect);
		VirtualProtect((void*)dwResizeBuffers, 2, PAGE_EXECUTE_READWRITE, &OldProtect2);

		MH_Initialize();
		MH_CreateHook((void*)dwPresent, (void*)hk_present, reinterpret_cast<void**>(&o_present));
		MH_EnableHook((void*)dwPresent);
		MH_CreateHook((void*)dwResizeBuffers, (void*)hk_resizebuffers, reinterpret_cast<void**>(&o_resizebuffers));
		MH_EnableHook((void*)dwResizeBuffers);

		/*uint64_t JmpedAddr = dwPresent + *(int*)(dwPresent + 0x1) + 0x5;
		HookBackUp RealJmp = mem::RPM<HookBackUp>(JmpedAddr);
		mem::WPM<HookBackUp>(Prestige::Engine::retaddr + 0x2, RealJmp);
		mem::WPM<uint64_t>(JmpedAddr + 0x6, Prestige::Engine::retaddr + 0x2);*/

		VirtualProtect((void*)dwPresent, 2, OldProtect, &OldProtect);
		VirtualProtect((void*)dwResizeBuffers, 2, OldProtect2, &OldProtect2);
		utils::ThreadStuff(false);
	}
}
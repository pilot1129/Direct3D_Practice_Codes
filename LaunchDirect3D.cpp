#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <dxgi.h>		// IDXGI�� ����ϱ� ���� Header
#include <iostream>
#include <string>		// wstring Header
#include <vector>
#include <d3dcompiler.h>
#include <d3d12.h>		// CommandQueue�� ����ϱ� ���� Header
#include <wrl.h>		// ComPtr�� ����ϱ� ���� Header
#include <Windows.h>
#include <cassert>		// assert ����ϱ� ���� Header
#include <WinUser.h>
#include <exception>	// throw
#include <DirectXColors.h>

using namespace Microsoft::WRL;
using namespace DirectX::Colors;
using namespace std;

/* WIKI
	* static_cast	: Compile Time�� Error�� Detect �ϱ� ���ؼ� ���
	* |				: ������ OR ������(Bit OR)
	* _countof		: sizeof(arr) / sizeof(arr[0]) ��, ������ ���� ���
*/

namespace DX
{
	class com_exception : public exception
	{
	public:
		com_exception(HRESULT hr) : result(hr) {}
		const char* what() const override
		{
			static char s_str[64] = {};
			sprintf_s(s_str, "%08X �� ������ �����Ͽ����ϴ�", static_cast<unsigned int>(result));
			return s_str;
		}
	private:
		HRESULT result;
	};

	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
			throw com_exception(hr);
	}
}

void InitDirect3D()
{
	static const int SwapChainBufferCount = 2;
	int CurBackBuffer = 0;
	UINT pRtvDescriptorSize = 0;
	UINT pDsvDescriptorSize = 0;
	UINT pCbvSrvDescriptorSize = 0;
	UINT pCurrentFence = 0;
	D3D12_VIEWPORT pScreenViewport;
	D3D12_RECT pScissorRect;

	wstring pMainWindowCaption = L"Direct3D";
	D3D_DRIVER_TYPE pD3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT pBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT pDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int pClinetWIdth = 800;
	int pClientHeight = 600;

	ComPtr<IDXGIFactory1> pDxgiFactory;
	ComPtr<IDXGISwapChain> pSwapChain;
	ComPtr<ID3D12Device> pD3dDevice;
	ComPtr<ID3D12Fence> pFence;
	ComPtr<ID3D12CommandQueue> pCommandQueue;
	ComPtr<ID3D12CommandAllocator> pDirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList> pCommandList;
	ComPtr<ID3D12Resource> pSwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> pDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> pRtvHeap;
	ComPtr<ID3D12DescriptorHeap> pDsvHeap;

	
	//��� ��Ͽ� ���õ� �޸��� ��Ȱ���� ���� ��� �Ҵ��� �缳��
	DX::ThrowIfFailed(pDirectCmdListAlloc->Reset());

	//��� ��� �缳��
	DX::ThrowIfFailed(pCommandList->Reset(pDirectCmdListAlloc.Get(), nullptr));
	
	// �ڿ� �뵵�� ���õ� ���� ���̸� Direct3D�� ����[�ڿ��� �ʱ���� -> Depth Buffer�� ����]
	D3D12_RESOURCE_BARRIER BarrierResult = {};
	D3D12_RESOURCE_BARRIER &barrierRes = BarrierResult;
	BarrierResult.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierResult.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierRes.Transition.pResource = pSwapChainBuffer[CurBackBuffer].Get();
	barrierRes.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierRes.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierRes.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pCommandList->ResourceBarrier(1, &BarrierResult);

	//����Ʈ ����(���� ���簢�� ���� ����)
	pCommandList->RSSetViewports(1, &pScreenViewport);
	pCommandList->RSSetScissorRects(1, &pScissorRect);

	//�ĸ� ���� �� ���� ���� ����
	D3D12_CPU_DESCRIPTOR_HANDLE Temp;
	Temp.ptr = SIZE_T(INT64(pRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr) + INT64(CurBackBuffer) * INT64(pRtvDescriptorSize));
	pCommandList->ClearRenderTargetView(Temp, LightSkyBlue, 0, nullptr); // FIX
	pCommandList->ClearDepthStencilView(pDsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	//������ ����� ��ϵ� ���� ��� ���� ����
	pCommandList->OMSetRenderTargets(1, &Temp, true, &pDsvHeap->GetCPUDescriptorHandleForHeapStart());

	//�ڿ� �뵵�� ���õ� ���� ���̸� Direct3D�� ����
	barrierRes.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierRes.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	pCommandList->ResourceBarrier(1, &BarrierResult);

	//��� ��� ����
	DX::ThrowIfFailed(pCommandList->Close());

	//��� ��� ��� ��⿭�� �߰�(���� ����)
	ID3D12CommandList* cmdList[] = { pCommandList.Get() };
	pCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

	//�ĸ� ���� �� ���� ���� ��ȯ
	DX::ThrowIfFailed(pSwapChain->Present(0, 0));
	CurBackBuffer = (CurBackBuffer + 1) % SwapChainBufferCount;

	//��� ���(Flush Command Queue)
	pCurrentFence++; // ����� �մ�� ���� Fence�������� ǥ��
	DX::ThrowIfFailed(pCommandQueue->Signal(pFence.Get(), pCurrentFence)); // ��� ��⿭�� ��� �߰� �� �� �潺 ���� ����
	if (pFence->GetCompletedValue() < pCurrentFence) // ��� �Ϸ� ���
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		DX::ThrowIfFailed(pFence->SetEventOnCompletion(pCurrentFence, eventHandle)); // ���� �潺 ���� �� �̺�Ʈ ����
		WaitForSingleObject(eventHandle, INFINITE); // �̺�Ʈ ���� ���
		CloseHandle(eventHandle);
	}
}

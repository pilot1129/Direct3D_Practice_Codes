#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <dxgi.h>		// IDXGI를 사용하기 위한 Header
#include <iostream>
#include <string>		// wstring Header
#include <vector>
#include <d3dcompiler.h>
#include <d3d12.h>		// CommandQueue를 사용하기 위한 Header
#include <wrl.h>		// ComPtr를 사용하기 위한 Header
#include <Windows.h>
#include <cassert>		// assert 사용하기 위한 Header
#include <WinUser.h>
#include <exception>	// throw
#include <DirectXColors.h>

using namespace Microsoft::WRL;
using namespace DirectX::Colors;
using namespace std;

/* WIKI
	* static_cast	: Compile Time에 Error을 Detect 하기 위해서 사용
	* |				: 포괄적 OR 연산자(Bit OR)
	* _countof		: sizeof(arr) / sizeof(arr[0]) 즉, 원소의 갯수 출력
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
			sprintf_s(s_str, "%08X 의 사유로 실패하였습니다", static_cast<unsigned int>(result));
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

	
	//명령 기록에 관련된 메모리의 재활용을 위해 명령 할당자 재설정
	DX::ThrowIfFailed(pDirectCmdListAlloc->Reset());

	//명령 목록 재설정
	DX::ThrowIfFailed(pCommandList->Reset(pDirectCmdListAlloc.Get(), nullptr));
	
	// 자원 용도에 관련된 상태 전이를 Direct3D에 통지[자원의 초기상태 -> Depth Buffer로 전이]
	D3D12_RESOURCE_BARRIER BarrierResult = {};
	D3D12_RESOURCE_BARRIER &barrierRes = BarrierResult;
	BarrierResult.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierResult.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierRes.Transition.pResource = pSwapChainBuffer[CurBackBuffer].Get();
	barrierRes.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierRes.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierRes.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pCommandList->ResourceBarrier(1, &BarrierResult);

	//뷰포트 설정(가위 직사각형 또한 설정)
	pCommandList->RSSetViewports(1, &pScreenViewport);
	pCommandList->RSSetScissorRects(1, &pScissorRect);

	//후면 버퍼 및 깊이 버퍼 제거
	D3D12_CPU_DESCRIPTOR_HANDLE Temp;
	Temp.ptr = SIZE_T(INT64(pRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr) + INT64(CurBackBuffer) * INT64(pRtvDescriptorSize));
	pCommandList->ClearRenderTargetView(Temp, LightSkyBlue, 0, nullptr); // FIX
	pCommandList->ClearDepthStencilView(pDsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	//렌더링 결과가 기록될 렌더 대상 버퍼 지정
	pCommandList->OMSetRenderTargets(1, &Temp, true, &pDsvHeap->GetCPUDescriptorHandleForHeapStart());

	//자원 용도에 관련된 상태 전이를 Direct3D에 통지
	barrierRes.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierRes.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	pCommandList->ResourceBarrier(1, &BarrierResult);

	//명령 기록 종료
	DX::ThrowIfFailed(pCommandList->Close());

	//명령 목록 명렬 대기열에 추가(실행 위해)
	ID3D12CommandList* cmdList[] = { pCommandList.Get() };
	pCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

	//후면 버퍼 및 전면 버퍼 교환
	DX::ThrowIfFailed(pSwapChain->Present(0, 0));
	CurBackBuffer = (CurBackBuffer + 1) % SwapChainBufferCount;

	//명령 대기(Flush Command Queue)
	pCurrentFence++; // 명령을 앞당겨 현재 Fence지점까지 표시
	DX::ThrowIfFailed(pCommandQueue->Signal(pFence.Get(), pCurrentFence)); // 명령 대기열에 명령 추가 후 새 펜스 지점 설정
	if (pFence->GetCompletedValue() < pCurrentFence) // 명령 완료 대기
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		DX::ThrowIfFailed(pFence->SetEventOnCompletion(pCurrentFence, eventHandle)); // 현재 펜스 도달 시 이벤트 실행
		WaitForSingleObject(eventHandle, INFINITE); // 이벤트 실행 대기
		CloseHandle(eventHandle);
	}
}

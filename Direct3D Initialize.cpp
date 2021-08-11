#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")
#include <dxgi.h> // IDXGI를 사용하기 위한 Header
#include <iostream>
#include <string> // wstring Header
#include <vector>
#include <d3d12.h> // CommandQueue를 사용하기 위한 Header
#include <wrl.h> // ComPtr를 사용하기 위한 Header
#include <Windows.h>
#include <cassert> // assert 사용하기 위한 Header
#include <WinUser.h>
//#include <afxwin.h>

using namespace Microsoft::WRL;
using namespace std;

void ControlCommandQueue()
{
	static const int SwapChainBufferCount = 2; // 전면buffer , 후면buffer 
	ComPtr<IDXGIFactory1> pFactory;
	ComPtr<IDXGIAdapter1> pAdapter;
	ComPtr<ID3D12Fence> pFence;
	ComPtr<ID3D12Device> testDevice;
	ComPtr<ID3D12CommandQueue> pCommandQueue;
	ComPtr<ID3D12CommandAllocator> pDirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList> pCommandList;
	ComPtr<IDXGISwapChain> pSwapChain;
	ComPtr<ID3D12DescriptorHeap> pRtvHeap;
	ComPtr<ID3D12DescriptorHeap> pDsvHeap;
	ComPtr<ID3D12Resource> pSwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> pDepthStencilBuffer[SwapChainBufferCount];

	CreateDXGIFactory1(IID_PPV_ARGS(&pFactory)); // IDXGIFactrory1 설정[교환 사슬 생성]
	pFactory->EnumAdapters1(0, &pAdapter); // adapter 열거
	IDXGIAdapter1* adapter = pAdapter.Get(); // adapter Get으로 값 불러오기

	//1. ID3D12Device (DisplayAdapter을 나타내는 device) 생성
	HRESULT hardwareRes = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&testDevice));

	//2. 울타리 생성 및 서술자 크기 획득
	testDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
	UINT pRtvDescriptorSize = testDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT pDsvDescriptorSize = testDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	UINT pCbvSrvDescriptorSize = testDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//3. 4X MSAA 품질 수준 지원 점검
	DXGI_FORMAT BufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MultiSampleQualityLevels;
	UINT MaxQualityLevel = 0;
	MultiSampleQualityLevels.Format = BufferFormat;
	MultiSampleQualityLevels.SampleCount = 4;
	MultiSampleQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MultiSampleQualityLevels.NumQualityLevels = 0;
	testDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MultiSampleQualityLevels, sizeof(MultiSampleQualityLevels));
	MaxQualityLevel = MultiSampleQualityLevels.NumQualityLevels;

	assert(MaxQualityLevel > 0 && "Unexpected MSAA quallity level.");

	//4. 명령 대기열 및 명령 목록 생성
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	testDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pCommandQueue));
	testDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(pDirectCmdListAlloc.GetAddressOf()));
	testDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pDirectCmdListAlloc.Get(), nullptr, IID_PPV_ARGS(pCommandList.GetAddressOf()));
	pCommandList->Close();

	//5. 교환 사슬 서술 및 생성
	pSwapChain.Reset(); // 기존 교환 사슬 해제
	DXGI_SWAP_CHAIN_DESC scd;
	HINSTANCE hInstance = 0;
	//hInstance = AfxGetInstanceHandle(); // include<afxwin.h> 필요
	HWND g_hWnd = CreateWindowExW(WS_EX_TOPMOST, (LPCWSTR)"PopUp", (LPCWSTR)"Direct3D", WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	scd.BufferDesc.Width = 1920;
	scd.BufferDesc.Height = 1080;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = BufferFormat;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.SampleDesc.Count = MaxQualityLevel ? 4 : 1;
	scd.SampleDesc.Quality = MaxQualityLevel ? (MaxQualityLevel - 1) : 0;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = SwapChainBufferCount;
	scd.OutputWindow = g_hWnd;
	scd.Windowed = true;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	pFactory->CreateSwapChain(pCommandQueue.Get(), &scd, pSwapChain.GetAddressOf());

	//6. 서술자 Heap 생성
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	testDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(pRtvHeap.GetAddressOf()));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	testDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(pDsvHeap.GetAddressOf()));
	
	// 7. 후면 버퍼 크기 설정 및 렌더 대상 뷰 생성
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(pRtvHeap->GetCPUDescriptorHandleForHeapStart());
	pSwapChainBuffer->Get();
	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		testDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pDirectCmdListAlloc));
		
		/* 작동안함 - pSwapChainBuffer안에 아무것도 없음(NULL)
		// swapchain의 i번째 buffer read
		pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pSwapChainBuffer[i]));
		// buffer에 대한 RTV 생성
		testDevice->CreateRenderTargetView(pSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		// Heap의 다음 항목으로 Pass
		rtvHeapHandle.ptr = size_t(INT64(rtvHeapHandle.ptr) + INT64(1) * INT64(pRtvDescriptorSize)); // CD3DX12_CPU_DESCRIPTOR_HANDLE의 OFFSET 함수를 Copy
		*/
	}

	// 8. depth buffer 생성
	D3D12_HEAP_PROPERTIES heapProps;
	heapProps = testDevice->GetCustomHeapProperties(0, D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC depthStencilDesc;
	DXGI_FORMAT pDepthStencilFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = 1920;
	depthStencilDesc.Height = 1080;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = pDepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = MaxQualityLevel ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = MaxQualityLevel ? (MaxQualityLevel - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = pDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	testDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(pDepthStencilBuffer[0].GetAddressOf()));

	//전체 자원이 밉맵수준 0에 대한 서술자를, 해당 자원의 픽셀 형식을 적용해서 생성
	testDevice->CreateDepthStencilView(pDepthStencilBuffer->Get(), nullptr, pRtvHeap->GetCPUDescriptorHandleForHeapStart());
	//자원을 초기 상태에서 깊이 버퍼로 사용할 수 있는 상태로 전이
	//CD3DX12_RESOURCE_BARRIER Transition 분석
	D3D12_RESOURCE_BARRIER BarrierResult = {};
	D3D12_RESOURCE_BARRIER &barrierRes = BarrierResult;
	BarrierResult.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierResult.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierRes.Transition.pResource = pDepthStencilBuffer->Get();
	barrierRes.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrierRes.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrierRes.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pCommandList->ResourceBarrier(1, &BarrierResult);
	
	//9. 뷰포트 설정
	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(1920);
	vp.Height = static_cast<float>(1080);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pCommandList->RSSetViewports(1, &vp);

	//10. 가위 직사각형 설정
	D3D12_RECT pScissorRect = { 0,0,1920 / 2,1080 / 2 };
	pCommandList->RSSetScissorRects(1, &pScissorRect);
}

int main()
{
	ControlCommandQueue();
	return 0;
}
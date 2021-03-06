#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")
#include <dxgi.h> // IDXGIAdapter를 사용하기 위한 Header
#include <iostream>
#include <string> // wstring Header
#include <vector>
#include <wrl.h>  //ComPtr를 사용하기 위한 Header
#include <d3d12.h> //featurelevelsinfo 를 사용하기 위한 Header

using namespace std;
using namespace Microsoft::WRL;


void PrintDisplayMode()
{
	ComPtr<IDXGIFactory1> pFactory;
	ComPtr<IDXGIAdapter1> pAdapter;
	ComPtr<ID3D12Device> testDevice;
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};

	// IDXGIFactrory1 설정
	CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	// adapter 열거
	pFactory->EnumAdapters1(0, &pAdapter); 
	// adapter Get으로 값 불러오기
	IDXGIAdapter1* adapter = pAdapter.Get(); 
	//DisplayAdapter을 나타내는 device 생성
	D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&testDevice));
	//CheckFeatureSupport : 기능지원점검
	testDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData));
}

int main()
{
	PrintDisplayMode();
	return 0;
}
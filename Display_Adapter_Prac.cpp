#pragma comment(lib, "dxgi.lib") // 넣지않으면 LNK2019, LNK1120 ERROR
#include <dxgi.h> // IDXGIAdapter interface를 사용하기 위한 Header
#include <iostream>
#include <string> // wstring Header

using namespace std;

// Display Adapter Log Print)
void LogAdapters()
{
	// 1. IDXGIAdapter 초기화
	IDXGIAdapter* adapter = nullptr;
	int i = 0;
	
	// 2. IDXGIFactory1 초기화
	IDXGIFactory1* pFactory = nullptr;
	
	// 3. IDXGIFactory1 Create
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	
	//DXGI_ERROR_NOT_FOUND : adapter error
	//pFactory->EnumAdapters : adapter 열거
	while (pFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc; //List 불러옴
		adapter->GetDesc(&desc); //List 입력
		wstring text = desc.Description; // 출력
		std::wcout << text << endl;
		++i;
	}
}
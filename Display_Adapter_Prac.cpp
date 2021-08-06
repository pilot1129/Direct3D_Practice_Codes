#pragma comment(lib, "dxgi.lib") // ���������� LNK2019, LNK1120 ERROR
#include <dxgi.h> // IDXGIAdapter interface�� ����ϱ� ���� Header
#include <iostream>
#include <string> // wstring Header

using namespace std;

// Display Adapter Log Print)
void LogAdapters()
{
	// 1. IDXGIAdapter �ʱ�ȭ
	IDXGIAdapter* adapter = nullptr;
	int i = 0;
	
	// 2. IDXGIFactory1 �ʱ�ȭ
	IDXGIFactory1* pFactory = nullptr;
	
	// 3. IDXGIFactory1 Create
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	
	//DXGI_ERROR_NOT_FOUND : adapter error
	//pFactory->EnumAdapters : adapter ����
	while (pFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc; //List �ҷ���
		adapter->GetDesc(&desc); //List �Է�
		wstring text = desc.Description; // ���
		std::wcout << text << endl;
		++i;
	}
}
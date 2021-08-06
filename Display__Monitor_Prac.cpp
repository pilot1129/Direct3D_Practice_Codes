#pragma comment(lib, "dxgi.lib")
#include <dxgi.h> // IDXGIAdapter interface를 사용하기 위한 Header
#include <iostream>
#include <string> // wstring Header
#include <vector>

using namespace std;

// System에 존재하는 Monitor Print
void DisplayAdapters()
{
	// 1. Init
	IDXGIAdapter* adapter = nullptr;
	IDXGIFactory1* pFactory = nullptr;
	vector<IDXGIAdapter*> adapterList;
	int i = 0;

	// 2. IDXGIFactory1 Setting
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	
	//3. adapter 입력
	while (pFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		adapterList.push_back(adapter);
		++i;
	}
	i = 0;

	//4. Display출력
	IDXGIOutput* output = nullptr;
	for (int j = 0; j < adapterList.size(); ++j)
	{
		adapter = adapterList[j];
		//adapter->EnumOutputs : adapter 열거
		while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc; // List 불러옴
			output->GetDesc(&desc); // List 입력
			wstring text = desc.DeviceName; // 출력
			std::wcout << text << endl;
			++i;
		}
	}
	
}
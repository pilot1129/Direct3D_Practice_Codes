#pragma comment(lib, "dxgi.lib")
#include <dxgi.h> // IDXGIAdapter interface를 사용하기 위한 Header
#include <iostream>
#include <string> // wstring Header
#include <vector>

using namespace std;

// System에 존재하는 Monitor Print
void DisplayAdapters()
{
	IDXGIAdapter* adapter = nullptr;
	vector<IDXGIAdapter*> adapterList;
	int i = 0;
	IDXGIFactory1* pFactory = nullptr;
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	
	//1. adapter 입력
	while (pFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		adapterList.push_back(adapter);
		++i;
	}
	i = 0;

	//2. Display출력
	IDXGIOutput* output = nullptr;
	for (int j = 0; j < adapterList.size(); ++j)
	{
		adapter = adapterList[j];
		while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);
			wstring text = desc.DeviceName; // 출력
			std::wcout << text << endl;
			++i;
		}
	}
	
}
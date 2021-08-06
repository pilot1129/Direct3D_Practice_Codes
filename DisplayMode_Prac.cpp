#pragma comment(lib, "dxgi.lib")
#include <dxgi.h> // IDXGIAdapter를 사용하기 위한 Header
#include <iostream>
#include <string> // wstring Header
#include <vector>

using namespace std;

//DisplayMode를 출력하기 위한 함수
void PrintDisplayMode()
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

	//2. Display출력 setting
	IDXGIOutput* output = nullptr;
	for (int j = 0; j < adapterList.size(); ++j)
	{
		adapter = adapterList[j];
		while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);

			// 3. Display Mode 출력
			UINT count = 0;
			UINT flags = 0;

			output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, flags, &count, nullptr);
			vector<DXGI_MODE_DESC> modeList(count);
			output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, flags, &count, &modeList[0]);

			for (auto& x : modeList)
			{
				UINT n = x.RefreshRate.Numerator;
				UINT d = x.RefreshRate.Denominator;
				wstring text = L"Width = " + to_wstring(x.Width) + L"Height = " + to_wstring(x.Height) + L"Refresh = " + to_wstring(n) + L"/" + to_wstring(d);
				wcout << text << endl;
			}
			++i;
		}
	}
}

int main()
{
	PrintDisplayMode();
	return 0;
}
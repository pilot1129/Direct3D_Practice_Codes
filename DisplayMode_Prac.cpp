#pragma comment(lib, "dxgi.lib")
#include <dxgi.h> // IDXGIAdapter�� ����ϱ� ���� Header
#include <iostream>
#include <string> // wstring Header
#include <vector>

using namespace std;

//DisplayMode�� ����ϱ� ���� �Լ�
void PrintDisplayMode()
{
	// 1. init
	IDXGIAdapter* adapter = nullptr;
	IDXGIFactory1* pFactory = nullptr;
	vector<IDXGIAdapter*> adapterList;
	int i = 0;
	
	// 2. pFactory Setting
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));

	//3. adapter Setting
	while (pFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		adapterList.push_back(adapter);
		++i;
	}
	i = 0;

	//4. Display��� setting
	IDXGIOutput* output = nullptr;
	for (int j = 0; j < adapterList.size(); ++j)
	{
		adapter = adapterList[j];
		//pFactory->EnumAdapters : adapter ����
		while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc); // desc ����

			//Display Mode ���
			UINT count = 0;
			UINT flags = 0;

			// List�� ũ��(count) �ҷ�����
			output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, flags, &count, nullptr);
			vector<DXGI_MODE_DESC> modeList(count);
			// DisplayModeList ����
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
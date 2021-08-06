#pragma comment(lib, "dxgi.lib")
#include <dxgi.h> // IDXGIAdapter interface�� ����ϱ� ���� Header
#include <iostream>
#include <string> // wstring Header
#include <vector>

using namespace std;

// System�� �����ϴ� Monitor Print
void DisplayAdapters()
{
	// 1. Init
	IDXGIAdapter* adapter = nullptr;
	IDXGIFactory1* pFactory = nullptr;
	vector<IDXGIAdapter*> adapterList;
	int i = 0;

	// 2. IDXGIFactory1 Setting
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	
	//3. adapter �Է�
	while (pFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		adapterList.push_back(adapter);
		++i;
	}
	i = 0;

	//4. Display���
	IDXGIOutput* output = nullptr;
	for (int j = 0; j < adapterList.size(); ++j)
	{
		adapter = adapterList[j];
		//adapter->EnumOutputs : adapter ����
		while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc; // List �ҷ���
			output->GetDesc(&desc); // List �Է�
			wstring text = desc.DeviceName; // ���
			std::wcout << text << endl;
			++i;
		}
	}
	
}
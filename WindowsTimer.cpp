#include <Windows.h>
#include <iostream>

using namespace std;

void TimerFunction()
{
	//Timer ���ļ� ȹ��
	__int64 CountPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&CountPerSec); // �ʴ� ƽ ��
	double SecondPerCount = 1.0 / (double)CountPerSec; // ƽ�� �� ��

	// ���� ������ CPU Clock ��
	__int64 curTimeStart;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTimeStart);

	// ���� ������ CPU Clock ��
	__int64 curTimeEnd;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTimeEnd);

	// ���� �ð� ���
	float ResultTime = (curTimeEnd - curTimeStart) / (float)CountPerSec;
	cout << ResultTime << endl;
}

int main()
{
	TimerFunction();
	return 0;
}
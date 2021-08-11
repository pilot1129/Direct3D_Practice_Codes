#include <Windows.h>
#include <iostream>

using namespace std;

void TimerFunction()
{
	//Timer 주파수 획득
	__int64 CountPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&CountPerSec); // 초당 틱 수
	double SecondPerCount = 1.0 / (double)CountPerSec; // 틱당 초 수

	// 시작 시점의 CPU Clock 수
	__int64 curTimeStart;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTimeStart);

	// 종료 시점의 CPU Clock 수
	__int64 curTimeEnd;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTimeEnd);

	// 측정 시간 결과
	float ResultTime = (curTimeEnd - curTimeStart) / (float)CountPerSec;
	cout << ResultTime << endl;
}

int main()
{
	TimerFunction();
	return 0;
}
#include <Windows.h>
#include <iostream>

using namespace std;

class Timer
{
public :
	Timer();

	float Time() const;
	float ResultTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	double mSecondPerCount;
	double mResultTime;
	__int64 mBaseTime;
	__int64 mPauseTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurTime;

	bool mIsStopped;
};

Timer::Timer()
{
	mSecondPerCount = 0;
	mResultTime = -1;
	mBaseTime = 0;
	mPauseTime = 0;
	mStopTime = 0;
	mPrevTime = 0;
	mCurTime = 0;
	mIsStopped = false;

	__int64 CountPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&CountPerSec);
	mSecondPerCount = 1.0 / (double)CountPerSec;
}

float Timer::Time() const
{
	if (mIsStopped)
	{
		float res = mStopTime - mPauseTime - mBaseTime;
		res *= mSecondPerCount;
		return res;
	}
	else
	{
		float res = mCurTime - mPauseTime - mBaseTime;
		res *= mSecondPerCount;
		return res;
	}
}
float Timer::ResultTime() const
{
	return mResultTime;
}

void Timer::Reset()
{
	__int64 curTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTime);

	mBaseTime = curTime;
	mPrevTime = curTime;
	mPauseTime = 0;
	mStopTime = 0;
	mIsStopped = false;
	return;
}
void Timer::Start()
{
	// if문에 들어가고 작업을 수행하게 되면 if문 진입 시간 또한 포함되어 따로 뽑아냄
	__int64 curTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTime);

	if (mIsStopped)
	{
		mPauseTime += (curTime - mStopTime); // 일시정지한 시간 계산(Total 계산용)
		mPrevTime = curTime;

		mStopTime = 0;
		mIsStopped = false;
	}
	return;
}
void Timer::Stop()
{
	if (!mIsStopped)
	{
		__int64 curTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&curTime);

		mStopTime = curTime;;
		mIsStopped = true;
	}
	return;

}
void Timer::Tick()
{
	if (mIsStopped)
	{
		mResultTime = 0;
		return;
	}

	__int64 curTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&curTime);
	mCurTime = curTime;
	mResultTime = (mCurTime - mPrevTime) * mSecondPerCount;
	mPrevTime = mCurTime;

	if (mResultTime < 0)
		mResultTime = 0;

	return;
}

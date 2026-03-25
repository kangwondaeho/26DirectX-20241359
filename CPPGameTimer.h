#pragma once
#include <chrono> // C++ 표준 시간 라이브러리

class CPPGameTimer {
public:
    CPPGameTimer() {
        // 현재 시점으로 초기화
        prevTime = std::chrono::steady_clock::now();
    }

    // 매 프레임마다 호출하여 델타 타임을 반환함.
    float Update() {
        // 1. 현재 시점 기록
        auto currentTime = std::chrono::steady_clock::now();

        /*
         * [C++ 스타일 강의 포인트 2: Duration 계산]
         * - 두 시점(time_point)을 빼면 기간(duration)이 나옴.
         * - duration_cast를 통해 우리가 원하는 단위(초, 밀리초 등)로 변환함.
         */
        std::chrono::duration<float> frameTime = currentTime - prevTime;

        // 2. 계산된 시간 간격을 멤버 변수에 저장
        deltaTime = frameTime.count();

        // 3. 이전 시점 갱신
        prevTime = currentTime;

        return deltaTime;
    }

    float GetDeltaTime() const { return deltaTime; }

private:
    std::chrono::steady_clock::time_point prevTime;
    float deltaTime = 0.0f;
};

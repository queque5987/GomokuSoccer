# Gomoku Soccer

![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.2-black?logo=unrealengine)
![C++](https://img.shields.io/badge/C++-blue.svg?logo=c%2B%2B)
![Steamworks](https://img.shields.io/badge/Steamworks-OSS-171A21?logo=steam)

**Gomoku Soccer**
Steam 정식 출시를 목표로 개발 중인 1인 프로젝트입니다.

오목 축구는 오목과 알까기를 접목시킨 멀티플레이 기반 턴제/실시간 보드게임입니다.

---

# 사용한 기술

## **1. Steam OSS 기반 P2P 멀티플레이 구현**
* **Steamworks SDK** : 플레이어간 세션 생성, 검색, 참여 기능을 구현하였습니다.
* **RPC** : 각 돌들의 위치를 Replicate하고, 이펙트와 사운드 재생 그리고 스코어 업데이트 등의 기능을 구현하며 서버와 클라이언트를 동기화하였습니다.

## **2. 멀티스레딩 & 좌표 기반 Min-Max 알고리즘 구현**
* **기존 Min-Max 알고리즘 확장** : 기존 271개 비트를 사용하는 Min-Max 알고리즘을 확장하여 바둑판의 그리드 방식이 아닌 좌표계에서도 활용할 수 있는 알고리즘을 구현하였습니다.
* **멀티스레드** : 바둑돌의 위치를 구하는 기능이 백그라운드 스레드에서 동작하도록 구현하였습니다.

## **3. 머티리얼 기반 UI 구현**
* **MID를 통한 UI 연출** : 다이나믹 머티리얼 인스턴스의 파라미터를 C++ 및 Blueprint에서 조정하여 UI 애니메이션을 구현하였습니다.
* **Material UI** : 버튼 및 HUD 등 모든 위젯을 머티리얼을 활용하여 구현하였습니다.

## **4. 확장성을 고려한 객체 지향적 구조**
* **컴파일 시간 최소화** : 인터페이스의 의존성을 정적인 클래스에 몰아두어 각 클래스 간 함수 호출을 자유롭게 하고, 순환 참조를 방지, 컴파일 시간을 단축시켰습니다.

---


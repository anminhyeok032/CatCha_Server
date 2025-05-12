# CatCha Server Project

##  프로젝트 게임 플레이영상
[![Watch the Demo Video](https://img.youtube.com/vi/ojGJYacFAhw/0.jpg)](https://www.youtube.com/watch?v=ojGJYacFAhw&t=2s)

##  프로젝트 구조
```
CatCha_Server/
│
├── Character/
│   ├── AI/
│   │   ├── AIPlayer.cpp
│   │   └── AIPlayer.h
│   ├── CharacterState/
│   │   ├── CatPlayer.cpp
│   │   ├── CatPlayer.h
│   │   ├── CharacterState.h
│   │   ├── MousePlayer.cpp
│   │   └── MousePlayer.h
│   ├── Character.h
│   └── Player.cpp
├── Global/
│   ├── Map/
│   ├── global.h
│   └── protocol.h
├── Voxel/
│   ├── Octree.cpp
│   ├── Octree.h
│   └── VoxelPatternManager.h
├── 리소스 파일/
│   └── Map.txt
├── 소스 파일/
│   ├── GameSession.cpp
│   └── MainServer.cpp
├── 헤더 파일/
│   ├── GameSession.h
│   └── Over_IO.h
├── README.md
└── .gitignore
```

##  실행
다운로드된 폴더 내 `Catcha_Server.exe`를 직접 실행
```bash
Catcha_Server/Catcha_Server.exe
```

##  Movement & Controls (이동 및 조작)
1. **클라이언트 입력**: 키보드(이동) 및 마우스(회전) 입력 패킷 전송
2. **샘플링 제한**: 마우스 이벤트는 초당 20회로 고정
3. **서버 처리**:
   - Key Input → Velocity 계산 → 충돌 검사
   - 마우스 입력 → Rotation 계산
4. **결과 브로드캐스트**: 확정된 위치(position) 및 회전(rotation) 전송
5. **클라이언트 보간**: 선형 보간으로 자연스러운 움직임 및 회전

##  Collision Detection (OBB vs OBB)
- **Separating Axis Theorem (SAT)** 기반
- 검사 축: 2 OBB 로컬 축 6개 + 외적 축 9개 = 총 15축
- 투영 반경 계산 후 겹침(overlap) 여부 확인
- 최소 침투 축을 충돌 법선(normal)으로 선택

##  Sliding & Position Correction
1. **슬라이딩 벡터**: 기존 이동 벡터에서 부딪힌 OBB 면의 법선 성분 제거
2. **침투 깊이**: 최소 침투 축 및 깊이 계산 및 위치 보정
3. **최종 이동 적용**: 보정 위치에 슬라이딩 벡터 더해 이동

##  Voxel-Based Cheese (치즈 구현)
1. **Octree 분할**: 치즈 영역 AABB → 재귀 8분할 → 최종 리프 당 Voxel
3. **상호작용 시 삭제**:
   - BoundingSphere → AABB 옥트리 순차 검사
4. **동기화**: 삭제된 Voxel이 있다면 BoundingSphere Position 브로드캐스트 → 클라이언트에서 삭제

##  AI Pathfinding (AI 이동)
1. **타일맵 생성**: 기존 맵 데이터를 기반으로 타일맵 생성
2. **경로 탐색**: A* 알고리즘으로 목표 지점까지 최단 경로 계산 및 저장
3. **주기적 이동**: 0.2초 고정 주기로 한 타일씩 이동, 도착 시 경로 재계산
4. **동기화**: 서버 → 클라이언트로 타일 좌표 브로드캐스팅, 클라이언트는 선형 보간으로 부드럽게 렌더링

##  폴더 조직
- 소스코드는 `Catcha_Server/` 아래에 위치

---

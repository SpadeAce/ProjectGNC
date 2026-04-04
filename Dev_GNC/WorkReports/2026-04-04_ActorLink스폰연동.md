# ActorLink를 통한 CardGameActor 스폰 연동

- **일시**: 2026-04-04
- **분류**: 기능 구현 (스폰 시스템)
- **범위**: StageSubsystem.h/.cpp

---

## 배경

`FPawnDataRow`와 `FMonsterDataRow`에 `TSoftClassPtr<ACardGameActor> ActorLink` 필드가 이미 정의되어 있었으나, 실제 스폰 로직(`SpawnActorOnTile`)에서는 항상 `ACardGameActor::StaticClass()`를 사용하여 기본 클래스만 스폰되고 있었다. DataTable에서 지정한 Blueprint 서브클래스(예: BP_SlimeActor, BP_TurtleActor)가 스폰되도록 연결이 필요했다.

---

## 변경 내용

### 1. SpawnActorOnTile 시그니처 확장 (StageSubsystem.h)

```cpp
// 변경 전
ACardGameActor* SpawnActorOnTile(ASquareTile* Tile);

// 변경 후
ACardGameActor* SpawnActorOnTile(ASquareTile* Tile, TSubclassOf<ACardGameActor> ActorClass = nullptr);
```

- `ActorClass` 파라미터 추가, 기본값 `nullptr`로 하위 호환 유지
- null일 경우 내부에서 `ACardGameActor::StaticClass()` 폴백

### 2. SpawnPawns — ActorLink resolve (StageSubsystem.cpp)

- `UDPawn* Pawn` 참조를 스폰 호출 전으로 이동
- `Pawn->Data->ActorLink.LoadSynchronous()`로 액터 클래스 resolve
- resolve된 클래스를 `SpawnActorOnTile(Tile, ActorClass)`에 전달

### 3. SpawnMonsters — ActorLink resolve (StageSubsystem.cpp)

- `DataSub->GetMonsterData(MonsterId)`로 row 데이터를 스폰 전에 먼저 조회
- `MonsterData->ActorLink.LoadSynchronous()`로 액터 클래스 resolve
- `Monster->InitFromData(MonsterData)` 사용하여 중복 DataTable 조회 방지

---

## 폴백 전략

| 상황 | 동작 |
|------|------|
| ActorLink 비어있음 (`IsNull()`) | 기본 `ACardGameActor::StaticClass()` 스폰 |
| `LoadSynchronous()` 실패 (null 반환) | SpawnActorOnTile 내부 폴백으로 기본 클래스 스폰 |

---

## 검증 방법

1. 빌드 성공 확인 (완료)
2. DataTable 에디터에서 DT_MonsterData / DT_PawnData의 ActorLink에 BP 서브클래스 지정
3. 스테이지 로드 시 지정된 BP 액터가 스폰되는지 확인
4. ActorLink 비어있을 때 기존처럼 기본 ACardGameActor가 스폰되는지 확인

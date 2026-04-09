# CardGameActor 이동 버그 수정

- **일시**: 2026-04-09
- **분류**: 버그 수정 (이동 시스템)
- **범위**: CardGameActor.cpp, GridEntity.cpp

---

## 배경

CardGameActor에서 `MoveTo()`로 이동을 시도하면 회전만 부분적으로 작동하고 실제 위치 이동이 발생하지 않는 문제가 있었다. 회전 방향도 비정상적이었다.

## 원인 분석

### 버그 1: Tick 미등록으로 이동 불가

`CardGameActor` 생성자에서 `PrimaryActorTick.bCanEverTick = false`로 설정되어 있었다. 이 설정은 Tick 함수를 엔진 틱 매니저에 **등록하지 않겠다**는 의미로, 이후 `MoveTo()`에서 `SetActorTickEnabled(true)`를 호출해도 등록되지 않은 Tick은 활성화할 수 없었다. `Tick()` 내부의 `FMath::VInterpConstantTo` 보간 코드가 실행되지 않아 위치가 전혀 변하지 않았다.

### 버그 2: SetSlot의 회전 초기화

`GridEntity::SetSlot()`에서 `SetActorRotation(FRotator::ZeroRotator)`를 무조건 호출하고 있었다. 이동 완료 시 `FinalTile->Slot->SetEntity(this)` → `SetSlot()` 체인으로 `FaceTarget()`이 설정한 회전 방향이 0으로 리셋되었다.

## 변경 사항

### 1. CardGameActor.cpp — Tick 등록 수정

```cpp
// 변경 전
PrimaryActorTick.bCanEverTick = false;

// 변경 후
PrimaryActorTick.bCanEverTick = true;
PrimaryActorTick.bStartWithTickEnabled = false;
```

- Tick을 엔진에 등록하되, 스폰 시에는 비활성 상태로 시작
- `MoveTo()`의 `SetActorTickEnabled(true)`가 정상 작동

### 2. GridEntity.cpp — SetSlot 회전 리셋 제거

```cpp
// 변경 전
SetActorLocation(Slot->GetOwner()->GetActorLocation());
SetActorRotation(FRotator::ZeroRotator);

// 변경 후
SetActorLocation(Slot->GetOwner()->GetActorLocation());
```

- 이동 완료 후 캐릭터가 마지막 이동 방향을 유지
- 새로 스폰된 액터는 기본값이 ZeroRotator이므로 초기 배치에 영향 없음

## 추가 확인 필요

- `FaceTarget()`의 `Direction.Rotation()`은 +X를 전방으로 간주. 스켈레탈 메시의 전방 축이 +X가 아니면 MeshComp에 RelativeRotation 오프셋 적용 필요.

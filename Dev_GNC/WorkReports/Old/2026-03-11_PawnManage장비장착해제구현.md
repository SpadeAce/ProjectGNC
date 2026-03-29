# 작업 보고서: PawnManage 장비 장착/해제 구현

**날짜**: 2026-03-11
**작업자**: Claude

## 요청 내용

PawnManagePage에서 장비 슬롯(PawnManageEquipSlot) 인터랙션을 구현한다.
PawnManageItem 클릭으로 Pawn을 선택하고, PawnManageEquipItem 드래그/우클릭으로 장비를 장착하며, 슬롯 우클릭으로 해제하는 전체 흐름을 구현한다.

## 수행한 작업

- `DPawn`에 `Equip(DEquipment)`, `Unequip(DEquipment)` 메서드 추가
- `PawnManageItem`에 `IPointerClickHandler` 구현 — 좌클릭 시 `OnSelected` 이벤트 발행, `SetSelected(bool)` 메서드 추가
- `PawnManageEquipItem`에 드래그 인터페이스(`IPointerDownHandler`, `IDragHandler`, `IPointerUpHandler`) 및 우클릭(`IPointerClickHandler`) 구현 — `OnDragBegin/OnDragging/OnDragEnd/OnRightClick` 이벤트 추가
- `PawnManageEquipSlot`에 슬롯 상태 관리(`SetEquip`, `Clear`, `IsEmpty`, `CurrentEquip`) 및 우클릭 해제(`OnRightClick` 이벤트) 구현
- `PawnManagePage`에 Pawn 선택 관리, 드래그-드롭 장착, 우클릭 자동장착, 슬롯 우클릭 해제 전체 조율 로직 구현
- `05_캐릭터_시스템.md` 기획서 갱신 — 장비 시스템·PawnManager 구현 상태 반영
- `06_UI_시스템.md` 기획서 갱신 — PawnManagePage 및 EquipIcon 항목 추가

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Data/DataObject/DPawn.cs` | 수정 | `Equip`, `Unequip` 메서드 추가 |
| `Assets/Scripts/UI/PawnManage/PawnManageItem.cs` | 수정 | `_pawn` 저장, `OnSelected` 이벤트, `SetSelected(bool)`, `IPointerClickHandler` 구현 |
| `Assets/Scripts/UI/PawnManage/PawnManageEquipItem.cs` | 수정 | 드래그 이벤트(OnDragBegin/OnDragging/OnDragEnd) + `OnRightClick` 이벤트, `Equip` 프로퍼티 추가 |
| `Assets/Scripts/UI/PawnManage/PawnManageEquipSlot.cs` | 수정 | `SetEquip`, `Clear`, `IsEmpty`, `CurrentEquip`, `SlotType`, `OnRightClick` 이벤트, `IPointerClickHandler` 구현 |
| `Assets/Scripts/UI/PawnManage/PawnManagePage.cs` | 수정 | 선택 관리(`_selectedPawnItem`), `RefreshEquipSlots`, `OnEquipItemDragEnd`, `OnEquipItemRightClick`, `OnSlotRightClick`, `EquipToSlot` 구현 |
| `WorkReports/Design/05_캐릭터_시스템.md` | 수정 | 장비 시스템·PawnManager 구현 상태 갱신, 체크리스트 업데이트 |
| `WorkReports/Design/06_UI_시스템.md` | 수정 | PawnManagePage 항목 및 EquipIcon 추가 |

## 주요 결정 사항

- **DeckPawnItem/DeckPawnSlot 패턴 채택**: 드래그 이벤트 구조(OnDragBegin/OnDragging/OnDragEnd)와 슬롯 상태 관리(_goEmpty, _spawnIcon 토글) 모두 기존 Deck 편성 패턴을 그대로 따름
- **드롭 감지에 RectTransformUtility.RectangleContainsScreenPoint 사용**: IDropHandler 대신 DragEnd 시점에 슬롯 위치를 순회 검사하는 방식 채택 (기존 패턴과 일관성 유지)
- **Tool 슬롯 다중 지원**: Tool_1/Tool_2 두 슬롯이 동일 `EquipSlotType.Tool`을 가지므로, `RefreshEquipSlots`에서 타입별 카운터를 두어 순서대로 할당
- **DPawn._equips는 슬롯 위치 정보 없이 단순 목록 유지**: 슬롯 위치(어느 슬롯에 장착됐는지)는 `PawnManageEquipSlot._equip`이 담당, DPawn은 장착 여부만 관리

## 참고 사항

- 장비 스탯 적용 로직 미구현 — DPawn에 장비가 장착돼도 HP/Attack 등 스탯에 반영되지 않음 (향후 구현 필요)
- `PawnManageEquipSlot`의 `_goLock`은 현재 미사용 (잠금 슬롯 기능 미구현)
- 동일 장비를 여러 Pawn에 중복 장착하는 것을 막는 검증 로직 없음

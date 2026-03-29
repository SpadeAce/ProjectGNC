# 작업 보고서: BattleResultPopup 카드 보상 구현

**날짜**: 2026-03-16
**작업자**: Claude

## 요청 내용

StageRewardDataTable을 프로젝트에 등록하고, BattleResultPopup에 스테이지 클리어 시 카드 보상 선택 기능을 구현한다. 골드/경험치 표시, 가중치 랜덤 카드 3장 선출, 1장 선택 후 공용 카드풀 추가까지의 전체 흐름을 완성한다.

## 수행한 작업

- StageRewardDataTable을 DataManager에 등록 (`/add-table` 스킬로 StageRewardTable.cs 생성, DataManager.cs 수정)
- StageRewardTable에 `GetByLevel(int level)` 메서드를 추가하여 레벨 기반 보상 데이터 조회 지원
- BattleResultItem에 Button Linker를 추가하여 카드 클릭 이벤트 처리 가능하게 함
- BattleResultPopup에 골드/경험치 텍스트 표시 로직 추가
- 승리 시 `_goRewardCard` 활성화, 패배 시 비활성화 처리 구현
- `PickWeightedCards()` 메서드로 cardId/cardProb 기반 가중치 랜덤 3장 선출 (중복 불가) 구현
- `OnClickRewardCard()` 메서드로 카드 선택/해제/전환 토글 로직 구현
- `OnClickContinue()`에서 선택된 카드를 `DeckManager.AddCard()`로 공용 카드풀에 추가하는 로직 구현
- 기획서 3개 갱신 (02_전투_시스템, 06_UI_시스템, 99_로드맵)

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Logic/Manager/TableData/StageRewardTable.cs` | 추가 → 수정 | /add-table로 생성 후 `GetByLevel(int level)` 메서드 추가 |
| `Assets/Scripts/Logic/Manager/DataManager.cs` | 수정 | StageReward 프로퍼티 및 LoadAll() 등록 추가 |
| `Assets/Scripts/UI/BattleResult/BattleResultItem.cs` | 수정 | Button Linker 추가 |
| `Assets/Scripts/UI/BattleResult/BattleResultPopup.cs` | 수정 | 골드/경험치 표시, 카드 보상 세팅, 선택 토글, 카드 획득 로직 전면 구현 |
| `WorkReports/Design/02_전투_시스템.md` | 수정 | 보상 흐름에 경험치/카드 보상 선택 흐름 추가 |
| `WorkReports/Design/06_UI_시스템.md` | 수정 | BattleResultPopup 상세 설명 갱신, 향후 작업 체크 |
| `WorkReports/Design/99_로드맵.md` | 수정 | "보상 선택 UI" 항목 완료 체크 |

## 주요 결정 사항

- **카드 중복 불가**: 3장 선출 시 동일 cardId가 중복 선출되지 않도록 선출 후 풀에서 제거하는 방식 채택
- **클릭 방식**: BattleResultItem에 Button Linker를 직접 추가하여 BattleResultPopup에서 onClick 등록
- **레벨 조회 직접 참조**: BattleResultParam에 stageLevel을 추가하지 않고, PreOpen()에서 `PlayerManager.Instance.DifficultyLevel`을 직접 참조하여 param 수정 최소화
- **선택 토글**: 같은 카드 재클릭 시 선택 해제, 다른 카드 클릭 시 전환. 미선택 상태로 Continue 가능 (카드 보상 없이 진행)

## 참고 사항

- BattleResultItem 프리팹에 Button 컴포넌트가 루트에 있어야 Linker가 정상 동작함. 없으면 프리팹에 Button 추가 필요
- StageRewardData에 해당 레벨 데이터가 없거나 cardId가 비어있으면 카드 보상 UI가 자동으로 비활성화됨
- cardId 종류가 3개 미만인 경우 가능한 수만큼만 카드가 표시되고 나머지 BattleResultItem은 비활성화됨

# 작업 보고서: 난이도 레벨 & PlayerManager

**날짜**: 2026-03-15
**작업자**: Claude

## 요청 내용

스테이지 클리어 횟수에 따라 난이도 레벨이 증가하고, 해당 레벨에 맞는 TileMapPreset을 `StagePresetDataTable`에서 조회해 랜덤으로 선택하는 시스템을 구현. 플레이어 상태(난이도 레벨, Gold) 관리를 위한 `PlayerManager`를 신설하고 기존 `DeckManager`의 Gold 로직을 이관.

## 수행한 작업

- `PlayerManager` 클래스 신설 (`MonoSingleton<PlayerManager>`) — DifficultyLevel, Gold 보유
- `StagePresetTable`에 `GetByLevel(int level)` 메서드 추가 (Level 필드 기반 조회)
- `StageScript.ResolveStagePreset()` 메서드 추가 — 현재 난이도 레벨의 프리셋 데이터를 조회해 PresetPath 중 랜덤 선택 후 Resources.Load, 실패 시 Inspector 폴백
- `BattleResultPopup.OnClickContinue()`에서 씬 전환 전 `PlayerManager.Instance.IncrementLevel()` 호출
- `DeckManager`에서 Gold 관련 필드/메서드 제거 (`_gold`, `Gold`, `SpendGold()`, `AddGold()`)
- Gold 참조 4개 파일을 `PlayerManager.Instance`로 교체 (RecruitPage, ShopPage, RecruitItem, ShopItem)
- `StagePresetDataTable` 등록 (`/add-table StagePresetDataTable` 커맨드 수행)
- `README.md`에 기획서 링크 섹션 추가
- 로드맵(`99_로드맵.md`)에 난이도 스케일링 섹션 추가 및 몬스터 능력치 스케일링 미구현 항목 기입

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Logic/Manager/PlayerManager.cs` | 추가 | DifficultyLevel·Gold 관리 MonoSingleton |
| `Assets/Scripts/Logic/Manager/TableData/StagePresetTable.cs` | 수정 | GetByLevel() 메서드 추가 |
| `Assets/Scripts/Logic/Manager/DataManager.cs` | 수정 | StagePreset 프로퍼티 및 LoadAll() 등록 |
| `Assets/Scripts/Logic/Manager/DeckManager.cs` | 수정 | Gold 관련 필드·메서드 제거 |
| `Assets/Scripts/Logic/Scene/StageScript.cs` | 수정 | ResolveStagePreset() 추가, LoadStage 인자 변경 |
| `Assets/Scripts/UI/BattleResult/BattleResultPopup.cs` | 수정 | OnClickContinue()에서 IncrementLevel() 호출 |
| `Assets/Scripts/UI/Recruit/RecruitPage.cs` | 수정 | Gold 참조 → PlayerManager |
| `Assets/Scripts/UI/Shop/ShopPage.cs` | 수정 | Gold 참조 → PlayerManager |
| `Assets/Scripts/UI/Recruit/RecruitItem.cs` | 수정 | SpendGold 참조 → PlayerManager |
| `Assets/Scripts/UI/Shop/ShopItem.cs` | 수정 | SpendGold 참조 → PlayerManager |
| `README.md` | 수정 | 기획서 링크 섹션 추가 |
| `WorkReports/Design/99_로드맵.md` | 수정 | 난이도 스케일링 섹션 추가 |

## 주요 결정 사항

- **PlayerManager를 MonoSingleton으로 신설**: DontDestroyOnLoad로 씬 간 레벨/Gold 상태 유지가 목적. PawnManager나 DeckManager에 합치지 않고 별도 분리해 책임 명확화.
- **StagePresetData.PresetPath는 RepeatedField**: 단일 string이 아닌 복수 경로를 담는 리스트이므로 `Random.Range`로 인덱스 선택. 향후 맵 풀 확장 시 데이터만 추가하면 됨.
- **Inspector의 `_stagePreset` 폴백 보존**: 테이블에 해당 레벨 데이터가 없거나 Resources.Load 실패 시 기존 Inspector 값 사용. 개발·디버그 편의성 유지.
- **IncrementLevel 위치를 BattleResultPopup.OnClickContinue()로**: 승리 시에만 활성화되는 버튼이므로 별도 조건 분기 불필요. 씬 전환 직전에 호출해 다음 스테이지 진입 시 즉시 반영.

## 참고 사항

- 몬스터 능력치 난이도 스케일링은 이번 작업 범위 외. 로드맵에 기입.
- `StagePresetData`의 `Level` 필드와 `Id` 필드는 별개이므로 `GetByLevel()`은 `_map.Values`를 순회하여 매칭. 데이터 행 수가 많지 않으므로 성능 문제 없음.
- Gold 획득 경로(스테이지 클리어 보상 등)는 미구현 상태이며 `PlayerManager.AddGold()`로 연동 가능.

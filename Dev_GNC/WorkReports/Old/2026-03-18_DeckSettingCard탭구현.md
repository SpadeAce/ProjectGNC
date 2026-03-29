# 작업 보고서: DeckSetting Card 탭 구현

**날짜**: 2026-03-18
**작업자**: Claude

## 요청 내용

DeckSettingPage의 Card 탭에 보유 중인 공용 카드 리스트를 표시하는 기능을 구현하도록 요청받았다. 수직 스크롤, 한 줄에 5장 표시, DeckCardItem의 _spawnIcon에 세팅된 CardIcon을 활용하는 조건이 주어졌다.

## 수행한 작업

- `DeckManager`에 `DeckCards` 읽기 전용 접근자(`IReadOnlyList<DCard>`)를 추가하여 보유 카드 목록을 외부에서 조회할 수 있도록 했다.
- `DeckCardItem`에 `SetData(DCard)` 메서드를 추가하여 `_spawnIcon` 내부의 `CardIcon`에 카드 데이터를 세팅하도록 구현했다.
- `DeckSetting_Card`에 `OnOpened()` → `RefreshCardList()` 로직을 구현했다. PawnManagePage의 장비 그리드 패턴과 동일하게 5열 수동 RectTransform 배치 + Content 높이 동적 조정 방식을 사용했다.
- `DeckSettingPage`의 탭 전환 메서드(`OnClickPawnTab`, `OnClickCardTab`)에 각각 `_pawn.OnOpened()`, `_card.OnOpened()` 호출을 추가하여 탭 전환 시 데이터가 갱신되도록 했다.
- 덱편성 시스템 기획서(`10_덱편성_시스템.md`)에 Card 탭 관련 API, 흐름, DeckCardItem 섹션을 추가하고 구현 상태를 갱신했다.
- 로드맵(`99_로드맵.md`)의 덱 편집 UI 항목에서 Card 탭 기본 구현을 완료 처리했다.

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Logic/Manager/DeckManager.cs` | 수정 | `DeckCards` IReadOnlyList 접근자 추가 |
| `Assets/Scripts/UI/DeckSetting/DeckCardItem.cs` | 수정 | `SetData(DCard)` 메서드 추가, CardIcon 데이터 세팅 |
| `Assets/Scripts/UI/DeckSetting/DeckSetting_Card.cs` | 수정 | `OnOpened()`, `RefreshCardList()` 구현 (5열 그리드 배치) |
| `Assets/Scripts/UI/DeckSetting/DeckSettingPage.cs` | 수정 | 탭 전환 시 `OnOpened()` 호출 추가 |
| `WorkReports/Design/10_덱편성_시스템.md` | 수정 | Card 탭 API/흐름/DeckCardItem 섹션 추가, 구현 상태 갱신 |
| `WorkReports/Design/99_로드맵.md` | 수정 | Card 탭 기본 구현 완료 처리 |

## 주요 결정 사항

- **PawnManagePage 장비 그리드 패턴 재사용**: GridLayoutGroup 대신 수동 RectTransform 배치를 사용했다. 프로젝트 전반에서 동일한 패턴을 사용하고 있어 일관성을 유지했다.
- **표시 전용 구현**: 현재 DeckCardItem은 카드 데이터 표시만 담당하며, 드래그/클릭 등 상호작용은 포함하지 않았다. 덱 편집(추가/제거) 기능은 후속 작업으로 분리했다.

## 참고 사항

- `itemH = 120f` 값은 실제 CardIcon 프리팹 크기에 따라 조정이 필요할 수 있다.
- 후속 작업: 카드 추가/제거 기능, 카드 상세 정보 팝업 구현이 필요하다.

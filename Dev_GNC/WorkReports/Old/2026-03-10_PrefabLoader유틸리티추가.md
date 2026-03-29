# 작업 보고서: PrefabLoader 유틸리티 추가

**날짜**: 2026-03-10
**작업자**: Claude

## 요청 내용

`MonoBehaviourEx`에 프리팹 로드 기능을 추가하는 방안을 검토하다가, 책임 분리 관점에서 별도 유틸리티 클래스(`PrefabLoader`)를 만드는 방향으로 결정. 코드베이스 4곳에서 반복되던 `GetCustomAttribute` + `Resources.Load` 패턴을 `PrefabLoader.Load<T>()`로 통일하고, reflection 결과를 캐싱하도록 구현.

## 수행한 작업

- `PrefabLoader.cs` 생성 — `AssetPathAttribute`에서 경로를 읽어 프리팹을 로드하는 static 유틸리티. `Dictionary<Type, string>`으로 reflection 결과 캐싱
- `UIManager.cs` 수정 — `OpenView<T>()` 내 2줄 패턴을 `PrefabLoader.Load<T>()`로 교체, 불필요한 `using System.Reflection` 제거
- `ShopPage.cs` 수정 — `RefreshItems()` 내 동일 패턴 교체, `using System.Reflection` 제거
- `RecruitPage.cs` 수정 — `RefreshItems()` 내 동일 패턴 교체, `using System.Reflection` 제거
- `DeckSetting_Pawn.cs` 수정 — `RefreshOwnPawns()` 내 동일 패턴 교체, `using System.Reflection` 제거

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Core/PrefabLoader.cs` | 추가 | AssetPath 기반 프리팹 로드 유틸리티, reflection 캐싱 포함 |
| `Assets/Scripts/Logic/Manager/UIManager.cs` | 수정 | `OpenView<T>()` 내 로드 패턴 → `PrefabLoader.Load<T>()` |
| `Assets/Scripts/UI/Shop/ShopPage.cs` | 수정 | `RefreshItems()` 내 로드 패턴 → `PrefabLoader.Load<ShopItem>()` |
| `Assets/Scripts/UI/Recruit/RecruitPage.cs` | 수정 | `RefreshItems()` 내 로드 패턴 → `PrefabLoader.Load<RecruitItem>()` |
| `Assets/Scripts/UI/DeckSetting/DeckSetting_Pawn.cs` | 수정 | `RefreshOwnPawns()` 내 로드 패턴 → `PrefabLoader.Load<DeckPawnItem>()` |

## 주요 결정 사항

- **MonoBehaviourEx 대신 별도 유틸리티 클래스 채택**: `ShopItem.GetPrefab()` 스타일은 C# static 상속 제약으로 구현 불가. `MonoBehaviourEx.LoadPrefab<T>()`와 `PrefabLoader.Load<T>()`는 호출 형태가 동일하므로, MonoBehaviour 확장과 Resources 로딩 책임을 분리하는 후자가 더 적합.
- **reflection 캐싱 추가**: `GetCustomAttribute<>`는 호출 빈도가 낮아 실질적 부담은 없으나, 로딩 로직을 중앙화하는 기회에 `Dictionary<Type, string>` 캐시를 함께 도입.

## 참고 사항

- 사용법: `PrefabLoader.Load<T>()` — `T`에 `[AssetPath("...")]` 어트리뷰트가 있어야 함
- `AssetPath`가 없는 타입을 넘기면 `null` 반환 (Resources.Load 동작과 동일)

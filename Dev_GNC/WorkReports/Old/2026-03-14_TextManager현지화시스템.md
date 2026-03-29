# 작업 보고서: TextManager 현지화 시스템

**날짜**: 2026-03-14
**작업자**: Claude

## 요청 내용

TextData 테이블을 현지화 텍스트 용도로 사용하기 위해, 기존 int Id 기반 접근 대신 string IdAlias 기반의 별도 텍스트 매니저 시스템을 구축해달라는 요청. 언어 선택은 enum으로, 옵션창에서 제어 가능하도록 설계.

## 수행한 작업

- `TextDataTable` proto 생성 확인 후 `TextTable.cs` 초기 등록 (DataManager 경유)
- 설계 검토 후 별도 `TextManager` 싱글톤 방식으로 재설계
- `Language` enum (`Kor`, `Eng`, `Jpn`) 신규 생성
- `TableLoader`에 `TKey` 제네릭 오버로드 추가 (string 키 지원)
- `TextTable`을 `string IdAlias` 키 기반으로 수정
- `TextManager` 싱글톤 생성 (언어 설정 PlayerPrefs 연동, `Get(alias)` 제공)
- `DataManager`에서 TextTable 제거 (TextManager가 직접 소유)
- `BootScript`에 `TextManager.Instance.Initialize()` 호출 추가

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Logic/Language.cs` | 추가 | Language enum (Kor/Eng/Jpn) |
| `Assets/Scripts/Logic/Manager/TextManager.cs` | 추가 | 현지화 텍스트 매니저 싱글톤 |
| `Assets/Scripts/Logic/Manager/TableData/TextTable.cs` | 추가→수정 | int Id → string IdAlias 키로 변경 |
| `Assets/Scripts/Logic/Manager/TableData/TableLoader.cs` | 수정 | TKey 제네릭 오버로드 추가 |
| `Assets/Scripts/Logic/Manager/DataManager.cs` | 수정 | Text 프로퍼티 및 Load 블록 제거 |
| `Assets/Scripts/Logic/Scene/BootScript.cs` | 수정 | TextManager.Instance.Initialize() 추가 |

## 주요 결정 사항

- **TextManager를 Singleton\<T\> 상속으로**: MonoBehaviour 불필요하고, DataManager와 동일한 패턴 사용
- **DataManager에서 TextTable 분리**: TextManager가 TextTable을 직접 소유해 언어 로직을 캡슐화. DataManager 경유 시 언어 선택 조합이 번거로움
- **TableLoader 제네릭 오버로드**: 기존 `int` 오버로드는 내부적으로 제네릭 버전을 호출하도록 리팩토링, 하위 호환 유지
- **미등록 alias fallback**: `Get(alias)` 시 미등록 키면 alias 문자열 그대로 반환 (런타임 오류 방지)

## 참고 사항

- `OptionPopup.cs`에 언어 선택 UI 연결은 별도 작업 필요 (`TextManager.Instance.SetLanguage(Language.Eng)` 호출)
- PlayerPrefs 키: `"Language"` (int 저장, 기본값 `Language.Kor`)
- 사용법: `TextManager.Instance.Get("alias")` / `TextManager.Instance.SetLanguage(Language.Eng)`

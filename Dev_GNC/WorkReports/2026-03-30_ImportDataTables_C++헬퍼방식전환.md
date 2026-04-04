# ImportDataTables: C++ 헬퍼 기반 classref/asset 보존 방식 전환

- **일시**: 2026-03-30
- **분류**: 버그 수정 + 기능 개선 (데이터 임포트 파이프라인)
- **범위**: DataTableEditorHelper.h/.cpp (신규), Tools/ImportDataTables.py (재작성)

---

## 배경

이전 구현(`2026-03-29_코드젠_classref보존.md`)에서 `ImportDataTables.py`가 기존 DataTable의 classref/asset 값을 보존하도록 `get_data_table_row_from_name()` (CustomThunk) 기반 로직을 추가했으나, UE5 Python에서 CustomThunk가 안정적으로 동작하지 않아 에디터에서 수동 설정한 BP 클래스 레퍼런스가 매번 소실되는 문제가 지속됐다.

---

## 해결 방향

CustomThunk 대신 C++ UFUNCTION 헬퍼로 기존 DataTable을 JSON 추출 → Python에서 병합 → 재임포트하는 방식으로 전환.

---

## 변경 내용

### 1. Source/Dev_GNC/CardGame/Data/DataTableEditorHelper.h/.cpp (신규)

`UBlueprintFunctionLibrary` 기반 헬퍼 클래스. `UDataTable::GetTableAsJSON()`은 UFUNCTION이 아니라 Python에서 직접 호출이 불가능하므로, 이를 UFUNCTION으로 래핑한다.

```cpp
UFUNCTION(BlueprintCallable, Category = "DataTable|Editor")
static FString GetDataTableAsJSON(UDataTable* DataTable);
```

- `Dev_GNC/CardGame/Data` 경로는 이미 Build.cs의 PublicIncludePaths에 등록됨
- `Engine` 모듈도 이미 의존성에 포함 — 추가 설정 불필요

### 2. Tools/ImportDataTables.py (재작성)

**제거한 함수:**
- `_read_existing_ref_values()` — CustomThunk 기반, 동작하지 않음
- `_merge_ref_values()` — 위 함수에 의존

**추가한 함수:**

| 함수 | 역할 |
|------|------|
| `_export_existing_json(data_table)` | C++ 헬퍼로 기존 DataTable JSON 추출 |
| `_merge_with_existing(new_json, existing_json, ref_fields)` | 기존 JSON과 새 JSON 병합 |
| `_is_valid_ref_value(val)` | `/`로 시작하는 유효한 UE5 에셋 경로 판별 |

**임포트 흐름:**

```
uasset 없음?
  → DataTable 생성 → fill_data_table_from_json_string → 저장

uasset 있음?
  → C++ 헬퍼로 기존 DataTable JSON 추출
  → 기존 JSON + 새 JSON 병합
     • 일반 필드: 새 JSON 값 사용
     • classref/asset 필드: 기존 값이 유효한 UE5 경로면 유지
  → fill_data_table_from_json_string(병합 JSON) → 저장
```

**필드명 케이싱 처리:**
- 새 JSON: camelCase (`prefabPath`)
- `GetTableAsJSON` 출력: PascalCase (`PrefabPath`)
- 병합 시 lowercase 키로 매칭하여 케이스 차이 해소

---

## 검증 절차

1. UE5 에디터에서 프로젝트 컴파일 (라이브 코딩 또는 빌드)
2. Output Log에서 C++ 헬퍼 테스트:
   ```
   py print(unreal.DataTableEditorHelper.get_data_table_as_json(unreal.EditorAssetLibrary.load_asset("/Game/CardGame/DataTables/DT_MonsterData")))
   ```
3. MonsterData의 PrefabPath에 BP 클래스 설정 → `ImportDataTables.py` 실행 → 값 유지 확인
4. ref 필드 없는 테이블(CardData 등)이 정상 동작하는지 확인

---

## 참고

- `GetTableAsJSON`은 Row Struct의 UPROPERTY 정의에 따라 `TSoftClassPtr`/`TSoftObjectPtr` 값을 `/Script/...` 또는 `/Game/...` 형식의 에셋 경로로 직렬화한다
- 보존 JSON은 파일에도 역기록되어 다음 파이프라인 실행 시에도 유지

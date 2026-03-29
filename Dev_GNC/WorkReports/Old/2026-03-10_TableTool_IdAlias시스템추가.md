# 작업 보고서: Table Tool IdAlias 시스템 추가

**날짜**: 2026-03-10
**작업자**: Claude

## 요청 내용

CardGame Table Tool에 Protobuf Alias 시스템을 도입해달라는 요청. 테이블간 참조 시 숫자 ID 대신 사람이 읽기 쉬운 alias 문자열로 입력하면 직렬화 시 자동으로 ID로 변환되는 기능을 추가한다.

## 수행한 작업

- 네이밍 컨벤션 기반 설계 수립: `idAlias`(alias 선언), `{X}IdAlias`(alias 참조 입력) 패턴 정의
- `proto_generator.py`에 `_is_ref_alias_field()` 함수 추가 및 `{X}IdAlias` 필드를 proto 생성에서 제외하는 필터 적용
- `data_serializer.py`에 alias 룩업 빌드 함수(`_build_alias_lookup`), 대상 테이블 탐색 함수(`_find_target_table`), alias 해석 함수(`_resolve_id_alias_fields`) 추가
- `serialize_all()`에 직렬화 전 alias_lookup 선행 빌드 및 `{X}IdAlias` 필드 직렬화 제외 처리 추가
- `validator.py`에 `idAlias` 중복값 검증 함수(`_validate_id_alias_uniqueness`) 추가
- `validator.py` 스키마 검증에 `{X}IdAlias` 선언 시 대응 `{X}Id` 필드 존재 여부 검증 추가

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `CardGameTable/Tool/proto_generator.py` | 수정 | `_is_ref_alias_field()` 추가, `{X}IdAlias` 필드를 proto 생성 시 제외 |
| `CardGameTable/Tool/data_serializer.py` | 수정 | `_build_alias_lookup`, `_find_target_table`, `_resolve_id_alias_fields` 추가, `serialize_all` 수정 |
| `CardGameTable/Tool/validator.py` | 수정 | `_validate_id_alias_uniqueness` 추가, 스키마 검증에 pair 검사 추가 |

## 주요 결정 사항

- **FieldType 추가 대신 네이밍 컨벤션 채택**: 새 FieldType(`alias`, `ref:MessageName`)을 도입하는 대신 필드명 패턴(`idAlias`, `{X}IdAlias`)을 기준으로 동작하도록 결정. Excel 스키마 변경 최소화, 기존 파이프라인과의 호환성 유지.
- **`idAlias`는 proto 포함, `{X}IdAlias`는 proto 제외**: 테이블 자체의 `idAlias`는 런타임 표시명으로 활용 가능하므로 binary에 포함. 참조용 `{X}IdAlias`는 툴 전용 입력 helper이므로 제외하여 binary 크기 절감.
- **`{X}Id`가 이미 있으면 alias 무시**: alias와 직접 ID 입력이 공존할 때 ID 우선 처리로 하위 호환 보장.
- **숫자 문자열도 ID로 직접 처리**: `{X}IdAlias` 컬럼에 숫자를 입력해도 그대로 ID로 해석, 점진적 마이그레이션 가능.
- **대상 테이블 탐색**: prefix 기반 대소문자 무관 prefix 매칭으로 `pawnIdAlias` → `PawnData` 자동 연결.

## 참고 사항

**사용 방법 요약**:
1. 참조되는 테이블 스키마에 `idAlias` (string) 필드 추가, 데이터에 alias 값 입력
2. 참조하는 테이블 스키마에 `{X}Id` (int32) + `{X}IdAlias` (string) 쌍으로 선언
3. 데이터 Excel의 `{X}IdAlias` 컬럼에 alias 문자열 입력
4. `RunTool.bat` 실행 → 자동으로 alias → ID 해석 후 `.bytes` 직렬화

**주의**: `{X}IdAlias` 필드는 proto/binary에 포함되지 않으므로, 런타임 C# 코드에서는 이 필드에 접근할 수 없다. 런타임에 alias 문자열이 필요한 경우 별도 `string` 필드를 선언해야 한다.

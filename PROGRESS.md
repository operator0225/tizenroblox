# TizenRoblox — 작업 진행 기록

## 목표
Samsung KQ83SF95AEXKR (OLED SF95 83", Tizen 9.0, ARM64) 에서 Roblox 네이티브 실행  
Samsung TV Virtual Gamepad + 실제 게임패드 필수 지원

---

## 전략 변경 (2026-07-03)

**당초 계획**: Android 런타임 shim을 처음부터 직접 구현  
**변경 후 전략**: 실제 Sober ARM64 Flatpak 바이너리를 Tizen에 맞게 적응

사용자가 업로드한 실제 Sober ARM64 Flatpak(`org.vinegarhq.Sober.flatpak`, aarch64/master, 2026-06-24 빌드)을 발견하면서 전략 전환.  
처음부터 구현하는 대신 진짜 Sober 바이너리 + 얇은 Tizen 호환 레이어를 사용한다.

---

## 아키텍처 개요

```
[Samsung KQ83SF95AEXKR TV]
        │
        ├── Tizen 9.0 / Wayland (Enlightenment compositor)
        │
        ├── tizen/launch.sh ─────────────── 메인 런처
        │       │
        │       ├── input_mapper ─────────── Samsung IR Remote → uinput virtual gamepad
        │       │       └── /dev/uinput → /dev/input/eventX (Sober가 읽음)
        │       │
        │       └── sober (ARM64) ─────────── Roblox Android ARM64 런타임
        │               ├── libloader.so  ── Android ELF 로더 (Sober 내장)
        │               ├── libbadcpu.so  ── CPU 검증 (Sober 내장)
        │               ├── libmimalloc.so.3 ── 메모리 할당자 (Sober 내장)
        │               └── [Tizen 호환 스텁]
        │                       ├── libsecret-1.so.0  ← GNOME keyring → 파일 기반
        │                       ├── libdecor-0.so.0   ← Wayland 데코레이션 (no-op)
        │                       └── libxml2.so.16     ← libxml2 3.x shim → 시스템 2.x
```

### Sober 의존성 분석
```
sober 바이너리 직접 의존성 (readelf -d):
  ✅ libloader.so        (Sober 내장)
  ✅ libmimalloc.so.3    (Sober 내장)
  ✅ libgcc_s.so.1       (Tizen 기본 제공)
  ✅ libc.so.6           (Tizen 기본 제공)
  ✅ libm.so.6           (Tizen 기본 제공)
  ✅ libz.so.1           (Tizen 기본 제공)
  ✅ libEGL.so.1         (Tizen TV GPU 드라이버)
  ✅ libGLESv2.so.2      (Tizen TV GPU 드라이버)
  ✅ libdbus-1.so.3      (Tizen 기본 제공)
  ⚠️ libcrypto.so.3      (Tizen에 OpenSSL 3.x 필요)
  ⚠️ libcurl.so.4        (Tizen 패키지 확인 필요)
  ⚠️ libfreetype.so.6    (Tizen 패키지 확인 필요)
  ⚠️ libfontconfig.so.1  (Tizen 패키지 확인 필요)
  ⚠️ libglib-2.0.so.0   (Tizen 패키지 확인 필요)
  ⚠️ libgobject-2.0.so.0 (GLib 패키지)
  ⚠️ libgstreamer-1.0.so.0  (GStreamer, Tizen에 있을 가능성 높음)
  ⚠️ libgstapp-1.0.so.0    (GStreamer app 플러그인)
  ⚠️ libgstvideo-1.0.so.0  (GStreamer video 플러그인)
  🔧 libsecret-1.so.0    (스텁 구현 완료)
  🔧 libdecor-0.so.0     (스텁 구현 완료)
  🔧 libxml2.so.16       (shim 구현 완료 → 시스템 libxml2.so.2 포워딩)
```

---

## 작업 로그

### Phase 1 — 프로젝트 셋업 ✅
- 날짜: 2026-06-18
- aarch64-linux-gnu 크로스컴파일 툴체인 설치
- CMakeLists.txt, 프로젝트 구조 생성
- toolchain/aarch64-tizen.cmake 작성

### Phase 2 — Sober Flatpak 분석 ✅
- 날짜: 2026-07-03
- org.vinegarhq.Sober.flatpak (aarch64/master, 2026-06-24) ostree 추출
- sober(7.6MB), sober_services(1.1MB), libloader.so(3.3MB), libbadcpu.so(323KB) 확인
- readelf로 전체 의존성 목록 분석
- sober_services (GTK4/WebKitGTK) → Tizen 비호환, 런처에서 제외

### Phase 3 — Tizen 호환 스텁 라이브러리 ✅
- 날짜: 2026-07-03

**stub/libsecret.c** — libsecret-1.so.0 스텁
- GNOME keyring 없는 Tizen용 파일 기반 자격증명 저장
- `$HOME/.tizenroblox_creds` 파일에 Roblox 로그인 정보 저장
- `secret_password_store_sync`, `secret_password_lookup_sync`, `secret_password_clear_sync` 구현

**stub/libdecor.c** — libdecor-0.so.0 스텁
- Tizen TV 전체화면 환경에서 Wayland 데코레이션 불필요
- `libdecor_configuration_get_content_size` → 3840×2160 반환
- `libdecor_configuration_get_window_state` → `LIBDECOR_WINDOW_STATE_FULLSCREEN`

**stub/libxml2_compat.c** — libxml2.so.16 호환 shim
- GNOME Platform 50이 libxml2 3.x (SONAME: libxml2.so.16) 사용
- Tizen TV에는 libxml2 2.x (SONAME: libxml2.so.2) 존재
- dlopen으로 시스템 libxml2.so.2 로드 후 9개 심볼 포워딩
  - xmlParseFile, xmlDocGetRootElement, xmlFreeDoc, xmlStrcmp
  - xmlGetProp, xmlNodeSetContent, xmlSaveFormatFileEnc, xmlCleanupParser, xmlFree

**빌드 확인**: aarch64 크로스컴파일 성공 (경고만, 오류 없음)

### Phase 4 — Samsung TV Input Mapper ✅
- 날짜: 2026-07-03

**tizen/input_mapper.c** — IR Remote → uinput 가상 게임패드
- `/dev/uinput` 가상 게임패드 생성 (Samsung vendor ID: 0x04e8)
- Samsung TV 리모컨 탐지: 디바이스 이름("Samsung", "TV Remote", "RC", "CEC") 또는 KEY_RED 기능
- 키 매핑 테이블:
  ```
  방향키 → ABS_HAT0X/Y (D-Pad)
  확인   → BTN_SOUTH (A버튼)
  뒤로   → BTN_EAST  (B버튼)
  빨강   → BTN_WEST  (X버튼)
  초록   → BTN_NORTH (Y버튼)
  노랑   → BTN_TL    (LB)
  파랑   → BTN_TR    (RB)
  재생   → BTN_START
  홈     → BTN_SELECT
  ```
- 전체 아날로그 축: ABS_X/Y, ABS_RX/RY (스틱), ABS_Z/RZ (트리거), ABS_HAT0X/Y (D패드)
- 물리적 게임패드 자동 탐지 (BTN_SOUTH 기능으로 구분)
- epoll 기반 메인 루프, 10초마다 새 게임패드 탐지

### Phase 5 — Tizen 런처 ✅
- 날짜: 2026-07-03

**tizen/launch.sh** — 통합 런처 스크립트
- Tizen Wayland 소켓 자동 탐지 (`/run/display/wayland-0`, `/tmp/.RTE/wayland-0`)
- LD_LIBRARY_PATH: 스텁 라이브러리 → sober 내장 → 시스템
- libxml2.so.16 symlink 자동 생성 (시스템에 있는 경우)
- input_mapper를 백그라운드 실행 후 sober 직접 호출
- sober_services (GTK4 UI) 우회
- 종료 시 자동 정리 (cleanup trap)

### Phase 6 — 빌드 시스템 완성 ✅
- 날짜: 2026-07-03

**CMakeLists.txt 업데이트**:
- `stub/`, `tizen/` 서브디렉토리 추가
- `add_subdirectory(stub)`, `add_subdirectory(tizen)`
- 출력 경로 수정: `dist/bin/`, `dist/lib/`

**toolchain/aarch64-tizen.cmake 수정**:
- `-mfpu=neon-fp-armv8` 제거 (32비트 ARM 전용 플래그, AArch64에서 오류)
- AArch64는 NEON/FP가 기본 내장

**scripts/build.sh** — 전체 빌드 스크립트
**scripts/deploy.sh** — TV SSH 배포 스크립트

**빌드 결과** (aarch64 크로스컴파일):
```
dist/
├── bin/
│   ├── sober           (6.7MB, ARM64 — Sober 실제 바이너리)
│   ├── input_mapper    (71KB,  ARM64 — 크로스컴파일 완료)
│   └── launch.sh       (7.6KB, 런처 스크립트)
└── lib/
    ├── libloader.so    (3.3MB — Sober Android ELF 로더)
    ├── libbadcpu.so    (323KB — CPU 검증)
    ├── libmimalloc.so.3 (216KB — 메모리 할당자)
    ├── libsecret-1.so.0 (70KB — GNOME keyring 스텁)
    ├── libdecor-0.so.0  (70KB — Wayland 데코레이션 스텁)
    └── libxml2.so.16   (70KB — libxml2 호환 shim)
```

---

## 남은 작업

### 단기 (TV 배포 전 필수)
- [ ] Tizen TV에서 누락 라이브러리 확인
  - libcrypto.so.3, libcurl.so.4, libglib-2.0.so.0, libgobject-2.0.so.0
  - libfreetype.so.6, libfontconfig.so.1
  - libgstreamer-1.0.so.0, libgstapp-1.0.so.0, libgstvideo-1.0.so.0
- [ ] 누락 라이브러리 스텁 또는 패키지 번들링
- [ ] TV SSH 접속 후 `scripts/deploy.sh <TV_IP>` 실행
- [ ] Wayland EGL 초기화 확인 (TV GPU 드라이버 경로)
- [ ] /dev/uinput 권한 확인 (root 또는 input 그룹 필요)

### 중기
- [ ] Roblox 로그인 처리 (libsecret 파일 스텁으로 자격증명 저장)
- [ ] GStreamer 비디오 파이프라인 테스트 (인게임 영상 재생)
- [ ] 성능 프로파일링 (NQ4 AI Gen3 SoC, 4GB RAM 기준)
- [ ] HDR / 4K 출력 설정

### 장기
- [ ] Tizen 앱 패키지화 (.tpk) — TV 런처에서 직접 실행
- [ ] 자동 업데이트 (Sober 새 버전 배포)
- [ ] 멀티플레이어 네트워크 (Tizen 방화벽 설정)

---

## 빌드 방법

```bash
# 크로스컴파일 (개발 PC에서)
bash scripts/build.sh

# Tizen TV 배포
bash scripts/deploy.sh <TV_IP>

# TV에서 직접 실행
ssh root@<TV_IP> /opt/tizenroblox/bin/launch.sh
```

## 참고
- Sober 소스: https://github.com/vinegarhq/sober
- Sober ARM64 이슈: https://github.com/vinegarhq/sober/issues/1221
- Tizen Developer Docs: https://developer.tizen.org
- uinput 커널 인터페이스: /usr/include/linux/uinput.h

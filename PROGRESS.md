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

### Phase 6 — 전체 의존성 커버리지 ✅
- 날짜: 2026-07-03

sober 바이너리의 모든 의존성에 대한 호환 shim 추가:

| 라이브러리 | SONAME | 방법 | 상태 |
|---|---|---|---|
| libsecret-1 | .so.0 | 파일 기반 스텁 | ✅ |
| libdecor-0 | .so.0 | 전체화면 no-op | ✅ |
| libxml2 | .so.16 | dlopen → libxml2.so.2 | ✅ |
| libcrypto | .so.3 | dlopen → OpenSSL 3.x 또는 1.1 | ✅ |
| libgstreamer-1.0 | .so.0 | dlopen → 시스템 GST | ✅ |
| libgstapp-1.0 | .so.0 | 동일 shim | ✅ |
| libgstvideo-1.0 | .so.0 | 동일 shim | ✅ |
| libglib-2.0 | .so.0 | dlopen → 시스템 GLib (4심볼) | ✅ |
| libgobject-2.0 | .so.0 | 동일 shim | ✅ |
| libfontconfig | .so.1 | dlopen → 시스템 FC | ✅ |
| libfreetype | .so.6 | dlopen → 시스템 FT | ✅ |
| libcurl | .so.4 | dlopen → 시스템 curl | ✅ |

### Phase 7 — 빌드 시스템 완성 ✅
- 날짜: 2026-07-03

**CMakeLists.txt 업데이트**:
- `stub/`, `tizen/` 서브디렉토리 추가
- `add_subdirectory(stub)`, `add_subdirectory(tizen)`
- 출력 경로 수정: `dist/bin/`, `dist/lib/`

**toolchain/aarch64-tizen.cmake 수정**:
- `-mfpu=neon-fp-armv8` 제거 (32비트 ARM 전용 플래그, AArch64에서 오류)
- AArch64는 NEON/FP가 기본 내장

**scripts/build.sh** — 전체 빌드 스크립트 (cross-compile + assemble)
**scripts/deploy.sh** — TV SSH 배포 스크립트
**scripts/deploy_sdb.sh** — TV SDB 배포 스크립트 (Tizen SDK 필요)
**scripts/setup.sh** — 자격증명 설정 도우미 (GNOME keyring import / 수동 입력)
**scripts/extract_sober.sh** — Flatpak에서 Sober 바이너리 추출
**scripts/package_tpk.sh** — Tizen .tpk 패키지 생성
**tizen/pkg/tizen-manifest.xml** — Tizen TV 앱 매니페스트

**빌드 결과** (aarch64 크로스컴파일, 오류 없음):
```
dist/bin/
  sober           (6.7MB — Sober ARM64 실제 바이너리)
  input_mapper    (71KB  — Samsung Remote → 가상 게임패드)
  launch.sh       (런처)

dist/lib/
  libloader.so         (3.3MB — Sober Android ELF 로더)
  libbadcpu.so         (323KB — CPU 검증)
  libmimalloc.so.3     (216KB — 메모리 할당자)
  libsecret-1.so.0     (스텁)
  libdecor-0.so.0      (스텁)
  libxml2.so.16        (shim)
  libcrypto.so.3       (shim)
  libgstreamer-1.0.so.0 (shim)
  libgstapp-1.0.so.0   (shim)
  libgstvideo-1.0.so.0 (shim)
  libglib-2.0.so.0     (shim)
  libgobject-2.0.so.0  (shim)
  libfontconfig.so.1   (shim)
  libfreetype.so.6     (shim)
  libcurl.so.4         (shim)
```

### Phase 8 — SDL2 Wayland 커버리지 + 성능 최적화 + 진단 도구 ✅
- 날짜: 2026-07-03

**SDL2 Wayland 라이브러리 분석**:
- sober 바이너리에서 SDL2 feature probe JSON 발견
- SDL2가 dlopen으로 탐지하는 선택적 라이브러리 목록 완전 파악:
  - Wayland 표시: libwayland-client.so.0, libwayland-egl.so.1, libwayland-cursor.so.0
  - 키보드: libxkbcommon.so.0
  - 오디오: libasound.so.2 (ALSA), libpulse.so.0 (PulseAudio)
  - 입력 핫플러그: libudev.so.1
  - GPU: libEGL.so.1, libGLESv2.so.2 (Tizen TV 드라이버 제공)

**tizen/launch.sh 개선**:
- `try_symlink_lib()` 함수 추가: 시스템 경로에서 라이브러리 탐색 후 lib/에 심볼릭 링크 생성
- Wayland 클라이언트 libs 자동 심볼릭 링크 (SDL2 화면 출력 필수)
- 오디오 libs 자동 심볼릭 링크 (ALSA/PulseAudio)
- GL 셰이더 캐시 경로 설정 (`~/.cache/sober/gl_shaders`)
- CPU 성능 거버너 설정 시도 (`/sys/devices/system/cpu/*/cpufreq/scaling_governor`)
- `SDL_VIDEODRIVER=wayland` 명시 설정

**stub/libwayland_stub.c** — Wayland passthrough 스텁:
- SDL2가 Wayland 백엔드를 활성화하려면 libwayland-client.so.0이 필요
- Tizen에 실제 libwayland가 있는 경우: 런처가 심볼릭 링크 생성 (이 스텁 사용 안 함)
- 없는 경우 최후 수단 폴백: wl_display_connect() → 내부에서 dlopen 재시도
- 15개 wl_display/registry/surface/proxy 함수 + wl_egl_window + wl_cursor 구현
- xdg_wm_base, xdg_surface, xdg_toplevel 인터페이스 객체 포함

**scripts/diagnose.sh** — 런타임 환경 진단:
- 아키텍처, 커널, /dev/uinput 권한 확인
- sober 바이너리 + bundled libs 존재 확인
- Wayland 소켓 탐지
- 모든 SDL2 선택적 라이브러리 (wayland, audio, xkb, udev) 확인
- GPU/EGL 확인
- 12개 TizenRoblox 스텁 라이브러리 확인
- 입력 장치 (삼성 리모컨, 게임패드) 탐지
- 자격증명 파일 확인
- PASS/WARN/FAIL 요약 출력

**OpenSSL 심볼 완성**:
- libcrypto3_compat.c: ASN1, ERR, OBJ, i2d_X509 36개 심볼 완전 구현
- `_GNU_SOURCE` 중복 정의 경고 수정 (`#ifndef` 가드)

**빌드 결과** (aarch64, 경고/오류 없음, 16개 타겟):
```
dist/lib/
  [이전 12개 스텁] +
  libwayland-client.so.0   (Wayland 폴백 스텁)
  libwayland-egl.so.1      (Wayland EGL 폴백 스텁)
  libwayland-cursor.so.0   (Wayland 커서 폴백 스텁)
```

---

## 남은 작업

### 단기 (TV 배포 전)
- [ ] Sober Flatpak에서 바이너리 추출: `bash scripts/extract_sober.sh <flatpak>`
- [ ] 빌드: `bash scripts/build.sh`
- [ ] TV 배포: `bash scripts/deploy.sh <TV_IP>`
- [ ] **진단 실행**: `ssh root@<TV_IP> bash /opt/tizenroblox/scripts/diagnose.sh`
- [ ] 자격증명 설정: `bash scripts/setup.sh` (진단 후 경고 있을 경우)
- [ ] TV에서 실행: `ssh root@<TV_IP> /opt/tizenroblox/bin/launch.sh`

### 중기 (기능 향상)
- [ ] 실제 TV에서 Wayland EGL 초기화 테스트
- [ ] 오디오 확인 (Tizen ALSA/PulseAudio 호환성)
- [ ] GStreamer 비디오 파이프라인 테스트 (인게임 영상)
- [ ] 성능 프로파일링 (NQ4 AI Gen3 SoC, 8GB RAM)
- [ ] HDR10+ / 4K 출력 최적화

### 장기 (완성도)
- [ ] Tizen .tpk 서명 + Samsung 스토어 배포
- [ ] 자동 Roblox 버전 업데이트
- [ ] 멀티플레이어 네트워크 테스트
- [ ] 게임패드 매핑 UI (Samsung OneRemote 설정)

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

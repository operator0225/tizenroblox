# TizenRoblox — 작업 진행 기록

## 목표
Samsung KQ83SF95AEXKR (OLED SF95 83", Tizen 9.0, ARM64) 에서 Roblox 네이티브 실행  
Samsung TV Virtual Gamepad + 실제 게임패드 필수 지원

## 아키텍처 개요

```
[Roblox Android ARM64 .so]
        ↓ dlopen
[tizenroblox loader]
  ├── libandroid_shim.so   ← ANativeActivity/Window/Looper/Input
  ├── liblog_shim.so       ← __android_log_print → syslog
  ├── libOpenSLES_shim.so  ← OpenSL ES → ALSA/PulseAudio
  ├── Fake JVM (JNI bridge)
  ├── EGL display (Tizen Wayland EGL)
  └── Input system (evdev + Samsung RC)
```

## 핵심 제약사항
- Sober 클로즈드소스 → 처음부터 자체 런타임 구현
- Roblox Hyperion 안티치트: Hyperion-free ARM64 빌드 필요 (Roblox 측 협력 필요)
- Tizen Wayland compositor 환경에서 EGL 초기화 필요

---

## 작업 로그

### Phase 1 — 프로젝트 셋업 ✅
- 날짜: 2026-06-18
- aarch64-linux-gnu 크로스컴파일 툴체인 설치
- 프로젝트 구조 생성
- CMakeLists.txt 작성

### Phase 2 — Android Shim 라이브러리
- [ ] libandroid_shim.so (ANativeActivity, ANativeWindow, ALooper, AInputQueue)
- [ ] liblog_shim.so (__android_log_print)
- [ ] libOpenSLES_shim.so (OpenSL ES → ALSA)
- [ ] cpu_features 스텁

### Phase 3 — JNI 환경
- [ ] Fake JavaVM 구현
- [ ] JNIEnv 230개 함수 포인터 구현
- [ ] Native method 등록 시스템

### Phase 4 — EGL / Display
- [ ] Tizen Wayland EGL 초기화
- [ ] EGL surface 생성
- [ ] OpenGL ES 3.0 컨텍스트

### Phase 5 — Input 시스템
- [ ] evdev 게임패드 (Linux /dev/input/event*)
- [ ] Samsung TV Virtual Gamepad (IR/CEC remote → gamepad 매핑)
- [ ] Roblox 입력 이벤트 브릿지

### Phase 6 — APK 로더
- [ ] APK(ZIP) 추출
- [ ] arm64-v8a .so 추출
- [ ] patchelf 라이브러리 의존성 패치

### Phase 7 — 통합 & 테스트
- [ ] 전체 파이프라인 연결
- [ ] Roblox JNI_OnLoad 호출
- [ ] 게임 루프 실행

### Phase 8 — Tizen 특화 최적화
- [ ] Tizen EGL surface 어댑터
- [ ] Samsung IR 리모컨 매핑
- [ ] 메모리 최적화 (3GB 이하 목표)

---

## 빌드 방법

```bash
# 크로스컴파일 (개발 PC에서)
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain/aarch64-tizen.cmake
make -j$(nproc)

# Tizen TV 배포
scp -r dist/ root@<TV_IP>:/opt/tizenroblox/
ssh root@<TV_IP> /opt/tizenroblox/run.sh
```

## 참고
- [vinegarhq/sober ARM64 이슈 #1221](https://github.com/vinegarhq/sober/issues/1221)
- [Android NDK 공식 문서](https://developer.android.com/ndk/guides)
- [JNI Tips](https://developer.android.com/training/articles/perf-jni)
- [Tizen Developer Docs](https://developer.tizen.org)

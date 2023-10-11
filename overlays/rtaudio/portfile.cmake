vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO thestk/rtaudio
        REF b4f04903312e0e0efffbe77655172e0f060dc085
        SHA512 54dbee377246446e6422448f51d5c1645eb63bb5af8041c5475084e5c9a337b098c4e23bbc153e7209f9c4995b95a71ea1c8c00558a19cfffd10b792a82769a7
        HEAD_REF master
)

string(COMPARE EQUAL "${VCPKG_CRT_LINKAGE}" "static" RTAUDIO_STATIC_MSVCRT)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
        FEATURES
        asio  RTAUDIO_API_ASIO
        alsa  RTAUDIO_API_ALSA
        pulse RTAUDIO_API_PULSE
)

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        OPTIONS
        -DRTAUDIO_STATIC_MSVCRT=${RTAUDIO_STATIC_MSVCRT}
        -DRTAUDIO_API_JACK=OFF
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

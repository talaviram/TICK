cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(TICK VERSION 0.6.2)
option(COPY_AFTER_BUILD "" ON)

if(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
        set(ICON "tick_icon_macOS.png")
        set(HARDENED_RUNTIME $<IF:$<CONFIG:Debug>,NO,YES>)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(APPLE_IOS TRUE)
        set(ICON "tick_icon_iOS.png")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum iOS deployment version")
        set(PLIST_ADDITION
            "
        <plist>
 <dict>
	<key>ABLLinkStartStopSyncSupported</key>
	<true/>
<key>UTExportedTypeDeclarations</key>
	<array>
		<dict>
			<key>UTTypeConformsTo</key>
			<array>
				<string>public.data</string>
			</array>
			<key>UTTypeIdentifier</key>
			<string>com.talaviram.Tick.preset</string>
			<key>UTTypeTagSpecification</key>
			<dict>
				<key>public.filename-extension</key>
				<string>tickpreset</string>
				<key>public.mime-type</key>
				<string>applications/tickpreset</string>
			</dict>
		</dict>
	</array>
</dict>
</plist>"
        )
    endif()

    set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)")
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM 7P7G2A8U54)
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(ICON "tick_icon.png")
endif()

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(IS_MOBILE YES)
endif()

if(IS_MOBILE)
    set(ICON "tick_icon_iOS.png")
endif()

set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL INTERNAL)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

add_subdirectory(${CMAKE_SOURCE_DIR}/libsamplerate)
add_subdirectory(${CMAKE_SOURCE_DIR}/JUCE)

set(PLUGIN_FORMATS_TO_BUILD Standalone)

if(NOT IS_MOBILE)
    list(APPEND PLUGIN_FORMATS_TO_BUILD VST3)

    if(EXISTS "${CMAKE_SOURCE_DIR}/private/VST2_SDK")
        message("VST2 In-place")
        juce_set_vst2_sdk_path(${CMAKE_SOURCE_DIR}/private/VST2_SDK)
        list(APPEND PLUGIN_FORMATS_TO_BUILD VST)
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/private/AAX_SDK" AND(APPLE OR MSVC))
        message("AAX In-place")
        juce_set_aax_sdk_path(${CMAKE_SOURCE_DIR}/private/AAX_SDK)
        list(APPEND PLUGIN_FORMATS_TO_BUILD AAX)
    endif()
endif()

if(APPLE)
    list(APPEND PLUGIN_FORMATS_TO_BUILD AU AUv3)
endif()

juce_add_plugin(TICK
    ICON_BIG "Media/Logo/${ICON}"
    BUNDLE_ID com.talaviram.Tick
    COMPANY_NAME "Tal Aviram"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD ${COPY_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE Tala
    PLUGIN_CODE Tick
    FORMATS ${PLUGIN_FORMATS_TO_BUILD}
    PRODUCT_NAME "TICK"
    DOCUMENT_EXTENSIONS tickpreset
    NEEDS_WEB_BROWSER FALSE
    HARDENED_RUNTIME_ENABLED ${HARDENED_RUNTIME}
    AAX_IDENTIFIER com.talaviram.Tick
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    AU_EXPORT_PREFIX TickAU
    AAX_CATEGORY AAX_ePlugInCategory_SWGenerators
    VST2_CATEGORY kPlugCategGenerator
    VST3_CATEGORIES Generator

    # iOS
    APP_GROUPS_ENABLED TRUE
    APP_GROUP_IDS group.talaviram.TICKPresets
    BACKGROUND_AUDIO_ENABLED TRUE
    BLUETOOTH_PERMISSION_ENABLED TRUE
    BACKGROUND_BLE_ENABLED TRUE
    DOCUMENT_BROWSER_ENABLED TRUE
    FILE_SHARING_ENABLED TRUE
    ICLOUD_PERMISSIONS_ENABLED TRUE
    NETWORK_MULTICAST_ENABLED TRUE
    REQUIRES_FULL_SCREEN FALSE
    IPAD_SCREEN_ORIENTATIONS UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight
    PLIST_TO_MERGE ${PLIST_ADDITION}
)

source_group("JUX" REGULAR_EXPRESSION "JUX/")
target_sources(TICK
    PRIVATE
    JUX/components/ListBox.h
    JUX/components/ListBox.cpp
    JUX/components/ListBoxMenu.h
    JUX/components/ListBoxMenu.cpp
)

source_group("Source" REGULAR_EXPRESSION "Source/")
source_group("Source/Components" REGULAR_EXPRESSION "Source/components")
source_group("Source/Model" REGULAR_EXPRESSION "Source/model")
source_group("Source/Utils" REGULAR_EXPRESSION "Source/utils")
target_sources(TICK
    PRIVATE
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/components/AboutView.h
    Source/components/DialogComponent.h
    Source/components/DialogComponent.cpp
    Source/components/EditBeatView.h
    Source/components/EditBeatView.cpp
    Source/components/LookAndFeel.h
    Source/components/LookAndFeel.cpp
    Source/components/ManageSamplesView.h
    Source/components/ManageSamplesView.cpp
    Source/components/PerformView.h
    Source/components/PerformView.cpp
    Source/components/PresetsView.h
    Source/components/PresetsView.cpp
    Source/components/TopBar.h
    Source/components/TopBar.cpp
    Source/components/TransportBar.h
    Source/components/TransportBar.cpp
    Source/model/AbletonLink.h
    Source/model/JuceState.h
    Source/model/TapModel.h
    Source/model/TapModel.cpp
    Source/model/TickModel.h
    Source/model/TickModel.cpp
    Source/utils/SamplesPaint.h
    Source/utils/SamplesPaint.cpp
    Source/utils/TickSplash.h
    Source/utils/TickSplash.cpp
    Source/utils/UtilityFunctions.h
    Source/utils/UtilityFunctions.cpp
)

if(APPLE_IOS)
    target_sources(TICK
        PRIVATE
        Source/model/AbletonLink.mm
        Source/utils/UtilityFunctions.mm
    )

    target_sources(TICK_Standalone PRIVATE "Installer/Factory")
    set_source_files_properties("Installer/Factory" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
endif()

target_sources(TICK_Standalone
    PRIVATE
    Source/Standalone.cpp
)

target_compile_definitions(TICK
    PUBLIC
    DONT_SET_USING_JUCE_NAMESPACE=1
    JUCE_DONT_DECLARE_PROJECTINFO=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DONT_AUTO_OPEN_MIDI_DEVICES_ON_MOBILE=1
    JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS=1
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_CONTENT_SHARING=1
)

juce_add_binary_data(TickBinaryData
    SOURCES
    factory_default.preset
    factory_samples.zip
    Media/Font/KlokanTechNotoSans-Bold.ttf
    Media/Font/KlokanTechNotoSans-Italic.ttf
    Media/Font/KlokanTechNotoSans-Regular.ttf
    Media/Logo/tick_icon.svg
    Media/Logo/tick_icon_with_text.svg
    Media/about_text.txt
    Media/arrow_back_ios-24px.svg
    Media/background.png
    Media/check_circle-24px.svg
    Media/edit-24px.svg
    Media/delete-24px.svg
    Media/expand_more-24px.svg
    Media/folder_open-24px.svg
    Media/ios_share_black_24dp.svg
    Media/lock_clock-24px.svg
    Media/metro_tick_icon.svg
    Media/more_horiz-24px.svg
    Media/more_vert-24px.svg
    Media/play-button.svg
    Media/sample-icon.svg
    Media/settings_black_24dp.svg
    Media/stop-button.svg
    Media/unfold_more_black_24dp.svg
)

add_custom_target(
    generate_git_revision
    COMMAND ${CMAKE_COMMAND}
    -Dlocal_dir="${CMAKE_CURRENT_SOURCE_DIR}"
    -Doutput_dir="${CMAKE_CURRENT_SOURCE_DIR}/Source/utils"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gitversion.cmake")

add_dependencies(TICK generate_git_revision)

if(APPLE_IOS)
    if(EXISTS "private/LinkKit")
        message("Ableton LinkKit (iOS)")
        set(CMAKE_XCODE_ATTRIBUTE_OTHER_LDFLAGS[sdk=iphoneos*] "${CMAKE_SOURCE_DIR}/private/LinkKit/LinkKit.xcframework/ios-arm64_armv7/libLinkKit.a")
        set(CMAKE_XCODE_ATTRIBUTE_OTHER_LDFLAGS[sdk=iphonesimulator*] "${CMAKE_SOURCE_DIR}/private/LinkKit/LinkKit.xcframework/ios-arm64_i386_x86_64-simulator/libLinkKit.a")
        set(IOS_INCLUDES "${CMAKE_SOURCE_DIR}/private/LinkKit/LinkKit.xcframework/ios-arm64_armv7/Headers")
    endif()
endif()

juce_generate_juce_header(TICK)

target_link_libraries(TICK
    PRIVATE
    TickBinaryData
    juce::juce_audio_utils
    juce::juce_opengl
    samplerate
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

target_include_directories(TICK
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/JUX
    ${CMAKE_SOURCE_DIR}/libsamplerate/include
    ${IOS_INCLUDES}
)

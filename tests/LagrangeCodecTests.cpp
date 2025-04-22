//
// Created by pk5ls on 2025/2/25.
//

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <fstream>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
    typedef HMODULE DynLibHandle;
    #define LoadDynLib(name) LoadLibraryA(name)
    #define GetDynLibSymbol(handle, symbol) GetProcAddress(handle, symbol)
    #define CloseDynLib(handle) FreeLibrary(handle)
    #define LIBRARY_NAME "../LagrangeCodec.dll"
#else
    #include <dlfcn.h>
    typedef void* DynLibHandle;
    #define LoadDynLib(name) dlopen(name, RTLD_LAZY)
    #define GetDynLibSymbol(handle, symbol) dlsym(handle, symbol)
    #define CloseDynLib(handle) dlclose(handle)
    #ifdef __APPLE__
        #define LIBRARY_NAME "../libLagrangeCodec.dylib"
    #else
        #define LIBRARY_NAME "../libLagrangeCodec.so"
    #endif
#endif

typedef void (cb_codec)(void* userdata, uint8_t* p, int len);

struct VideoInfo {
    int width;
    int height;
    int64_t duration;
};

typedef int (*silk_decode_func)(uint8_t* silk_data, int len, cb_codec* callback, void* userdata);
typedef int (*silk_encode_func)(uint8_t* pcm_data, int len, cb_codec* callback, void* userdata);
typedef int (*video_first_frame_func)(uint8_t* video_data, int data_len, uint8_t*& out, int& out_len);
typedef int (*video_get_size_func)(uint8_t* video_data, int data_len, VideoInfo& info);
typedef int (*audio_to_pcm_func)(uint8_t* audio_data, int data_len, cb_codec* callback, void* userdata);

void testCallback(void* userdata, uint8_t* data, int len) {
    if (userdata && data && len > 0) {
        auto buffer = static_cast<std::vector<uint8_t>*>(userdata);
        buffer->insert(buffer->end(), data, data + len);
    }
}

class LagrangeCodecTest : public testing::Test {
protected:
    DynLibHandle library = nullptr;
    silk_decode_func silk_decode = nullptr;
    silk_encode_func silk_encode = nullptr;
    video_first_frame_func video_first_frame = nullptr;
    video_get_size_func video_get_size = nullptr;
    audio_to_pcm_func audio_to_pcm = nullptr;

    std::vector<uint8_t> audioData;
    std::vector<uint8_t> videoData;
    bool hasAudioData = false;
    bool hasVideoData = false;

    static std::vector<uint8_t> loadTestData(const std::string& file_name) {
        using namespace std;
        try {
            const filesystem::path exePath = filesystem::current_path();
            std::cout << "Current working directory: " << exePath << std::endl;
            filesystem::path dataPath = exePath.parent_path().parent_path() / "tests" / "test_data" / file_name;
            std::cout << "Attempting to load file: " << dataPath << std::endl;

            std::ifstream file(dataPath, std::ios::binary);
            if (!file.is_open()) {
                std::cerr << "Failed to open test data file: " << dataPath << std::endl;
                return {};
            }

            file.seekg(0, std::ios::end);
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<uint8_t> buffer(size);
            file.read(reinterpret_cast<char*>(buffer.data()), size);

            if (file.gcount() != size) {
                std::cerr << "Failed to read entire file: " << dataPath << std::endl;
                return {};
            }

            std::cout << "Successfully loaded " << size << " bytes from " << dataPath << std::endl;
            return buffer;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception while loading test data: " << e.what() << std::endl;
            return {};
        }
    }

    void SetUp() override {
        library = LoadDynLib(LIBRARY_NAME);
        ASSERT_TRUE(library != nullptr) << "Failed to load the LagrangeCodec library";

        silk_decode = reinterpret_cast<silk_decode_func>(GetDynLibSymbol(library, "silk_decode"));
        ASSERT_TRUE(silk_decode != nullptr) << "Failed to load silk_decode function";

        silk_encode = reinterpret_cast<silk_encode_func>(GetDynLibSymbol(library, "silk_encode"));
        ASSERT_TRUE(silk_encode != nullptr) << "Failed to load silk_encode function";

        video_first_frame = reinterpret_cast<video_first_frame_func>(GetDynLibSymbol(library, "video_first_frame"));
        ASSERT_TRUE(video_first_frame != nullptr) << "Failed to load video_first_frame function";

        video_get_size = reinterpret_cast<video_get_size_func>(GetDynLibSymbol(library, "video_get_size"));
        ASSERT_TRUE(video_get_size != nullptr) << "Failed to load video_get_size function";

        audio_to_pcm = reinterpret_cast<audio_to_pcm_func>(GetDynLibSymbol(library, "audio_to_pcm"));
        ASSERT_TRUE(audio_to_pcm != nullptr) << "Failed to load audio_to_pcm function";

        std::cout << "Loading audio test data..." << std::endl;
        audioData = loadTestData("test_audio.mp3");
        hasAudioData = !audioData.empty();
        if (hasAudioData) {
            std::cout << "Audio data loaded successfully: " << audioData.size() << " bytes" << std::endl;
        } else {
            std::cout << "Failed to load audio data!" << std::endl;
        }

        std::cout << "Loading video test data..." << std::endl;
        videoData = loadTestData("test_video.mp4");
        hasVideoData = !videoData.empty();
        if (hasVideoData) {
            std::cout << "Video data loaded successfully: " << videoData.size() << " bytes" << std::endl;
        } else {
            std::cout << "Failed to load video data!" << std::endl;
        }
    }

    void TearDown() override {
        if (library) {
            CloseDynLib(library);
            library = nullptr;
        }
    }
};

class LagrangeAudioCodecTest : public LagrangeCodecTest {
protected:
    std::vector<uint8_t> pcmData;
    std::vector<uint8_t> silkData;
    std::vector<uint8_t> decodedPcmData;
};

TEST_F(LagrangeAudioCodecTest, TestAudioEncodingChain) {
    ASSERT_TRUE(hasAudioData) << "Audio test data not available";

    std::cout << "Step 1: Converting MP3 to PCM..." << std::endl;
    int result = audio_to_pcm(audioData.data(), static_cast<int>(audioData.size()), testCallback, &pcmData);
    ASSERT_EQ(result, 0) << "audio_to_pcm function failed";
    ASSERT_FALSE(pcmData.empty()) << "No PCM data was generated";
    std::cout << "PCM data size: " << pcmData.size() << " bytes" << std::endl;

    std::cout << "Step 2: Encoding PCM to SILK..." << std::endl;
    result = silk_encode(pcmData.data(), static_cast<int>(pcmData.size()), testCallback, &silkData);
    ASSERT_EQ(result, 0) << "silk_encode function failed";
    ASSERT_FALSE(silkData.empty()) << "No SILK data was generated";
    std::cout << "SILK encoded data size: " << silkData.size() << " bytes" << std::endl;

    std::cout << "Step 3: Decoding SILK to PCM..." << std::endl;
    result = silk_decode(silkData.data(), static_cast<int>(silkData.size()), testCallback, &decodedPcmData);
    ASSERT_EQ(result, 0) << "silk_decode function failed";
    ASSERT_FALSE(decodedPcmData.empty()) << "No PCM data was decoded";
    std::cout << "Decoded PCM data size: " << decodedPcmData.size() << " bytes" << std::endl;

    EXPECT_GT(pcmData.size(), 0);
    EXPECT_GT(decodedPcmData.size(), 0);
}

TEST_F(LagrangeCodecTest, TestVideoFirstFrame) {
    ASSERT_TRUE(hasVideoData) << "Video test data not available";

    uint8_t* frameData = nullptr;
    int frameLen = 0;
    std::cout << "Extracting first frame from video..." << std::endl;
    const int result = video_first_frame(videoData.data(), static_cast<int>(videoData.size()), frameData, frameLen);
    std::cout << "Extraction completed with result: " << result << std::endl;
    EXPECT_EQ(result, 0) << "video_first_frame function failed";
    EXPECT_TRUE(frameData != nullptr) << "No frame data was generated";
    EXPECT_GT(frameLen, 0) << "Frame length is invalid";
    if (frameData != nullptr) {
        std::cout << "First frame data size: " << frameLen << " bytes" << std::endl;
    }
    // // save frame data
    // std::ofstream file("first_frame.png", std::ios::binary);
    // file.write(reinterpret_cast<char*>(frameData), frameLen);
    // file.close();
}

TEST_F(LagrangeCodecTest, TestVideoGetSize) {
    ASSERT_TRUE(hasVideoData) << "Video test data not available";

    VideoInfo info = {};
    std::cout << "Getting video size info..." << std::endl;
    const int result = video_get_size(videoData.data(), static_cast<int>(videoData.size()), info);
    std::cout << "Video info retrieval completed with result: " << result << std::endl;
    EXPECT_EQ(result, 0) << "video_get_size function failed";
    EXPECT_EQ(info.width, 320) << "Video width is not expected";
    EXPECT_EQ(info.height, 240) << "Video height is not expected";
    EXPECT_EQ(info.duration, 124) << "Video duration is not expected";
    std::cout << "Video dimensions: " << info.width << "x" << info.height
              << ", duration: " << info.duration << std::endl;
}


TEST_F(LagrangeAudioCodecTest, TestAudioToPcmOnly) {
    ASSERT_TRUE(hasAudioData) << "Audio test data not available";

    std::vector<uint8_t> localPcmData;
    int result = audio_to_pcm(audioData.data(), static_cast<int>(audioData.size()), testCallback, &localPcmData);
    EXPECT_EQ(result, 0) << "audio_to_pcm function failed";
    EXPECT_FALSE(localPcmData.empty()) << "No PCM data was generated";
    std::cout << "PCM data size: " << localPcmData.size() << " bytes" << std::endl;
}

TEST_F(LagrangeAudioCodecTest, TestPcmToSilkOnly) {
    ASSERT_TRUE(hasAudioData) << "Audio test data not available";

    std::vector<uint8_t> localPcmData;
    int result = audio_to_pcm(audioData.data(), static_cast<int>(audioData.size()), testCallback, &localPcmData);
    ASSERT_EQ(result, 0) << "Failed to prepare PCM data";

    std::vector<uint8_t> localSilkData;
    result = silk_encode(localPcmData.data(), static_cast<int>(localPcmData.size()), testCallback, &localSilkData);
    EXPECT_EQ(result, 0) << "silk_encode function failed";
    EXPECT_FALSE(localSilkData.empty()) << "No SILK data was generated";
    std::cout << "SILK encoded data size: " << localSilkData.size() << " bytes" << std::endl;
}

int main(int argc, char **argv) {
    std::cout << "Starting LagrangeCodec tests..." << std::endl;
    testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
    std::cout << "Tests completed with result: " << result << std::endl;
    return result;
}
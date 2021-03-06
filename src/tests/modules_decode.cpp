
#include "tests.hpp"
#include "lib_modules/modules.hpp"
#include "lib_media/decode/libav_decode.hpp"
#include "lib_media/encode/libav_encode.hpp"
#include "lib_media/in/file.hpp"
#include "lib_media/out/null.hpp"
#include "lib_media/transform/audio_convert.hpp"
#include "lib_utils/tools.hpp"

extern "C" {
#include "libavcodec/avcodec.h"
}

using namespace Tests;
using namespace Modules;

namespace {
Decode::LibavDecode* createGenericDecoder(enum AVCodecID id) {
	auto codec = avcodec_find_decoder(id);
	auto context = avcodec_alloc_context3(codec);
	MetadataPktLibav metadata(context);
	auto decode = create<Decode::LibavDecode>(metadata);
	avcodec_close(context);
	av_free(context);
	return decode;
}

Decode::LibavDecode* createMp3Decoder() {
	return createGenericDecoder(AV_CODEC_ID_MP3);
}

template<size_t numBytes>
std::shared_ptr<DataBase> createAvPacket(uint8_t const (&bytes)[numBytes]) {
	auto pkt = std::make_shared<DataAVPacket>(numBytes);
	memcpy(pkt->data(), bytes, numBytes);
	return pkt;
}

std::shared_ptr<DataBase> getTestMp3Frame() {
	static const uint8_t mp3_sine_frame[] = {
		0xff, 0xfb, 0x30, 0xc0, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x49, 0x6e, 0x66, 0x6f,
		0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x29,
		0x00, 0x00, 0x19, 0xb6, 0x00, 0x0c, 0x0c, 0x12,
		0x12, 0x18, 0x18, 0x18, 0x1e, 0x1e, 0x24, 0x24,
		0x24, 0x2a, 0x2a, 0x30, 0x30, 0x30, 0x36, 0x36,
		0x3c, 0x3c, 0x43, 0x43, 0x43, 0x49, 0x49, 0x4f,
		0x4f, 0x4f, 0x55, 0x55, 0x5b, 0x5b, 0x5b, 0x61,
		0x61, 0x67, 0x67, 0x67, 0x6d, 0x6d, 0x73, 0x73,
		0x79, 0x79, 0x79, 0x7f, 0x7f, 0x86, 0x86, 0x86,
		0x8c, 0x8c, 0x92, 0x92, 0x92, 0x98, 0x98, 0x9e,
		0x9e, 0xa4, 0xa4, 0xa4, 0xaa, 0xaa, 0xb0, 0xb0,
		0xb0, 0xb6, 0xb6, 0xbc, 0xbc, 0xbc, 0xc3, 0xc3,
		0xc9, 0xc9, 0xc9, 0xcf, 0xcf, 0xd5, 0xd5, 0xdb,
		0xdb, 0xdb, 0xe1, 0xe1, 0xe7, 0xe7, 0xe7, 0xed,
		0xed, 0xf3, 0xf3, 0xf3, 0xf9, 0xf9, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00
	};

	return createAvPacket(mp3_sine_frame);
}

}

unittest("decode: audio simple") {
	auto decode = uptr(createMp3Decoder());

	auto null = uptr(create<Out::Null>());
	ConnectOutputToInput(decode->getOutput(0), null);

	auto frame = getTestMp3Frame();
	decode->process(frame);
}

namespace {
Decode::LibavDecode* createVideoDecoder() {
	return createGenericDecoder(AV_CODEC_ID_H264);
}

std::shared_ptr<DataBase> getTestH24Frame() {
	static const uint8_t h264_gray_frame[] = {
		0x00, 0x00, 0x00, 0x01,
		0x67, 0x4d, 0x40, 0x0a, 0xe8, 0x8f, 0x42, 0x00,
		0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x03, 0x00,
		0x64, 0x1e, 0x24, 0x4a, 0x24,
		0x00, 0x00, 0x00, 0x01, 0x68, 0xeb, 0xc3, 0xcb,
		0x20, 0x00, 0x00, 0x01, 0x65, 0x88, 0x84, 0x00,
		0xaf, 0xfd, 0x0f, 0xdf,
	};

	return createAvPacket(h264_gray_frame);
}
}

unittest("decode: video simple") {
	auto decode = uptr(createVideoDecoder());
	auto data = getTestH24Frame();

	auto onPic = [&](Data data) {
		auto const pic = safe_cast<const DataPicture>(data);
		auto const format = pic->getFormat();
		ASSERT_EQUALS(16, format.res.width);
		ASSERT_EQUALS(16, format.res.height);
		ASSERT_EQUALS(YUV420P, format.format);

		auto const firstPixel = *pic->getPlane(0);
		auto const lastPixel = *(pic->getPlane(0) + format.res.width * format.res.height - 1);
		ASSERT_EQUALS(0x80, firstPixel);
		ASSERT_EQUALS(0x80, lastPixel);
	};

	Connect(decode->getOutput(0)->getSignal(), onPic);
	decode->process(data);
	decode->process(data);
}

#ifdef ENABLE_FAILING_TESTS
//TODO: this test fails because the exception is caught by a Signals future. To be tested when tasks are pushed to an executor
unittest("decode: failing audio mp3 to AAC") {
	auto decode = uptr(createMp3Decoder());
	auto encoder = uptr(create<Encode::LibavEncode>(Encode::LibavEncode::Audio));

	ConnectOutputToInput(decode->getOutput(0), encoder);

	auto frame = getTestMp3Frame();
	bool thrown = false;
	try {
		decode->process(frame);
	} catch (std::exception const& e) {
		std::cerr << "Expected error: " << e.what() << std::endl;
		thrown = true;
	}
	ASSERT(thrown);
}
#endif

#ifdef ENABLE_FAILING_TESTS
//TODO: fails because the dst number of samples for the resampler is not ok for some AAC encoders
unittest("decode: audio mp3 to converter to AAC") {
	auto decode = uptr(createMp3Decoder());
	auto encoder = uptr(create<Encode::LibavEncode>(Encode::LibavEncode::Audio));

	auto srcFormat = PcmFormat(44100, 1, AudioLayout::Mono, AudioSampleFormat::S16, AudioStruct::Planar);
	auto dstFormat = PcmFormat(44100, 2, AudioLayout::Stereo, AudioSampleFormat::S16, AudioStruct::Interleaved);
	auto converter = uptr(create<Transform::AudioConvert>(srcFormat, dstFormat));

	ConnectOutputToInput(decode->getOutput(0), converter);
	ConnectOutputToInput(converter->getOutput(0), encoder);

	auto frame = getTestMp3Frame();
	decode->process(frame);
}
#endif



#include "tests.hpp"
#include "modules.hpp"
#include <memory>

#include "libavcodec/avcodec.h" //FIXME: there should be none of the modules include at the application level

using namespace Tests;
using namespace Modules;

namespace {

	unittest("Packet type erasure + multi-output-pin: libav Demux -> libav Decoder (Video Only) -> Render::SDL2") {
		std::unique_ptr<Demux::LibavDemux> demux(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
		ASSERT(demux != nullptr);
		std::unique_ptr<Out::Null> null(Out::Null::create());
		ASSERT(null != nullptr);

		size_t videoIndex = std::numeric_limits<size_t>::max();
		for (size_t i = 0; i < demux->signals.size(); ++i) {
			Props *props = demux->signals[i]->props.get();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);
			if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_VIDEO) { //TODO: expose it somewhere
				videoIndex = i;
			} else {
				//FIXME: we have to set Print output to avoid asserts. Should be remove once the framework is more tested.
				Connect(demux->signals[i]->signal, null.get(), &Out::Null::process);
			}
		}
		ASSERT(videoIndex != std::numeric_limits<size_t>::max());
		Props *props = demux->signals[videoIndex]->props.get();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		std::unique_ptr<Decode::LibavDecode> decode(Decode::LibavDecode::create(*decoderProps));
		ASSERT(decode != nullptr);

		std::unique_ptr<Render::SDLVideo> render(Render::SDLVideo::create());
		ASSERT(render != nullptr);

		Connect(demux->signals[videoIndex]->signal, decode.get(), &Decode::LibavDecode::process);
		Connect(decode->signals[0]->signal, render.get(), &Render::SDLVideo::process);

		while (demux->process(nullptr)) {
		}

		demux->destroy();
		decode->destroy();
	}

	unittest("Packet type erasure + multi-output-pin: libav Demux -> libav Decoder (Audio Only) -> Render::SDL2") {
		std::unique_ptr<Demux::LibavDemux> demux(Demux::LibavDemux::create("data/BatmanHD_1000kbit_mpeg_0_20_frag_1000.mp4"));
		ASSERT(demux != nullptr);
		std::unique_ptr<Out::Null> null(Out::Null::create());
		ASSERT(null != nullptr);

		size_t videoIndex = std::numeric_limits<size_t>::max();
		for (size_t i = 0; i < demux->signals.size(); ++i) {
			Props *props = demux->signals[i]->props.get();
			PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
			ASSERT(decoderProps);
			if (decoderProps->getAVCodecContext()->codec_type == AVMEDIA_TYPE_AUDIO) { //TODO: expose it somewhere
				videoIndex = i;
			} else {
				//FIXME: we have to set Print output to avoid asserts. Should be remove once the framework is more tested.
				Connect(demux->signals[i]->signal, null.get(), &Out::Null::process);
			}
		}
		ASSERT(videoIndex != std::numeric_limits<size_t>::max());
		Props *props = demux->signals[videoIndex]->props.get();
		PropsDecoder *decoderProps = dynamic_cast<PropsDecoder*>(props);
		std::unique_ptr<Decode::LibavDecode> decode(Decode::LibavDecode::create(*decoderProps));
		ASSERT(decode != nullptr);

		std::unique_ptr<Render::SDLAudio> render(Render::SDLAudio::create());
		ASSERT(render != nullptr);

		Connect(demux->signals[videoIndex]->signal, decode.get(), &Decode::LibavDecode::process);
		Connect(decode->signals[0]->signal, render.get(), &Render::SDLAudio::process);

		while (demux->process(nullptr)) {
		}

		demux->destroy();
		decode->destroy();
	}

}
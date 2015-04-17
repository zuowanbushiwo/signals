#include "tests.hpp"
#include "lib_modules/modules.hpp"

#include "lib_media/demux/libav_demux.hpp"
#include "lib_media/mux/gpac_mux_mp4.hpp"
#include "lib_utils/tools.hpp"


using namespace Tests;
using namespace Modules;

namespace {
size_t ConnectPinToModuleNewInput(IPin* pin, Module* module, size_t outputIdx) {
	Modules::IProcessor *input = new Input<Data>(module);
	auto inputPin = module->addInputPin(input);
	ASSERT((void*)inputPin == (void*)module->getInputPin(outputIdx));
	return ConnectPinToModule(pin, module->getInputPin(outputIdx));
}
}

unittest("remux test: GPAC mp4 mux") {
	auto demux = uptr(new Demux::LibavDemux("data/BatmanHD_1000kbit_mpeg.mp4"));
	auto mux = uptr(new Mux::GPACMuxMP4("output_video_libav"));
	for (size_t i = 0; i < demux->getNumOutputPins(); ++i) {
		//TODO: rename helpers? Pin/Module -> Connector/Connected ?
		ConnectPinToModuleNewInput(demux->getOutputPin(i), mux.get(), i);
		break; //FIXME
	}

	demux->process(nullptr);
}
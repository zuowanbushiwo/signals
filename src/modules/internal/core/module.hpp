#pragma once

#include "data.hpp"
#include "pin.hpp"
#include <vector>


namespace Modules {

class Module {
public:
	Module() = default;

	virtual ~Module() noexcept(false) {
	}

	virtual void process(std::shared_ptr<const Data> data) = 0;
	virtual void flush() { };

	size_t getNumPin() const {
		return pins.size();
	}

	IPin* getPin(size_t i) {
		return pins[i].get();
	}

protected:
	Module(Module const&) = delete;
	Module const& operator=(Module const&) = delete;

	std::vector<std::unique_ptr<IPin>> pins;
	template<typename T>
	T* addPin(T* p) {
		pins.push_back(uptr(p));
		return p;
	}
};

}
CXX   = g++
EMCC  = emcc

SRCS  = game.cpp player.cpp strategies.cpp shoe.cpp hand.cpp card.cpp utilities.cpp

NATIVE_FLAGS = -O3 -std=c++17 -pthread
WASM_FLAGS   = -O3 -std=c++17 \
	-s EXPORTED_FUNCTIONS='["_bjsim_configure","_bjsim_run_batch","_bjsim_reset","_bjsim_get_results"]' \
	-s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
	-s MODULARIZE=1 \
	-s EXPORT_NAME=BJSim \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s ENVIRONMENT=worker

.PHONY: native wasm test clean

native:
	$(CXX) $(NATIVE_FLAGS) -o bjsim $(SRCS) bjsim.cpp

wasm:
	$(EMCC) $(WASM_FLAGS) -o public/bjsim.js $(SRCS) wasm_api.cpp

test:
	$(CXX) -O2 -std=c++17 -o tests/tests \
		tests/tests.cpp hand.cpp shoe.cpp strategies.cpp utilities.cpp card.cpp player.cpp

clean:
	rm -f bjsim tests/tests public/bjsim.js public/bjsim.wasm

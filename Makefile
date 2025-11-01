CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -g
SRCDIR = src
OBJDIR = obj

# Subdiretórios
SUBDIRS = \
    $(SRCDIR)/frontend/lexer \
    $(SRCDIR)/frontend/parser \
    $(SRCDIR)/frontend/ast \
    $(SRCDIR)/frontend/semantic \
    $(SRCDIR)/middleend/symbol_table \
    $(SRCDIR)/middleend/tac \
    $(SRCDIR)/backend/arm \
    $(SRCDIR)/backend/optimization \
    $(SRCDIR)/emscripten \
    $(SRCDIR)/runtime/threads \
    $(SRCDIR)/runtime/channels \
    $(SRCDIR)/middleend/runtime

# Fontes para build NATIVO (exclui emscripten)
SOURCES_ROOT = $(wildcard $(SRCDIR)/*.cpp)
SOURCES_ROOT := $(filter-out $(SRCDIR)/main_emscripten.cpp,$(SOURCES_ROOT))
SOURCES_ROOT := $(filter-out $(SRCDIR)/emscripten_interface.cpp,$(SOURCES_ROOT))
SOURCES_SUB  = $(foreach d,$(SUBDIRS),$(wildcard $(d)/*.cpp))
SOURCES = $(SOURCES_ROOT) $(SOURCES_SUB)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
TARGET = compilador

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

clean-wasm:
	rm -rf obj_wasm web/compilador.js web/compilador.wasm

test: $(TARGET)
	./$(TARGET) exemplos/teste_simples.minipar

wasm:
	make -f Makefile.emscripten

serve:
	cd web && python3 -m http.server 8000

web: wasm serve

.PHONY: clean clean-wasm test serve web wasm
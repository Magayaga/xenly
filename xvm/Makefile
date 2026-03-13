# ═══════════════════════════════════════════════════════════════════════════
# XVM — Xenly Virtual Machine Build System
#
# Created, designed, and developed by Cyril John Magayaga
# Available for Windows, macOS, and Linux
# ═══════════════════════════════════════════════════════════════════════════

.PHONY: all build xenlybyc xenlyrun clean test fmt vet help

GOCMD   = go
GOBUILD = $(GOCMD) build
GOTEST  = $(GOCMD) test
GOFMT   = $(GOCMD) fmt
GOVET   = $(GOCMD) vet

# Output binaries
BINDIR    = bin
XENLYBYC  = $(BINDIR)/xenlybyc
XENLYRUN  = $(BINDIR)/xenlyrun

# Windows suffix
ifeq ($(OS),Windows_NT)
    EXE = .exe
    XENLYBYC := $(XENLYBYC).exe
    XENLYRUN := $(XENLYRUN).exe
endif

# Build flags
LDFLAGS   = -ldflags="-s -w"
BUILD_FLAGS =

all: build

build: xenlybyc xenlyrun
	@echo ""
	@echo "✓  XVM build complete"
	@echo "   Bytecode compiler : $(XENLYBYC)"
	@echo "   VM launcher       : $(XENLYRUN)"
	@echo ""

xenlybyc:
	@mkdir -p $(BINDIR)
	@echo "Building xenlybyc..."
	$(GOBUILD) $(BUILD_FLAGS) $(LDFLAGS) -o $(XENLYBYC) ./xenlybyc/
	@echo "  ✓  $(XENLYBYC)"

xenlyrun:
	@mkdir -p $(BINDIR)
	@echo "Building xenlyrun..."
	$(GOBUILD) $(BUILD_FLAGS) $(LDFLAGS) -o $(XENLYRUN) ./xenlyrun/
	@echo "  ✓  $(XENLYRUN)"

# ── Cross compilation ────────────────────────────────────────────────────────

cross-linux-amd64:
	@mkdir -p $(BINDIR)
	GOOS=linux   GOARCH=amd64   $(GOBUILD) $(LDFLAGS) -o $(BINDIR)/xenlybyc-linux-amd64   ./xenlybyc/
	GOOS=linux   GOARCH=amd64   $(GOBUILD) $(LDFLAGS) -o $(BINDIR)/xenlyrun-linux-amd64   ./xenlyrun/

cross-linux-arm64:
	@mkdir -p $(BINDIR)
	GOOS=linux   GOARCH=arm64   $(GOBUILD) $(LDFLAGS) -o $(BINDIR)/xenlybyc-linux-arm64   ./xenlybyc/
	GOOS=linux   GOARCH=arm64   $(GOBUILD) $(LDFLAGS) -o $(BINDIR)/xenlyrun-linux-arm64   ./xenlyrun/

cross-darwin-amd64:
	@mkdir -p $(BINDIR)
	GOOS=darwin  GOARCH=amd64   $(GOBUILD) $(LDFLAGS) -o $(BINDIR)/xenlybyc-darwin-amd64  ./xenlybyc/
	GOOS=darwin  GOARCH=amd64   $(GOBUILD) $(LDFLAGS) -o $(BINDIR)/xenlyrun-darwin-amd64  ./xenlyrun/

cross-darwin-arm64:
	@mkdir -p $(BINDIR)
	GOOS=darwin  GOARCH=arm64   $(GOBUILD) $(LDFLAGS) -o $(BINDIR)/xenlybyc-darwin-arm64  ./xenlybyc/
	GOOS=darwin  GOARCH=arm64   $(GOBUILD) $(LDFLAGS) -o $(BINDIR)/xenlyrun-darwin-arm64  ./xenlyrun/

cross-windows-amd64:
	@mkdir -p $(BINDIR)
	GOOS=windows GOARCH=amd64   $(GOBUILD) $(LDFLAGS) -o $(BINDIR)/xenlybyc-windows-amd64.exe ./xenlybyc/
	GOOS=windows GOARCH=amd64   $(GOBUILD) $(LDFLAGS) -o $(BINDIR)/xenlyrun-windows-amd64.exe ./xenlyrun/

cross-all: cross-linux-amd64 cross-linux-arm64 cross-darwin-amd64 cross-darwin-arm64 cross-windows-amd64
	@echo "✓  All cross-compile targets built"

# ── Testing ──────────────────────────────────────────────────────────────────

test: build
	@echo "═══════════════════════════════════════"
	@echo "  Running XVM test suite"
	@echo "═══════════════════════════════════════"
	@echo 'print("Hello, World!")' > /tmp/xvm_test.xe
	$(XENLYBYC) /tmp/xvm_test.xe -o /tmp/xvm_test.xyc
	@echo "  ✓  xenlybyc compiled hello"
	$(XENLYRUN) /tmp/xvm_test.xyc
	@echo "  ✓  xenlyrun executed hello"
	@rm -f /tmp/xvm_test.xe /tmp/xvm_test.xyc
	$(GOTEST) ./...
	@echo "✓  All tests passed"

# ── Code quality ──────────────────────────────────────────────────────────────

fmt:
	$(GOFMT) ./...

vet:
	$(GOVET) ./...

# ── Clean ─────────────────────────────────────────────────────────────────────

clean:
	@echo "Cleaning..."
	rm -rf $(BINDIR)
	rm -f *.xyc
	@echo "✓  Clean complete"

# ── Install ───────────────────────────────────────────────────────────────────

PREFIX ?= /usr/local
BININSTDIR = $(PREFIX)/bin

install: build
	install -d $(BININSTDIR)
	install -m 755 $(XENLYBYC) $(BININSTDIR)/xenlybyc$(EXE)
	install -m 755 $(XENLYRUN)  $(BININSTDIR)/xenlyrun$(EXE)
	@echo "✓  Installed to $(BININSTDIR)"

uninstall:
	rm -f $(BININSTDIR)/xenlybyc$(EXE)
	rm -f $(BININSTDIR)/xenlyrun$(EXE)
	@echo "✓  Uninstalled"

# ── Help ──────────────────────────────────────────────────────────────────────

help:
	@echo ""
	@echo "XVM — Xenly Virtual Machine Build System"
	@echo "══════════════════════════════════════════════════"
	@echo ""
	@echo "  Targets:"
	@echo "    all              Build xenlybyc + xenlyrun (default)"
	@echo "    xenlybyc         Build the bytecode compiler only"
	@echo "    xenlyrun         Build the VM launcher only"
	@echo "    test             Build and run test suite"
	@echo "    fmt              Auto-format all Go source"
	@echo "    vet              Run go vet on all packages"
	@echo "    cross-all        Build for all supported platforms"
	@echo "    cross-linux-amd64   Linux x86-64"
	@echo "    cross-linux-arm64   Linux ARM64"
	@echo "    cross-darwin-amd64  macOS x86-64"
	@echo "    cross-darwin-arm64  macOS Apple Silicon"
	@echo "    cross-windows-amd64 Windows x86-64"
	@echo "    install          Install to PREFIX (default: /usr/local)"
	@echo "    uninstall        Remove installed files"
	@echo "    clean            Remove build artifacts"
	@echo "    help             Show this message"
	@echo ""
	@echo "  Variables:"
	@echo "    PREFIX=<path>    Install prefix  (default: /usr/local)"
	@echo ""
	@echo "  Workflow examples:"
	@echo "    make                         # build both tools"
	@echo "    make test                    # build + smoke test"
	@echo "    ./bin/xenlybyc hello.xe      # compile hello.xe → hello.xyc"
	@echo "    ./bin/xenlyrun  hello.xyc    # run compiled bytecode"
	@echo "    ./bin/xenlyrun --run hello.xe  # compile + run in one step"
	@echo "    make cross-all               # build for all platforms"
	@echo ""

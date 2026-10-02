TEST_STRESS_SRC := $(PWD)/testing/stcp_stress.c
TEST_STRESS_BIN := $(PWD)/testing/stcp-stress

.PHONY: \
	stress-build \
	stress-tcp \
	stress-udp \
	stress-all

stress-build: $(TEST_STRESS_BIN)

$(TEST_STRESS_BIN): $(TEST_STRESS_SRC)
	$(TEST_CC) \
		-O2 \
		-g \
		-Wall \
		-Wextra \
		-Werror \
		-pthread \
		"$<" \
		-o "$@"

stress-tcp: module stress-build
	@set -e; \
	cleanup() { \
		$(SUDO) rmmod stcp 2>/dev/null || true; \
	}; \
	trap cleanup EXIT INT TERM; \
	$(SUDO) modprobe libcurve25519; \
	$(SUDO) modprobe libchacha20poly1305; \
	$(SUDO) insmod "$(PWD)/stcp.ko"; \
	bash "$(PWD)/testing/run-stress-tcp.sh"

stress-udp: module stress-build
	@set -e; \
	cleanup() { \
		$(SUDO) rmmod stcp 2>/dev/null || true; \
	}; \
	trap cleanup EXIT INT TERM; \
	$(SUDO) modprobe libcurve25519; \
	$(SUDO) modprobe libchacha20poly1305; \
	$(SUDO) insmod "$(PWD)/stcp.ko"; \
	bash "$(PWD)/testing/run-stress-udp.sh"

stress-all: stress-tcp stress-udp
	@echo
	@echo "========================================"
	@echo " ALL STCP STRESS TESTS PASSED"
	@echo "========================================"

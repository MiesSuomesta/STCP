CARGO ?= cargo
.PHONY: all test clean
all:
	$(CARGO) build --release --locked
	cp target/release/ben2-server ./ben2-server
test:
	$(CARGO) test --locked
clean:
	$(CARGO) clean
	rm -f ben2-server

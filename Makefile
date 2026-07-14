TASKS := \
	task00-sort \
	task01-syscalls \
	task02-fileio \
	task03-processes \
	task04-concurrency \
	task05-memory \
	task06-sockets \
	task07-llvm

.PHONY: all clean $(TASKS)

all: $(TASKS)

$(TASKS):
	$(MAKE) -C $@

clean:
	@set -e; for task in $(TASKS); do \
		$(MAKE) -C $$task clean; \
	done

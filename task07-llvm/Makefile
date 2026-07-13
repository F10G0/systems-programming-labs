.PHONY: all dce memory-safety clean

all: dce memory-safety

dce:
	$(MAKE) -C dead-code-elimination

memory-safety:
	$(MAKE) -C memory-safety

clean:
	$(MAKE) -C dead-code-elimination clean
	$(MAKE) -C memory-safety clean
	
.PHONY: client sysmodule all

client:
	$(MAKE) -C client

sysmodule:
	$(MAKE) -C sysmodule
	
clean:
	$(MAKE) clean -C client
	$(MAKE) clean -C sysmodule

all: client sysmodule

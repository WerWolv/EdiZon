.PHONY: homebrew sysmodule all

client:
	$(MAKE) -C homebrew

sysmodule:
	$(MAKE) -C sysmodule
	
clean:
	$(MAKE) clean -C homebrew
	$(MAKE) clean -C sysmodule

all: homebrew sysmodule

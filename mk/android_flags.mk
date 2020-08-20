CFG_TA_DYNLINK = y
CFG_CORE_ASLR = y

ifneq (,$(filter hikey%, $(TARGET_PRODUCT)))
OPTEE_EXTRA_FLAGS += CFG_CORE_HEAP_SIZE=196608
endif

# https://docs.google.com/document/d/1YhXpsmefaMhWE4dIwPViG0lzfCKVkf1WqjvknyPh7Pw
# core crash (alignment fault) when looking up early TAs (memcmp on UUIDs)
# https://github.com/OP-TEE/optee_os/issues/3903
CFG_SCTLR_ALIGNMENT_CHECK = n

# Force DEBUG=0 for now else boot will hang (https://pastebin.ubuntu.com/p/jX3yfQ2xYV/)
OPTEE_EXTRA_FLAGS += DEBUG=0

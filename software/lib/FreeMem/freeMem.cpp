//
// Used in calculating free memory.
//
extern unsigned int __bss_end;
extern void *__brkval;

//
// Returns the current amount of free memory in bytes.
//
int freeMemory() {
	int free_memory;
	if ((int) __brkval) {
		return ((int) &free_memory) - ((int) __brkval);
  }
	return ((int) &free_memory) - ((int) &__bss_end);
}
#include "current.h"
#include "sched.h"
#include <stddef.h>
#include "printf.h"

struct stackframe {
	unsigned long fp;
	unsigned long pc;
};

extern unsigned long kallsyms_addresses;
extern unsigned long kallsyms_num_syms;
extern unsigned long kallsyms_names;
extern unsigned long kallsyms_token_table;
extern unsigned long kallsyms_token_index;
extern unsigned long kallsyms_markers;
extern unsigned long _bss_end;

static unsigned long get_symbol_pos(unsigned long addr,
				    unsigned long *symbolsize,
				    unsigned long *offset)
{
	unsigned long symbol_start = 0, symbol_end = 0;
	unsigned long i, low, high, mid;
	/* Do a binary search on the sorted kallsyms_addresses array. */
	low = 0;
	high = kallsyms_num_syms;

	while (high - low > 1) {
		mid = low + (high - low) / 2;
		if ((&kallsyms_addresses)[mid] <= addr)
			low = mid;
		else
			high = mid;
	}

	/*
	 * Search for the first aliased symbol. Aliased
	 * symbols are symbols with the same address.
	 */
	while (low && (&kallsyms_addresses)[low-1] == (&kallsyms_addresses)[low])
		--low;

	symbol_start = (&kallsyms_addresses)[low];

	/* Search for next non-aliased symbol. */
	for (i = low + 1; i < kallsyms_num_syms; i++) {
		if ((&kallsyms_addresses)[i] > symbol_start) {
			symbol_end = (&kallsyms_addresses)[i];
			break;
		}
	}

	/* If we found no next symbol, we use the end of the section. */
	if (!symbol_end) {
		symbol_end = (unsigned long)_bss_end;
	}

	if (symbolsize)
		*symbolsize = symbol_end - symbol_start;
	if (offset)
		*offset = addr - symbol_start;

	return low;
}


static unsigned int get_symbol_offset(unsigned long pos)
{
	unsigned char *name;
	int i;

	/*
	 * Use the closest marker we have. We have markers every 256 positions,
	 * so that should be close enough.
	 */
	name = (char *)(&kallsyms_names);

	/*
	 * Sequentially scan all the symbols up to the point we're searching
	 * for. Every symbol is stored in a [<len>][<len> bytes of data] format,
	 * so we just need to add the len to the current pointer for every
	 * symbol we wish to skip.
	 */
	for (i = 0; i < (pos & 0xFF); i++)
    {
		name = name + (*name) + 1;
    }

	return name - (unsigned char *)(&kallsyms_names);
}

static unsigned int kallsyms_expand_symbol(unsigned int off,
					   char *result, size_t maxlen)
{
	int namesLen;
	unsigned char *tptr, *data = (char *)&kallsyms_names + off;
    unsigned char *tokenStr;
    int tokenLen;
    int r_index = 0;
	namesLen = *data;
	data++;
    for(int i = 0; i < namesLen ; i++)
    {
        tptr = data + i;
        tokenStr = (char *)&kallsyms_token_table + *((short *)&kallsyms_token_index + *tptr);
        tokenLen =  ((short *)&kallsyms_token_index)[1 + (*tptr)] - ((short *)&kallsyms_token_index)[*tptr] - 1;
        for(int j = 0; j < tokenLen; j++)
        {
            result[r_index++] = tokenStr[j];
            maxlen--;
        }
    }
	if (maxlen)
		result[r_index] = '\0';
    return off;
}

static void dump_backtrace_entry(unsigned long where)
{
    char namebuf[128];
    unsigned long symbolsize,offset, index;
	index = get_symbol_pos(where, &symbolsize, &offset);
    kallsyms_expand_symbol(get_symbol_offset(index), namebuf, 128);
    printf("%s + offset:0x%x\n", namebuf+1, offset);
}

extern void *ret_to_user;
int unwind_frame(struct task_struct *tsk, struct stackframe *frame)
{
	unsigned long fp = frame->fp;

	if (fp & 0xf)
		return -1;

	if (!tsk)
		tsk = current;

	frame->fp = *(unsigned long *)(fp);
	frame->pc = *(unsigned long *)(fp + 8);
    if (!frame->fp && frame->pc == (unsigned long)&ret_to_user)
            return 1;

	/*
	 * Frames created upon entry from EL0 have NULL FP and PC values, so
	 * don't bother reporting these. Frames created by __noreturn functions
	 * might have a valid FP even if PC is bogus, so only terminate where
	 * both are NULL.
	 */
	if (!frame->fp && !frame->pc)
		return 1;

	return 0;
}

void dump_backtrace(struct pt_regs *regs, struct task_struct *tsk)
{
	struct stackframe frame;
	int skip;

	printf("%s(regs = %p tsk = %p)\n", __func__, regs, tsk);

	if (!tsk)
		tsk = current;

	if (tsk == current) {
		frame.fp = (unsigned long)__builtin_frame_address(0);
		frame.pc = (unsigned long)dump_backtrace;
	} else {
		frame.fp = tsk->cpu_context.fp;
		frame.pc = tsk->cpu_context.pc;
	}

	skip = !!regs;
	printf("Call trace:\n");
	do {
		/* skip until specified stack frame */
		if (!skip) {
			dump_backtrace_entry(frame.pc);
		} else if (frame.fp == regs->regs[29]) {
			skip = 0;
			/*
			 * Mostly, this is the case where this function is
			 * called in panic/abort. As exception handler's
			 * stack frame does not contain the corresponding pc
			 * at which an exception has taken place, use regs->pc
			 * instead.
			 */
			dump_backtrace_entry(regs->pc);
		}
	} while (!unwind_frame(tsk, &frame));

	//put_task_stack(tsk);
}